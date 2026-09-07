#include "core/include/tlk_i2c.h"
#include "api/include/tlk_time.h"
#include "core/include/tlk_clock.h"
#include "core/include/tlk_dma.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_plic.h"
#include "properties/tlk_i2c.h"
#include "properties/tlk_plic.h"
#include "registers/tlk_chip.h"
#include "registers/tlk_clock.h"
#include "registers/tlk_i2c.h"
#include "system/debug/log/tlk_log.h"
#include <string.h>

TLK_LOG_CREATE(i2c_driver, "I2C_DRIVER");

#define TLK_I2C_WRITE_READ_BIT          1
#define TLK_I2C_SLAVE_RX_IRQ_TRIG_LEVEL 4

#define TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us)                                              \
    if ((tlk_api_time_get_micros() - (start_tick)) > (timeout_us))                                 \
    return TLK_I2C_TIMEOUT

enum tlk_i2c_command
{
    TLK_I2C_CMD_ID    = TLK_BIT(0),
    TLK_I2C_CMD_ADDR  = TLK_BIT(1),
    TLK_I2C_CMD_DATAW = TLK_BIT(2),
    TLK_I2C_CMD_DATAR = TLK_BIT(3),
    TLK_I2C_CMD_START = TLK_BIT(4),
    TLK_I2C_CMD_STOP  = TLK_BIT(5),
    TLK_I2C_CMD_ID_R  = TLK_BIT(6),
    TLK_I2C_CMD_ACK   = TLK_BIT(7),
};

struct tlk_i2c_pm_device
{
    struct tlk_i2c_pinmux_config pinmux;
    union
    {
        struct tlk_i2c_master_config master_config;
        struct tlk_i2c_slave_config  slave_config;
    };
};

struct tlk_i2c_dma_config
{
    struct tlk_dma_config tx_config;
    struct tlk_dma_config rx_config;
    enum tlk_dma_chn      tx_channel;
    enum tlk_dma_chn      rx_channel;
};

#if TLK_IS_ENABLED(CONFIG_TLK_I2C0_DMA)

struct tlk_i2c_dma_config tlk_i2c0_dma_config = {
    .tx_config =
        {
            .dst_req_sel    = UNISDK_DMA_FUNC_NUM_I2C_TX,
            .src_req_sel    = 0,
            .dst_addr_ctrl  = TLK_DMA_ADDR_FIX,
            .src_addr_ctrl  = TLK_DMA_ADDR_INCREMENT,
            .dstmode        = TLK_DMA_HANDSHAKE_MODE,
            .srcmode        = TLK_DMA_NORMAL_MODE,
            .dstwidth       = TLK_DMA_WORD_WIDTH,
            .srcwidth       = TLK_DMA_WORD_WIDTH,
            .src_burst_size = 0,
            .read_num_en    = 0,
            .priority       = 0,
            .write_num_en   = 0,
            .auto_en        = 0,
        },
    .rx_config =
        {
            .dst_req_sel    = 0,
            .src_req_sel    = UNISDK_DMA_FUNC_NUM_I2C_RX,
            .dst_addr_ctrl  = TLK_DMA_ADDR_INCREMENT,
            .src_addr_ctrl  = TLK_DMA_ADDR_FIX,
            .dstmode        = TLK_DMA_NORMAL_MODE,
            .srcmode        = TLK_DMA_HANDSHAKE_MODE,
            .dstwidth       = TLK_DMA_WORD_WIDTH,
            .srcwidth       = TLK_DMA_WORD_WIDTH,
            .src_burst_size = 0,
            .read_num_en    = 0,
            .priority       = 0,
            .write_num_en   = 0,
            .auto_en        = 0,
        },
    .tx_channel = TLK_DMA_NONE,
    .rx_channel = TLK_DMA_NONE,
};

#endif
struct tlk_i2c_slave
{
    uint8_t*                      tx_buffer;
    uint8_t*                      rx_buffer;
    uint32_t                      tx_buffer_size;
    uint32_t                      rx_buffer_size;
    tlk_i2c_slave_event_handler_t handler;
    uint32_t                      rx_count;
};

#if TLK_IS_ENABLED(CONFIG_TLK_I2C0_SLAVE)

struct tlk_i2c_slave tlk_i2c0_slave = {};

#endif

struct tlk_i2c_device
{
    const enum tlk_gpio_mux sda_mux : 8;
    const enum tlk_gpio_mux scl_mux : 8;
    struct
    {
        uint8_t rx_index  : 2;
        uint8_t role      : 1;
        uint8_t send_stop : 1;
    };
    struct
    {
        uint32_t timeout_us;
    } master;

#if TLK_IS_ENABLED(CONFIG_TLK_I2C_PM_DEVICE)
    struct tlk_i2c_pm_device* retention;
#endif
};

#define TLK_DEFINE_I2C_DEVICE(num)                                                                 \
    TLK_IF_ENABLED(CONFIG_TLK_I2C##num##_PM_DEVICE, (struct tlk_i2c_pm_device tlk_i2c##num##_pm;)) \
                                                                                                   \
    struct tlk_i2c_device tlk_i2c##num = {                                                         \
        .scl_mux           = TLK_GPIO_MUX_I2C##num##_SCL,                                          \
        .sda_mux           = TLK_GPIO_MUX_I2C##num##_SDA,                                          \
        .send_stop         = 1,                                                                    \
        .master.timeout_us = 10000,                                                                \
        TLK_IF_ENABLED(CONFIG_TLK_I2C##num##_PM_DEVICE, (.retention = &tlk_i2c##num##_pm, ))};

#define TLK_DEFINE_I2C_DEVICE_IF_ENABLED(num)                                                      \
    TLK_IF_ENABLED(CONFIG_TLK_I2C##num, (TLK_DEFINE_I2C_DEVICE(num)))

TLK_FOR_CALL(UNISDK_I2C_COUNT, TLK_DEFINE_I2C_DEVICE_IF_ENABLED)

#if TLK_IS_ENABLED(CONFIG_TLK_I2C0)

enum tlk_i2c0_irq
{
    TLK_I2C0_SLAVE_WR_IRQ = TLK_BIT(
        0), // the slave parsing master cmd interrupt, when received the master read or write cmd
    TLK_I2C0_MASTER_NAK_IRQ = TLK_BIT(1), // master detect to the nack, will generate interrupt
    TLK_I2C0_RX_BUF_IRQ  = TLK_BIT(2), // rxfifo_cnt >= I2C_RX_IRQ_TRIG_LEV generates an interrupt
    TLK_I2C0_TX_BUF_IRQ  = TLK_BIT(3), // txfifo_cnt <= I2C_TX_IRQ_TRIG_LEV generates an interrupt
    TLK_I2C0_RX_DONE_IRQ = TLK_BIT(4), // when the stop signal is detected, an interrupt occurs
    TLK_I2C0_TX_DONE_IRQ = TLK_BIT(5), // when the stop signal is detected, an interrupt occurs
    TLK_I2C0_RX_END_IRQ  = TLK_BIT(6), // An interrupt is generated when one frame of data is
                                       // received (the stop signal has been sent)
    TLK_I2C0_TX_END_IRQ = TLK_BIT(7),  // An interrupt is generated when one frame of data is sent
                                       // (the stop signal has been sent)
    TLK_I2C0_STRETCH_IRQ = TLK_BIT(8), // The slave stretch interrupt, when tx_fifo is empty or
                                       // rx_fifo is full, will generate interrupt
};

__attribute__((unused)) static void tlk_i2c0_irq_clr(enum tlk_i2c0_irq status)
{
    if (status & TLK_I2C0_STRETCH_IRQ)
    {
        tlk_i2c0_reg.status1_bit.ss_scl_irq = 1;
    }
    if (status & TLK_I2C0_RX_BUF_IRQ)
    {
        tlk_i2c0.rx_index = 0;
    }
    tlk_i2c0_reg.irq_status = status;
}

__attribute__((unused)) static bool tlk_i2c0_irq_get(enum tlk_i2c0_irq status)
{
    if (status & TLK_I2C0_STRETCH_IRQ)
    {
        return tlk_i2c0_reg.status1_bit.ss_scl_irq;
    }
    return tlk_i2c0_reg.irq_status & status;
}

__attribute__((unused)) static void tlk_i2c0_irq_enable(enum tlk_i2c0_irq mask)
{
    if (mask & TLK_I2C0_STRETCH_IRQ)
    {
        tlk_i2c0_reg.control2_bit.slave_stretch_irq_en = 1;
    }
    tlk_i2c0_reg.irq_mask |= mask;
}

#endif

#if TLK_IS_ENABLED(CONFIG_TLK_I2C0)

#if TLK_IS_DISABLED(CONFIG_TLK_I2C0_DMA)

static enum tlk_i2c_status tlk_i2c0_handle_nak(uint32_t start_tick, uint32_t timeout_us)
{
    tlk_i2c0_irq_clr(TLK_I2C0_MASTER_NAK_IRQ);

    tlk_i2c0_reg.command = TLK_I2C_CMD_STOP;

    while (tlk_i2c0_reg.master_bit.busy)
    {
        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

    tlk_i2c0_irq_clr(TLK_I2C0_TX_BUF_IRQ);

    return TLK_I2C_NACK;
}

#endif

static void tlk_i2c0_master_set_len(uint32_t len)
{
    tlk_i2c0_reg.len[0] = (len >> 0) & 0xff;
    tlk_i2c0_reg.len[1] = (len >> 8) & 0xff;
    tlk_i2c0_reg.len[2] = (len >> 16) & 0xff;
}

static enum tlk_i2c_status tlk_i2c0_master_write(tlk_i2c_address_t id, uint8_t* buffer,
                                                 uint32_t size)
{
    uint32_t start_tick = tlk_api_time_get_micros();
    uint32_t timeout_us = tlk_i2c0.master.timeout_us;

#if TLK_IS_ENABLED(CONFIG_TLK_I2C0_MASTER_DMA)

    tlk_i2c0_irq_clr(TLK_I2C0_TX_BUF_IRQ);
    tlk_i2c0_reg.id = id << 1;

    tlk_dma_chn_transfer_configure(
        tlk_i2c0_dma_config.tx_channel, (uint32_t) buffer, (uint32_t) tlk_i2c0_reg.data_buff, size);

    tlk_dma_chn_transfer_start(tlk_i2c0_dma_config.tx_channel);

    tlk_i2c0_master_set_len(size);
    tlk_i2c0_reg.command &= ~TLK_I2C_CMD_STOP;
    tlk_i2c0_reg.command = TLK_I2C_CMD_START | TLK_I2C_CMD_ID | TLK_I2C_CMD_DATAW |
                           (tlk_i2c0.send_stop ? TLK_I2C_CMD_STOP : 0);

    while (tlk_i2c0_reg.master_bit.busy)
    {
        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

#else

    tlk_i2c0_irq_clr(TLK_I2C0_TX_BUF_IRQ);
    tlk_i2c0_reg.id      = id << 1;
    tlk_i2c0_reg.command = TLK_I2C_CMD_START | TLK_I2C_CMD_ID;

    while (tlk_i2c0_reg.master_bit.busy)
    {
        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

    if (tlk_i2c0_irq_get(TLK_I2C0_MASTER_NAK_IRQ))
    {
        return tlk_i2c0_handle_nak(start_tick, timeout_us);
    }

    tlk_i2c0_master_set_len(size);

    uint32_t count = 0;
    while (count < size)
    {
        if (tlk_i2c0_irq_get(TLK_I2C0_MASTER_NAK_IRQ))
        {
            return tlk_i2c0_handle_nak(start_tick, timeout_us);
        }

        if (tlk_i2c0_reg.buff_count_bit.tx_buf_cnt < 8)
        {
            tlk_i2c0_reg.data_buff[count % 4] = buffer[count];
            count++;

            if (count == 1)
            {
                // If stop is configured in the data phase, when nack is received in the data phase,
                // stop will also be configured in the exception handling, which may result in one
                // more stop configured. therefore, the next time you read or write, stop needs to
                // be cleared to avoid sending stop by mistake.
                tlk_i2c0_reg.command &= ~TLK_I2C_CMD_STOP;
                tlk_i2c0_reg.command =
                    TLK_I2C_CMD_DATAW | (tlk_i2c0.send_stop ? TLK_I2C_CMD_STOP : 0);
            }
        }

        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

    while (tlk_i2c0_reg.buff_count_bit.tx_buf_cnt)
    {
        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);

        if (tlk_i2c0_irq_get(TLK_I2C0_MASTER_NAK_IRQ))
        {
            return tlk_i2c0_handle_nak(start_tick, timeout_us);
        }
    }

    while (tlk_i2c0_reg.master_bit.busy)
    {
        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

#endif

    return TLK_I2C_OK;
}

static enum tlk_i2c_status tlk_i2c0_master_read(tlk_i2c_address_t id, uint8_t* buffer,
                                                uint32_t size)
{
    uint32_t start_tick = tlk_api_time_get_micros();
    uint32_t timeout_us = tlk_i2c0.master.timeout_us;

#if TLK_IS_ENABLED(CONFIG_TLK_I2C0_MASTER_DMA)

    tlk_i2c0_irq_clr(TLK_I2C0_TX_BUF_IRQ);
    tlk_i2c0_reg.id                         = (id << 1) | TLK_I2C_WRITE_READ_BIT;
    tlk_i2c0_reg.control3_bit.master_nak_en = 1;

    tlk_dma_chn_transfer_configure(
        tlk_i2c0_dma_config.rx_channel, (uint32_t) tlk_i2c0_reg.data_buff, (uint32_t) buffer, size);

    tlk_dma_chn_transfer_start(tlk_i2c0_dma_config.rx_channel);

    tlk_i2c0_master_set_len(size);
    tlk_i2c0_reg.command &= ~TLK_I2C_CMD_STOP;
    tlk_i2c0_reg.command = TLK_I2C_CMD_START | TLK_I2C_CMD_ID | TLK_I2C_CMD_DATAR |
                           TLK_I2C_CMD_ID_R | (tlk_i2c0.send_stop ? TLK_I2C_CMD_STOP : 0);

    while (tlk_i2c0_reg.master_bit.busy)
    {
        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

#else

    tlk_i2c0_irq_clr(TLK_I2C0_RX_BUF_IRQ);
    tlk_i2c0_reg.id                         = (id << 1) | TLK_I2C_WRITE_READ_BIT;
    tlk_i2c0_reg.control3_bit.master_nak_en = 1;

    tlk_i2c0_reg.command = TLK_I2C_CMD_START | TLK_I2C_CMD_ID;

    while (tlk_i2c0_reg.master_bit.busy)
    {
        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

    if (tlk_i2c0_irq_get(TLK_I2C0_MASTER_NAK_IRQ))
    {
        return tlk_i2c0_handle_nak(start_tick, timeout_us);
    }

    tlk_i2c0_master_set_len(size);

    tlk_i2c0_reg.command &= ~TLK_I2C_CMD_STOP;
    tlk_i2c0_reg.command =
        TLK_I2C_CMD_DATAR | TLK_I2C_CMD_ID_R | (tlk_i2c0.send_stop ? TLK_I2C_CMD_STOP : 0);

    uint32_t count = 0;
    while (count < size)
    {
        if (tlk_i2c0_reg.buff_count_bit.rx_buf_cnt > 0)
        {
            buffer[count] = tlk_i2c0_reg.data_buff[count % 4];
            count++;
        }

        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

    while (tlk_i2c0_reg.master_bit.busy)
    {
        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

#endif

    return TLK_I2C_OK;
}

#endif

#if TLK_IS_ENABLED(CONFIG_TLK_I2C1)

static enum tlk_i2c_status tlk_i2c1_handle_nack(uint32_t start_tick, uint32_t timeout_us)
{
    tlk_i2c1_reg.command = TLK_I2C_CMD_STOP;

    while (tlk_i2c1_reg.master_bit.busy)
    {
        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

    return TLK_I2C_NACK;
}

static enum tlk_i2c_status tlk_i2c1_master_write(tlk_i2c_address_t id, uint8_t* buffer,
                                                 uint32_t size)
{
    uint32_t start_tick = tlk_api_time_get_micros();
    uint32_t timeout_us = tlk_i2c1.master.timeout_us;

    tlk_i2c1_reg.id      = id << 1;
    tlk_i2c1_reg.command = TLK_I2C_CMD_ID | TLK_I2C_CMD_START;

    while (tlk_i2c1_reg.master_bit.busy)
    {
        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

    if (tlk_i2c1_reg.master_bit.ack_in)
    {
        return tlk_i2c1_handle_nack(start_tick, timeout_us);
    }

    for (uint32_t count = 0; count < size; count++)
    {
        tlk_i2c1_reg.data_write = buffer[count];
        tlk_i2c1_reg.command    = TLK_I2C_CMD_DATAW;

        while (tlk_i2c1_reg.master_bit.busy)
        {
            TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
        }

        if (tlk_i2c1_reg.master_bit.ack_in)
        {
            return tlk_i2c1_handle_nack(start_tick, timeout_us);
        }
    }

    tlk_i2c1_reg.command = (tlk_i2c1.send_stop ? TLK_I2C_CMD_STOP : 0);

    while (tlk_i2c1_reg.master_bit.busy)
    {
        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

    return TLK_I2C_OK;
}

static enum tlk_i2c_status tlk_i2c1_master_read(tlk_i2c_address_t id, uint8_t* buffer,
                                                uint32_t size)
{
    uint32_t start_tick = tlk_api_time_get_micros();
    uint32_t timeout_us = tlk_i2c1.master.timeout_us;

    tlk_i2c1_reg.id      = (id << 1) | TLK_I2C_WRITE_READ_BIT;
    tlk_i2c1_reg.command = TLK_I2C_CMD_ID | TLK_I2C_CMD_START;

    while (tlk_i2c1_reg.master_bit.busy)
    {
        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

    if (tlk_i2c1_reg.master_bit.ack_in)
    {
        return tlk_i2c1_handle_nack(start_tick, timeout_us);
    }

    uint32_t count = 0;
    while (--size)
    {
        tlk_i2c1_reg.command = TLK_I2C_CMD_DATAR | TLK_I2C_CMD_ID_R;

        while (tlk_i2c1_reg.master_bit.busy)
        {
            TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
        }

        buffer[count] = tlk_i2c1_reg.data_read;
        count++;
    }

    tlk_i2c1_reg.command = TLK_I2C_CMD_DATAR | TLK_I2C_CMD_ID_R | TLK_I2C_CMD_ACK;

    while (tlk_i2c1_reg.master_bit.busy)
    {
        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

    buffer[count] = tlk_i2c1_reg.data_read;

    tlk_i2c1_reg.command = (tlk_i2c1.send_stop ? TLK_I2C_CMD_STOP : 0);

    while (tlk_i2c1_reg.master_bit.busy)
    {
        TLK_I2C_CHECK_TIMEOUT(start_tick, timeout_us);
    }

    return TLK_I2C_OK;
}

#endif

/* I2C driver API */

enum tlk_i2c_status tlk_i2c_pinmux_configure(struct tlk_i2c_device*        dev,
                                             struct tlk_i2c_pinmux_config* config)
{
    tlk_gpio_configure(
        config->scl_port_pin.port, config->scl_port_pin.pin, TLK_GPIO_INPUT_PULL_UP_10K);
    tlk_gpio_configure(
        config->sda_port_pin.port, config->sda_port_pin.pin, TLK_GPIO_INPUT_PULL_UP_10K);

    tlk_gpio_set_mux(config->scl_port_pin.port, config->scl_port_pin.pin, dev->scl_mux);
    tlk_gpio_set_mux(config->sda_port_pin.port, config->sda_port_pin.pin, dev->sda_mux);

    tlk_gpio_disable(config->scl_port_pin.port, config->scl_port_pin.pin);
    tlk_gpio_disable(config->sda_port_pin.port, config->sda_port_pin.pin);

#if TLK_IS_ENABLED(CONFIG_TLK_I2C_PM_DEVICE)
    if (dev->retention != NULL)
    {
        dev->retention->pinmux = *config;
    }
#endif

    return TLK_I2C_OK;
}

enum tlk_i2c_status tlk_i2c_master_configure(struct tlk_i2c_device*        dev,
                                             struct tlk_i2c_master_config* config)
{
    uint8_t clock_speed = tlk_sys_clk.pclk * 1000 * 1000 / (4 * config->clock_speed);

#if TLK_IS_ENABLED(CONFIG_TLK_I2C0)
    if (dev == &tlk_i2c0)
    {
        tlk_reset_reg.reset0_bit.i2c    = 1;
        tlk_clock_reg.clock_en0_bit.i2c = 1;

        tlk_i2c0_reg.control2_bit.master_en      = 1;
        tlk_i2c0_reg.control2_bit.clk_stretch_en = config->stretch_en;
        tlk_i2c0_reg.control2_bit.nak_stop_en    = 1; // ???

        tlk_i2c0_reg.clock_speed        = clock_speed;
        tlk_clock_reg.clock_en0_bit.i2c = 1; // ???

#if TLK_IS_ENABLED(CONFIG_TLK_I2C0_DMA)
        if (tlk_i2c0_dma_config.tx_channel == TLK_DMA_NONE)
        {
            tlk_i2c0_dma_config.tx_channel = tlk_dma_chn_request();
            tlk_dma_chn_configure(tlk_i2c0_dma_config.tx_channel, &tlk_i2c0_dma_config.tx_config);
        }
        if (tlk_i2c0_dma_config.rx_channel == TLK_DMA_NONE)
        {
            tlk_i2c0_dma_config.rx_channel = tlk_dma_chn_request();
            tlk_dma_chn_configure(tlk_i2c0_dma_config.rx_channel, &tlk_i2c0_dma_config.rx_config);
        }

        tlk_i2c0_irq_enable(TLK_I2C0_MASTER_NAK_IRQ);
        tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_I2C);
#endif
    }
#endif
#if TLK_IS_ENABLED(CONFIG_TLK_I2C1)
    if (dev == &tlk_i2c1)
    {
        tlk_reset_reg.reset2_bit.i2c1    = 1;
        tlk_clock_reg.clock_en2_bit.i2c1 = 1;

        tlk_i2c1_reg.control_bit.master_en      = 1;
        tlk_i2c1_reg.control_bit.clk_stretch_en = config->stretch_en;

        tlk_i2c1_reg.clock_speed = clock_speed;
    }
#endif

    dev->role = TLK_I2C_MASTER;

#if TLK_IS_ENABLED(CONFIG_TLK_I2C_PM_DEVICE)
    if (dev->retention != NULL)
    {
        dev->retention->master_config = *config;
    }
#endif

    return TLK_I2C_OK;
}

enum tlk_i2c_status tlk_i2c_master_write(struct tlk_i2c_device* dev, tlk_i2c_address_t id,
                                         uint8_t* buffer, uint32_t size)
{
#if TLK_IS_ENABLED(CONFIG_TLK_I2C0)
    if (dev == &tlk_i2c0)
    {
        return tlk_i2c0_master_write(id, buffer, size);
    }
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_I2C1)
    if (dev == &tlk_i2c1)
    {
        return tlk_i2c1_master_write(id, buffer, size);
    }
#endif

    return TLK_I2C_UNSUPPORTED;
}

enum tlk_i2c_status tlk_i2c_master_read(struct tlk_i2c_device* dev, tlk_i2c_address_t id,
                                        uint8_t* buffer, uint32_t size)
{
#if TLK_IS_ENABLED(CONFIG_TLK_I2C0)
    if (dev == &tlk_i2c0)
    {
        return tlk_i2c0_master_read(id, buffer, size);
    }
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_I2C1)
    if (dev == &tlk_i2c1)
    {
        return tlk_i2c1_master_read(id, buffer, size);
    }
#endif

    return TLK_I2C_UNSUPPORTED;
}

enum tlk_i2c_status tlk_i2c_master_xfer(struct tlk_i2c_device* dev, tlk_i2c_address_t id,
                                        struct tlk_i2c_message* messages, uint8_t count)
{
    enum tlk_i2c_status status = TLK_I2C_OK;

    dev->send_stop = 0;

    for (uint8_t i = 0; i < count; i++)
    {
        if (i == count - 1)
        {
            dev->send_stop = 1;
        }

        if (messages[i].is_read)
        {
            status = tlk_i2c_master_read(dev, id, messages[i].buffer, messages[i].size);
        }
        else
        {
            status = tlk_i2c_master_write(dev, id, messages[i].buffer, messages[i].size);
        }

        if (status != TLK_I2C_OK)
        {
            dev->send_stop = 1;
            break;
        }
    }

    return status;
}

enum tlk_i2c_status tlk_i2c_master_set_timeout(struct tlk_i2c_device* dev, uint32_t timeout_us)
{
    dev->master.timeout_us = timeout_us;

    return TLK_I2C_OK;
}

enum tlk_i2c_status tlk_i2c_slave_configure(__unused struct tlk_i2c_device*       dev,
                                            __unused struct tlk_i2c_slave_config* config)
{
#if TLK_IS_ENABLED(CONFIG_TLK_I2C0_SLAVE)
    if (dev == &tlk_i2c0)
    {
        tlk_reset_reg.reset0_bit.i2c    = 1;
        tlk_clock_reg.clock_en0_bit.i2c = 1;

        tlk_i2c0_reg.control2_bit.master_en = 0;
        tlk_i2c0_reg.id                     = config->address << 1;

        tlk_i2c0_reg.trig_bit.rx_irq_trig_lvl = TLK_I2C_SLAVE_RX_IRQ_TRIG_LEVEL;

        tlk_i2c0_irq_enable(TLK_I2C0_RX_BUF_IRQ | TLK_I2C0_RX_END_IRQ | TLK_I2C0_TX_END_IRQ |
                            TLK_I2C0_SLAVE_WR_IRQ);
        tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_I2C);

        tlk_i2c0_reg.control3_bit.slave_clk_stretch_en = config->stretch_en;

        tlk_i2c0.role = TLK_I2C_SLAVE;

#if TLK_IS_ENABLED(CONFIG_TLK_I2C0_PM_DEVICE)
        if (tlk_i2c0.retention != NULL)
        {
            tlk_i2c0.retention->slave_config = *config;
        }
#endif

        return TLK_I2C_OK;
    }
#endif

    TLK_LOG_WARN(i2c_driver, "tlk_i2c_slave_configure has been called on the unsupported device.");

    return TLK_I2C_UNSUPPORTED;
}

enum tlk_i2c_status tlk_i2c_slave_set_rx(__unused struct tlk_i2c_device* dev,
                                         __unused uint8_t* buffer, __unused uint32_t size)
{
#if TLK_IS_ENABLED(CONFIG_TLK_I2C0_SLAVE)
    if (dev == &tlk_i2c0)
    {
        tlk_i2c0_slave.rx_buffer      = buffer;
        tlk_i2c0_slave.rx_buffer_size = size;

        return TLK_I2C_OK;
    }
#endif

    return TLK_I2C_UNSUPPORTED;
}

enum tlk_i2c_status tlk_i2c_slave_set_tx(__unused struct tlk_i2c_device* dev,
                                         __unused uint8_t* buffer, __unused uint32_t size)
{
#if TLK_IS_ENABLED(CONFIG_TLK_I2C0_SLAVE)
    if (dev == &tlk_i2c0)
    {
        tlk_i2c0_slave.tx_buffer      = buffer;
        tlk_i2c0_slave.tx_buffer_size = size;

        return TLK_I2C_OK;
    }
#endif

    return TLK_I2C_UNSUPPORTED;
}

enum tlk_i2c_status tlk_i2c_slave_set_handler(__unused struct tlk_i2c_device*        dev,
                                              __unused tlk_i2c_slave_event_handler_t handler)
{
#if TLK_IS_ENABLED(CONFIG_TLK_I2C0_SLAVE)
    if (dev == &tlk_i2c0)
    {
        tlk_i2c0_slave.handler = handler;

        return TLK_I2C_OK;
    }
#endif

    return TLK_I2C_UNSUPPORTED;
}

#if TLK_IS_ENABLED(CONFIG_TLK_I2C0)

#if TLK_IS_ENABLED(CONFIG_TLK_I2C0_SLAVE)

static void tlk_i2c0_slave_read(uint8_t* buffer, uint32_t size)
{
    uint32_t count = 0;
    while (count < size)
    {
        if (tlk_i2c0_reg.buff_count_bit.rx_buf_cnt > 0)
        {
            buffer[count]     = tlk_i2c0_reg.data_buff[tlk_i2c0.rx_index];
            tlk_i2c0.rx_index = (tlk_i2c0.rx_index + 1) % 4;
            count++;
        }
    }
}

static void tlk_i2c0_slave_write(uint8_t* buffer, uint32_t size)
{
    tlk_i2c0_irq_clr(TLK_I2C0_TX_BUF_IRQ);
    uint32_t count = 0;
    while (count < size)
    {
        if (tlk_i2c0_reg.buff_count_bit.tx_buf_cnt < 8)
        {
            tlk_i2c0_reg.data_buff[count % 4] = buffer[count];
            count++;
        }
    }
}

#endif

_tlk_attribute_ram_code_sec_noinline_ void tlk_i2c_irq_handle(void)
{
#if TLK_IS_ENABLED(CONFIG_TLK_I2C0_MASTER_DMA)
    if (tlk_i2c0_irq_get(TLK_I2C0_MASTER_NAK_IRQ))
    {
        tlk_i2c0_irq_clr(TLK_I2C0_MASTER_NAK_IRQ);

        tlk_i2c0_reg.command = TLK_I2C_CMD_STOP;
        while (tlk_i2c0_reg.master_bit.busy)
        {
        }
        tlk_dma_chn_transfer_abort(tlk_i2c0_dma_config.tx_channel);

        if (!tlk_i2c0_reg.command_bit.en_read_id)
        {
            tlk_i2c0_irq_clr(TLK_I2C0_TX_BUF_IRQ);
        }
    }
#endif
#if TLK_IS_ENABLED(CONFIG_TLK_I2C0_SLAVE)
    tlk_i2c_slave_event_handler_t handler = tlk_i2c0_slave.handler;

    if (tlk_i2c0_irq_get(TLK_I2C0_SLAVE_WR_IRQ))
    {
        tlk_i2c0_irq_clr(TLK_I2C0_SLAVE_WR_IRQ);
        tlk_i2c0_slave.rx_count = 0;

        if (handler != NULL)
        {
            if (tlk_i2c0_reg.status1_bit.slave_rw_status)
            {
                handler(&tlk_i2c0, TLK_I2C_EVENT_READ_REQUEST);
                tlk_i2c0_slave_write(tlk_i2c0_slave.tx_buffer, tlk_i2c0_slave.tx_buffer_size);
            }
            else
            {
                handler(&tlk_i2c0, TLK_I2C_EVENT_WRITE_REQUEST);
            }
        }
    }
    if (tlk_i2c0_irq_get(TLK_I2C0_RX_BUF_IRQ))
    {
        tlk_i2c0_slave_read(&tlk_i2c0_slave.rx_buffer[tlk_i2c0_slave.rx_count],
                            TLK_I2C_SLAVE_RX_IRQ_TRIG_LEVEL);
        tlk_i2c0_slave.rx_count += TLK_I2C_SLAVE_RX_IRQ_TRIG_LEVEL;
    }
    if ((tlk_i2c0_irq_get(TLK_I2C0_RX_END_IRQ)))
    {
        tlk_i2c0_irq_clr(TLK_I2C0_RX_END_IRQ);
        if (tlk_i2c0_reg.buff_count_bit.rx_buf_cnt > 0)
        {
            tlk_i2c0_slave_read(&tlk_i2c0_slave.rx_buffer[tlk_i2c0_slave.rx_count],
                                tlk_i2c0_reg.buff_count_bit.rx_buf_cnt);
        }
        tlk_i2c0_slave.rx_count = 0;

        if (handler != NULL)
        {
            handler(&tlk_i2c0, TLK_I2C_EVENT_STOP);
        }
    }
    if (tlk_i2c0_irq_get(TLK_I2C0_TX_END_IRQ))
    {
        tlk_i2c0_irq_clr(TLK_I2C0_TX_END_IRQ);

        if (handler != NULL)
        {
            handler(&tlk_i2c0, TLK_I2C_EVENT_STOP);
        }
    }
#endif
}

TLK_PLIC_ISR_REGISTER(tlk_i2c_irq_handle, UNISDK_PLIC_IRQ_NUM_I2C)

#endif

#if TLK_IS_ENABLED(CONFIG_TLK_I2C_PM_DEVICE)

#include "common/include/tlk_init.h"

#define TLK_DECLARE_I2C_RESTORE_CONTEXT(num)                                                       \
    static void tlk_i2c##num##_restore_context(void)                                               \
    {                                                                                              \
        tlk_i2c_pinmux_configure(&tlk_i2c##num, &tlk_i2c##num.retention->pinmux);                  \
        if (tlk_i2c##num.role == TLK_I2C_MASTER)                                                   \
        {                                                                                          \
            tlk_i2c_master_configure(&tlk_i2c##num, &tlk_i2c##num.retention->master_config);       \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            tlk_i2c_slave_configure(&tlk_i2c##num, &tlk_i2c##num.retention->slave_config);         \
        }                                                                                          \
    }                                                                                              \
    TLK_REGISTER_AFTER_SLEEP(                                                                      \
        tlk_i2c##num##_restore_context, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)

#define TLK_DECLARE_I2C_RESTORE_CONTEXT_IF_ENABLED(num)                                            \
    TLK_IF_ENABLED(CONFIG_TLK_I2C##num##_PM_DEVICE, (TLK_DECLARE_I2C_RESTORE_CONTEXT(num)))

TLK_FOR_CALL(UNISDK_I2C_COUNT, TLK_DECLARE_I2C_RESTORE_CONTEXT_IF_ENABLED)

#endif

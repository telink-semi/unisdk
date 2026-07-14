#include "core/include/tlk_spi.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_clock.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_plic.h"
#include "properties/tlk_plic.h"
#include "registers/tlk_chip.h"
#include "registers/tlk_clock.h"
#include "registers/tlk_spi.h"

#define TLK_SPI_TIMEOUT_US 1000

#define TLK_DECLARE_SPI_MODULE(num) TLK_SPI##num,

enum tlk_spi_module
{
    TLK_FOR_CALL(UNISDK_SPI_COUNT, TLK_DECLARE_SPI_MODULE)
};

enum tlk_spi_irq_status
{
    TLK_SPI_IRQ_RXFIFO_OR = TLK_BIT(0),
    TLK_SPI_IRQ_TXFIFO_UR = TLK_BIT(1),
    TLK_SPI_IRQ_RXFIFO    = TLK_BIT(2),
    TLK_SPI_IRQ_TXFIFO    = TLK_BIT(3),
    TLK_SPI_IRQ_END       = TLK_BIT(4),
    TLK_SPI_IRQ_CMD       = TLK_BIT(5),
};

struct tlk_spi_device
{
    const enum tlk_spi_module     module;
    tlk_spi_slave_event_handler_t handler;
};

struct tlk_spi_device tlk_spi0 = {
    .module = TLK_SPI0 // gspi
};

/* --- config dependencies --- */
void tlk_gspi_set_pin_mux(struct tlk_gpio_port_pin port_pin, enum tlk_gpio_mux function)
{
    if (function == TLK_GPIO_MUX_GSPI_CS)
    {
        tlk_gpio_configure(port_pin.port, port_pin.pin, TLK_GPIO_INPUT_PULL_UP);
    }
    else
    {
        tlk_gpio_configure(port_pin.port, port_pin.pin, TLK_GPIO_INPUT_NO_PULL);
    }

    tlk_gpio_disable(port_pin.port, port_pin.pin);
    tlk_gpio_set_mux(port_pin.port, port_pin.pin, function);
}

static inline void tlk_spi_rx_irq_trig_cnt(enum tlk_spi_module module, uint8_t cnt)
{
    tlk_spi_reg(module).rxfifo_thres = cnt;
}

static inline void tlk_spi_tx_irq_trig_cnt(enum tlk_spi_module module, uint8_t cnt)
{
    tlk_spi_reg(module).txfifo_thres = cnt;
}

static inline void tlk_spi_set_slave_irq_trigs(enum tlk_spi_module module)
{
    tlk_spi_rx_irq_trig_cnt(module, 4); // default gspi burst1(N=1) ,rx_fifo trig_cnt=4*N=4.
    tlk_spi_tx_irq_trig_cnt(
        module, 4); // default gspi burst1(N=1) ,tx_fifo trig_cnt=rx fifo depth-4*N=8-4=4.
}

void tlk_spi_set_transmode(enum tlk_spi_module module, enum tlk_spi_control2_transmode_e mode)
{
    tlk_spi_reg(module).control2_bit.transmode = mode;
}

void tlk_spi_set_io_mode(enum tlk_spi_module module, enum tlk_spi_control1_data_lane_io_mode_e mode,
                         bool is_3line)
{
    tlk_spi_reg(module).control3_bit.spi_3line = is_3line;
    tlk_spi_reg(module).control1_bit.data_lane_io_mode =
        is_3line ? TLK_SPI_CONTROL1_DATA_LANE_IO_MODE_SINGLE : mode;
}

static inline void tlk_spi_set_irq_mask(enum tlk_spi_module module, enum tlk_spi_irq_status status)
{
    tlk_spi_reg(module).control0 |= status;
}

static inline uint8_t tlk_spi_get_irq_status(enum tlk_spi_module       module,
                                             enum tlk_spi_int_status_e status)
{
    return tlk_spi_reg(module).int_status & status;
}

static inline void tlk_spi_clr_irq_status(enum tlk_spi_module       module,
                                          enum tlk_spi_int_status_e status)
{
    tlk_spi_reg(module).int_status = status;
}
/* --- config dependencies --- */

/* --- other dependencies --- */
static inline bool tlk_spi_is_busy(enum tlk_spi_module module)
{
    return tlk_spi_reg(module).status_bit.busy;
}
/* --- other dependencies --- */

/* --- write dependencies --- */
static inline void tlk_spi_tx_fifo_clr(enum tlk_spi_module module)
{
    tlk_spi_reg(module).status_bit.txfifo_clr_level         = 1;
    tlk_spi_reg(module).control3_bit.dmatx_sof_clrtxfifo_en = 0;
    while (tlk_spi_reg(module).status_bit.txfifo_clr_level)
        ;
}

bool tlk_spi_txfifo_num_is_word(enum tlk_spi_module module)
{
    return (20 - (tlk_spi_reg(module).txfifo_status_bit.txfifo_entries) < 4);
}

bool tlk_spi_txfifo_is_full(enum tlk_spi_module module)
{
    return tlk_spi_reg(module).txfifo_status_bit.txfifo_full;
}

enum tlk_spi_status tlk_spi_write(struct tlk_spi_device* dev, const uint8_t* buffer, uint32_t size)
{
    uint32_t word_count = size / 4;
    uint8_t  tail_count = size % 4;

    // The tx _fifo depth of the gspi is 8 bytes. When the remaining size in the tx_fifo is not less
    // than 4 bytes, the MCU will move the data according to the word length.
    for (uint32_t i = 0; i < word_count; i++)
    {
        TLK_WAIT_FOR_TRUE_OR_TIMEOUT((!tlk_spi_txfifo_num_is_word(dev->module)),
                                     (TLK_SPI_TIMEOUT_US),
                                     (return TLK_SPI_TIMEOUT;))

        *(volatile uint32_t*) tlk_spi_reg(dev->module).wr_rd_data = ((const uint32_t*) buffer)[i];
    }

    // When the remaining size in tx_fifo is less than 4 bytes, the MCU moves the data according to
    // the word length.
    for (uint8_t i = 0; i < tail_count; i++)
    {
        TLK_WAIT_FOR_TRUE_OR_TIMEOUT(
            (!tlk_spi_txfifo_is_full(dev->module)), (TLK_SPI_TIMEOUT_US), (return TLK_SPI_TIMEOUT;))

        tlk_spi_reg(dev->module).wr_rd_data[i] = buffer[(word_count * 4) + i];
    }

    return TLK_SPI_OK;
}
/* --- write dependencies --- */

/* --- read dependencies --- */
static inline void tlk_spi_rx_fifo_clr(enum tlk_spi_module module)
{
    tlk_spi_reg(module).status_bit.rxfifo_clr_level         = 1;
    tlk_spi_reg(module).control3_bit.dmarx_eof_clrrxfifo_en = 0;
    while (tlk_spi_reg(module).status_bit.rxfifo_clr_level)
        ;
}

static inline uint8_t tlk_spi_get_rxfifo_num(enum tlk_spi_module module)
{
    return tlk_spi_reg(module).rxfifo_status_bit.rxfifo_entries;
}

bool tlk_spi_rxfifo_num_is_word(enum tlk_spi_module module)
{
    return tlk_spi_reg(module).rxfifo_status_bit.rxfifo_entries < 4;
}

bool tlk_spi_rxfifo_is_empty(enum tlk_spi_module module)
{
    return tlk_spi_reg(module).rxfifo_status_bit.rxfifo_empty;
}

enum tlk_spi_status tlk_spi_read(struct tlk_spi_device* dev, uint8_t* buffer, uint32_t size)
{
    uint32_t word_count = size / 4;
    uint8_t  tail_count = size % 4;

    // When the data size in rx_fifo is not less than 4 bytes, the MCU moves the data according to
    // the word length
    for (uint32_t i = 0; i < word_count; i++)
    {
        TLK_WAIT_FOR_TRUE_OR_TIMEOUT((!tlk_spi_rxfifo_num_is_word(dev->module)),
                                     (TLK_SPI_TIMEOUT_US),
                                     (return TLK_SPI_TIMEOUT;))

        ((uint32_t*) buffer)[i] = *(volatile uint32_t*) tlk_spi_reg(dev->module).wr_rd_data;
    }

    // When the data size in rx_fifo is less than 4 bytes, the MCU moves the data according to the
    // word length
    for (uint8_t i = 0; i < tail_count; i++)
    {
        TLK_WAIT_FOR_TRUE_OR_TIMEOUT((!tlk_spi_rxfifo_is_empty(dev->module)),
                                     (TLK_SPI_TIMEOUT_US),
                                     (return TLK_SPI_TIMEOUT;))

        buffer[(word_count * 4) + i] = tlk_spi_reg(dev->module).wr_rd_data[i];
    }

    return TLK_SPI_OK;
}
/* --- read dependencies --- */

/* --- IRQ --- */
_tlk_attribute_ram_code_sec_ void tlk_spi_irq_handler(struct tlk_spi_device* dev)
{
    if (tlk_spi_get_irq_status(dev->module, TLK_SPI_INT_STATUS_SLAVE_CMD_INT_STUS))
    {
        tlk_spi_clr_irq_status(dev->module, TLK_SPI_INT_STATUS_SLAVE_CMD_INT_STUS);

        if (dev->handler != NULL)
        {
            dev->handler(dev, tlk_spi_reg(dev->module).command);
        }
    }
}

_tlk_attribute_ram_code_sec_ void tlk_gspi_irq_handler(void)
{
    tlk_spi_irq_handler(&tlk_spi0);
}

TLK_PLIC_ISR_REGISTER(tlk_gspi_irq_handler, UNISDK_PLIC_IRQ_NUM_GSPI);
/* --- IRQ --- */

/* SPI driver API */

enum tlk_spi_status tlk_spi_configure_pinmux(struct tlk_spi_device* dev, struct tlk_spi_pinmux* mux)
{
    (void) dev;

    tlk_gspi_set_pin_mux(mux->cs, TLK_GPIO_MUX_GSPI_CS);
    tlk_gspi_set_pin_mux(mux->sck, TLK_GPIO_MUX_GSPI_SCK);
    tlk_gspi_set_pin_mux(mux->mosi, TLK_GPIO_MUX_GSPI_MOSI);
    tlk_gspi_set_pin_mux(mux->miso, TLK_GPIO_MUX_GSPI_MISO);

    return TLK_SPI_OK;
}

enum tlk_spi_status tlk_spi_master_configure(struct tlk_spi_device* dev, struct tlk_spi_config* cfg)
{
    // TODO: check if pll is enabled
    tlk_reset_reg.reset1_bit.gspi    = 1;
    tlk_clock_reg.clock_en1_bit.gspi = 1;

    // GSPI clock source select pll_clk when div_clock <= 255,
    // select xtl 24m when div_clock > 255.
    uint16_t div_clock = tlk_sys_clk.source_clk * 1000000 / cfg->frequency;

    if (div_clock > 255)
    {
        tlk_clock_reg.gspi_mode_bit.gspi_mod        = 24 * 1000000 / cfg->frequency;
        tlk_clock_reg.gspi_mode_bit.gspi_div_in_sel = TLK_CLOCK_GSPI_MODE_GSPI_DIV_IN_SEL_24M_XTAL;
    }
    else
    {
        tlk_clock_reg.gspi_mode_bit.gspi_mod        = div_clock;
        tlk_clock_reg.gspi_mode_bit.gspi_div_in_sel = TLK_CLOCK_GSPI_MODE_GSPI_DIV_IN_SEL_PLL;
    }

    tlk_spi_reg(dev->module).control3 |=
        TLK_SPI_CONTROL3_SPI_MASTER | TLK_SPI_CONTROL3_AUTO_HREADY_EN;
    tlk_spi_reg(dev->module).control3_bit.spi_mode = (enum tlk_spi_control3_spi_mode_e) cfg->mode;

    tlk_spi_set_transmode(dev->module, TLK_SPI_CONTROL2_TRANSMODE_WRITE_ONLY);

    // set io mode
    tlk_spi_set_io_mode(dev->module, cfg->io_mode & 0x03, cfg->io_mode == TLK_SPI_3_LINE_MODE);

    // config
    tlk_spi_reg(dev->module).control4_bit.xip_enable = 0;
    tlk_spi_reg(dev->module).control1_bit.cmd_en     = 0;
    tlk_spi_reg(dev->module).control1_bit.addr_en    = 0;

    return TLK_SPI_OK;
}

enum tlk_spi_status tlk_spi_master_xfer(struct tlk_spi_device* dev, struct tlk_spi_xfer* xfer)
{
    enum tlk_spi_status status = TLK_SPI_OK;

    tlk_spi_tx_fifo_clr(dev->module);
    tlk_spi_rx_fifo_clr(dev->module);

    tlk_spi_reg(dev->module).tx_count_bit.count = xfer->tx_size - 1;
    tlk_spi_reg(dev->module).rx_count_bit.count = xfer->rx_size - 1;

    tlk_spi_set_transmode(dev->module, (enum tlk_spi_control2_transmode_e) xfer->type);

    tlk_spi_reg(dev->module).control1_bit.cmd_en = xfer->use_cmd;
    tlk_spi_reg(dev->module).command             = xfer->cmd;

    switch (xfer->type)
    {
    case TLK_SPI_NO_DATA:
        break;
    case TLK_SPI_TRANSCEIVE:
    {
        uint32_t size       = xfer->tx_size;
        uint8_t  chunk_size = 8;

        for (uint32_t i = 0; i < size; i = i + chunk_size)
        {
            if (chunk_size > (size - i))
            {
                chunk_size = size - i;
            }

            tlk_spi_write(dev, xfer->tx_buffer + i, chunk_size);

            if (size < 8)
            {
                // read all if less than chunk
                tlk_spi_read(dev, xfer->rx_buffer, chunk_size);
            }
            else if (i == 0)
            {
                // the first read is 1 byte less then chunk
                tlk_spi_read(dev, xfer->rx_buffer, chunk_size - 1);
            }
            else if ((size - i) > 8)
            {
                // the middle reads should be with shifted index
                tlk_spi_read(dev, xfer->rx_buffer + i - 1, chunk_size);
            }
            else
            {
                // the last read should read 1 byte more
                tlk_spi_read(dev, xfer->rx_buffer + i - 1, chunk_size + 1);
            }

            tlk_spi_rx_fifo_clr(dev->module);
            tlk_spi_tx_fifo_clr(dev->module);
        }

        break;
    }
    case TLK_SPI_TX:
    case TLK_SPI_DUMMY_TX:
    {
        status = tlk_spi_write(dev, xfer->tx_buffer, xfer->tx_size);

        if (status != TLK_SPI_OK)
        {
            return status;
        }

        break;
    }
    case TLK_SPI_RX:
    case TLK_SPI_DUMMY_RX:
    {
        status = tlk_spi_read(dev, xfer->rx_buffer, xfer->rx_size);

        if (status != TLK_SPI_OK)
        {
            return status;
        }

        break;
    }
    case TLK_SPI_TX_THEN_RX:
    case TLK_SPI_TX_DUMMY_RX:
    {
        status = tlk_spi_write(dev, xfer->tx_buffer, xfer->tx_size);

        if (status != TLK_SPI_OK)
        {
            return status;
        }

        status = tlk_spi_read(dev, xfer->rx_buffer, xfer->rx_size);

        if (status != TLK_SPI_OK)
        {
            return status;
        }

        break;
    }
    case TLK_SPI_RX_THEN_TX:
    case TLK_SPI_RX_DUMMY_TX:
    {
        status = tlk_spi_read(dev, xfer->rx_buffer, xfer->rx_size - 1);

        if (status != TLK_SPI_OK)
        {
            return status;
        }

        status = tlk_spi_write(dev, xfer->tx_buffer, xfer->tx_size);

        if (status != TLK_SPI_OK)
        {
            return status;
        }

        break;
    }
    }

    TLK_WAIT_FOR_TRUE_OR_TIMEOUT(
        (!tlk_spi_is_busy(dev->module)), (TLK_SPI_TIMEOUT_US), (return TLK_SPI_TIMEOUT;))

    return TLK_SPI_OK;
}

enum tlk_spi_status tlk_spi_slave_configure(struct tlk_spi_device* dev, struct tlk_spi_config* cfg)
{
    tlk_reset_reg.reset1_bit.gspi    = 1;
    tlk_clock_reg.clock_en1_bit.gspi = 1;

    tlk_clock_reg.gspi_mode_bit.gspi_mod        = tlk_sys_clk.source_clk / 48;
    tlk_clock_reg.gspi_mode_bit.gspi_div_in_sel = TLK_CLOCK_GSPI_MODE_GSPI_DIV_IN_SEL_PLL;

    tlk_spi_reg(dev->module).control3_bit.spi_master     = 0;
    tlk_spi_reg(dev->module).control3_bit.auto_hready_en = 1;
    tlk_spi_reg(dev->module).control3_bit.spi_mode = (enum tlk_spi_control3_spi_mode_e) cfg->mode;

    tlk_spi_set_slave_irq_trigs(dev->module);
    tlk_spi_set_transmode(dev->module, TLK_SPI_CONTROL2_TRANSMODE_WRITE_AND_READ);

    tlk_spi_clr_irq_status(dev->module, (uint8_t) TLK_SPI_IRQ_CMD);
    tlk_spi_set_irq_mask(dev->module, TLK_SPI_IRQ_CMD);

    tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_GSPI);

    // set io mode
    tlk_spi_set_io_mode(dev->module, cfg->io_mode & 0x03, cfg->io_mode == TLK_SPI_3_LINE_MODE);

    // config
    tlk_spi_reg(dev->module).control4_bit.xip_enable = 0;
    tlk_spi_reg(dev->module).control1_bit.cmd_en     = 1;
    tlk_spi_reg(dev->module).control1_bit.addr_en    = 0;

    return TLK_SPI_OK;
}

enum tlk_spi_status tlk_spi_slave_set_handler(struct tlk_spi_device*        dev,
                                              tlk_spi_slave_event_handler_t handler)
{
    dev->handler = handler;

    return TLK_SPI_OK;
}

enum tlk_spi_status tlk_spi_slave_write(struct tlk_spi_device* dev, uint8_t* buffer, uint32_t size)
{
    tlk_spi_tx_fifo_clr(dev->module);
    return tlk_spi_write(dev, buffer, size);
}

enum tlk_spi_status tlk_spi_slave_read(struct tlk_spi_device* dev, uint8_t* buffer, uint32_t size)
{
    tlk_spi_rx_fifo_clr(dev->module);
    return tlk_spi_read(dev, buffer, size);
}

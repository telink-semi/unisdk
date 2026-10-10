#include "core/include/tlk_core.h"
#include "core/include/tlk_dma.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_i2s.h"
#include "system/debug/log/tlk_log.h"

#define BUFFER_SIZE 1024

TLK_LOG_CREATE(i2s_demo, "I2S_DEMO");

uint16_t tx_buffer[BUFFER_SIZE] __attribute__((aligned(4))) = {1, 2, 3, 4, 5, 6, 7, 8, 9};
uint16_t rx_buffer[BUFFER_SIZE] __attribute__((aligned(4)));

int main(void)
{
    struct tlk_i2s_pinmux pinmux = {
        .bclk       = {TLK_GPIO_PORT_B, TLK_GPIO_PIN_0},
        .adc_lr_clk = {TLK_GPIO_PORT_B, TLK_GPIO_PIN_1},
        .adc_dat    = {TLK_GPIO_PORT_B, TLK_GPIO_PIN_2},
        .dac_lr_clk = {TLK_GPIO_PORT_B, TLK_GPIO_PIN_6},
        .dac_dat    = {TLK_GPIO_PORT_B, TLK_GPIO_PIN_7},
    };

    struct tlk_i2s_config config = {
        .data_width = TLK_I2S_DATA_WIDTH_16_BIT,
        .mode       = TLK_I2S_MODE_I2S,
    };

#if TLK_IS_ENABLED(CONFIG_TLK_I2S_DEMO_ROLE_MASTER)

    config.role        = TLK_I2S_MASTER;
    config.sample_rate = (struct tlk_i2s_sample_rate){8, 625, 0, 64, 64};

#elif TLK_IS_ENABLED(CONFIG_TLK_I2S_DEMO_ROLE_SLAVE)

    config.role = TLK_I2S_SLAVE;

#endif

    struct tlk_dma_ll_node rx_buff0 = {
        .dst_addr = rx_buffer,
        .length   = BUFFER_SIZE,
        .next     = &rx_buff0,
    };

    struct tlk_dma_ll_node tx_buff0 = {
        .src_addr = tx_buffer,
        .length   = BUFFER_SIZE,
        .next     = &tx_buff0,
    };

    struct tlk_i2s_io_config tx_io_config = {
        .ll_head     = &tx_buff0,
        .i2s_channel = TLK_I2S_CHANNEL_LEFT,
        .dma_channel = tlk_dma_chn_request(),
        .fifo        = TLK_I2S_FIFO0,
    };

    struct tlk_i2s_io_config rx_io_config = {
        .ll_head     = &rx_buff0,
        .i2s_channel = TLK_I2S_CHANNEL_LEFT,
        .dma_channel = tlk_dma_chn_request(),
        .fifo        = TLK_I2S_FIFO0,
    };

    struct tlk_i2s_device* dev = &tlk_i2s0;

    tlk_core_interrupt_enable();

    tlk_gpio_configure(PINMUX_LED_0_PORT, PINMUX_LED_0_PIN, TLK_GPIO_OUTPUT);

    tlk_i2s_configure(dev, &config);
    tlk_i2s_pinmux_configure(dev, &pinmux);
    tlk_i2s_configure_output(dev, &tx_io_config);
    tlk_i2s_input_configure(dev, &rx_io_config);

    tlk_i2s_start(dev);

    while (1)
    {
        // Mark the successful transmission with the enabled LED
        // I2S has a known issue that the rx buffer has 3 dummy elements at the beginning
        tlk_gpio_pin_write(PINMUX_LED_0_PORT, PINMUX_LED_0_PIN, tx_buffer[0] == rx_buffer[3]);
    }
}

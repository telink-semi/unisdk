#include "core/include/tlk_audio.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_dma.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_i2s.h"
#include "storage.h"
#include "storage_device.h"
#include "system/debug/benchmark/tlk_benchmark.h"
#include "system/debug/log/tlk_log.h"

#define BUFFER_SIZE  1024
#define BUFFER_COUNT 256
#define STORAGE_SIZE (BUFFER_COUNT * BUFFER_SIZE)

TLK_LOG_CREATE(audio_demo, "AUDIO_DEMO");

const uintptr_t storage_addr =
    UNISDK_CHIP_MEMORY_ROM_STARTADDR + UNISDK_CHIP_MEMORY_ROM_SIZE * 1024 - STORAGE_SIZE;
struct tlk_storage_device* storage;

uint16_t          audio_buffer[2][BUFFER_SIZE / sizeof(uint16_t)] __attribute__((aligned(4)));
volatile uint16_t audio_rx_buffer_index, audio_tx_buffer_index;
volatile bool     is_rx_done, is_tx_done;

void save_buffer(void)
{
    tlk_storage_write(storage_addr + audio_rx_buffer_index * BUFFER_SIZE,
                      audio_buffer[audio_rx_buffer_index % 2],
                      BUFFER_SIZE);

    audio_rx_buffer_index++;
}

void fill_buffer(void)
{
    tlk_storage_read(storage_addr + audio_tx_buffer_index * BUFFER_SIZE,
                     audio_buffer[audio_tx_buffer_index % 2],
                     BUFFER_SIZE);

    audio_tx_buffer_index++;
}

void on_rx_buffer_fill(enum tlk_dma_irq_type irq_type)
{
    if (irq_type != TLK_DMA_IRQ_TC)
    {
        TLK_LOG_ERROR(audio_demo, "rx dma irq status: %d", irq_type);
        return;
    }

    save_buffer();

    if (audio_rx_buffer_index == BUFFER_COUNT)
    {
        tlk_audio_input_disable();
        is_rx_done = 1;
    }
}

void on_tx_buffer_sent(enum tlk_dma_irq_type irq_type)
{
    if (irq_type != TLK_DMA_IRQ_TC)
    {
        TLK_LOG_ERROR(audio_demo, "tx dma irq status: %d", irq_type);
        return;
    }

    fill_buffer();

    if (audio_tx_buffer_index == BUFFER_COUNT)
    {
        tlk_audio_output_disable();
        is_tx_done = 1;
    }
}

int main(void)
{
// TODO: refactor this
#if TLK_IS_ENABLED(CONFIG_TLK_CORE_TL321X)
    struct tlk_audio_sdm_pinmux sdm_pinmux = {
        .sdm0_p = {TLK_GPIO_PORT_D, TLK_GPIO_PIN_5},
        .sdm0_n = {TLK_GPIO_PORT_E, TLK_GPIO_PIN_2},
        .sdm1_p = {TLK_GPIO_PORT_E, TLK_GPIO_PIN_3},
        .sdm1_n = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_0},
    };

    struct tlk_audio_amic_pinmux amic_pinmux = {
        .bias = {TLK_GPIO_PORT_B, TLK_GPIO_PIN_4},
    };

    struct tlk_audio_dmic_pinmux dmic_pinmux = {
        .data = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_2},
        .clk1 = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_3},
        .clk2 = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_4},
    };
#elif TLK_IS_ENABLED(CONFIG_TLK_CORE_TL721X)
    struct tlk_audio_sdm_pinmux sdm_pinmux = {
        .sdm0_p = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_0},
        .sdm0_n = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_1},
        .sdm1_p = {TLK_GPIO_PORT_F, TLK_GPIO_PIN_4},
        .sdm1_n = {TLK_GPIO_PORT_F, TLK_GPIO_PIN_5},
    };

    struct tlk_audio_amic_pinmux amic_pinmux = {
        .bias = {TLK_GPIO_PORT_D, TLK_GPIO_PIN_1},
    };

    struct tlk_audio_dmic_pinmux dmic_pinmux = {
        .data = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_2},
        .clk1 = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_3},
        .clk2 = {TLK_GPIO_PORT_A, TLK_GPIO_PIN_4},
    };
#elif TLK_IS_ENABLED(CONFIG_TLK_CORE_B92)
    struct tlk_audio_sdm_pinmux sdm_pinmux = {};

    struct tlk_audio_amic_pinmux amic_pinmux = {};

    struct tlk_audio_dmic_pinmux dmic_pinmux = {
        .data = {TLK_GPIO_PORT_D, TLK_GPIO_PIN_5},
        .clk1 = {TLK_GPIO_PORT_D, TLK_GPIO_PIN_4},
        .clk2 = {TLK_GPIO_PORT_D, TLK_GPIO_PIN_3},
    };
#endif

    struct tlk_dma_ll_node tx_buffer0 = {
        .length   = BUFFER_SIZE,
        .src_addr = audio_buffer[0],
    };

    struct tlk_dma_ll_node tx_buffer1 = {
        .next     = &tx_buffer0,
        .length   = BUFFER_SIZE,
        .src_addr = audio_buffer[1],
    };

    tx_buffer0.next = &tx_buffer1;

    struct tlk_dma_ll_node rx_buffer0 = {
        .length   = BUFFER_SIZE,
        .dst_addr = audio_buffer[0],
    };

    struct tlk_dma_ll_node rx_buffer1 = {
        .next     = &rx_buffer0,
        .length   = BUFFER_SIZE,
        .dst_addr = audio_buffer[1],
    };

    rx_buffer0.next = &rx_buffer1;

    struct tlk_audio_input rx_config = {
        .stream        = TLK_AUDIO_INPUT_STREAM0,
        .source        = TLK_AUDIO_INPUT_DMIC,
        .audio_channel = TLK_I2S_CHANNEL_LEFT,
        .sample_rate   = TLK_AUDIO_48K,
        .data_width    = TLK_I2S_DATA_WIDTH_16_BIT,
        .dma_channel   = tlk_dma_chn_request(),
        .fifo          = TLK_I2S_FIFO0,
        .ll_head       = &rx_buffer0,
    };

    struct tlk_audio_output tx_config = {
        .sample_rate   = TLK_AUDIO_48K,
        .audio_channel = TLK_I2S_CHANNEL_LEFT,
        .data_width    = TLK_I2S_DATA_WIDTH_16_BIT,
        .dma_channel   = tlk_dma_chn_request(),
        .fifo          = TLK_I2S_FIFO0,
        .ll_head       = &tx_buffer0,
    };

    tlk_core_interrupt_enable();

    storage = tlk_storage_find_device(storage_addr, STORAGE_SIZE);

    if (!storage)
    {
        TLK_LOG_ERROR(audio_demo, "no storage device found");
    }

    tlk_audio_init();

    tlk_audio_amic_pinmux_configure(&amic_pinmux);
    tlk_audio_dmic_pinmux_configure(rx_config.stream, &dmic_pinmux);
    tlk_audio_sdm_pinmux_configure(&sdm_pinmux);

    tlk_dma_chn_ll_set_irq_mode(rx_config.dma_channel, TLK_DMA_LL_IRQ_INTERRUPT_MODE);
    tlk_dma_chn_add_callback(rx_config.dma_channel, on_rx_buffer_fill);

    tlk_dma_chn_ll_set_irq_mode(tx_config.dma_channel, TLK_DMA_LL_IRQ_INTERRUPT_MODE);
    tlk_dma_chn_add_callback(tx_config.dma_channel, on_tx_buffer_sent);

    tlk_audio_input_configure(&rx_config);
    tlk_audio_output_configure(&tx_config);

    tlk_gpio_configure(PINMUX_LED_1_PORT, PINMUX_LED_1_PIN, TLK_GPIO_OUTPUT);

    struct tlk_benchmark_span span;

    while (1)
    {
        audio_tx_buffer_index = 0;
        audio_rx_buffer_index = 0;

        is_rx_done = 0;
        is_tx_done = 0;

        tlk_benchmark_begin(&span);

        tlk_storage_erase(storage_addr, STORAGE_SIZE);

        tlk_benchmark_end(&span);

        TLK_LOG_INFO(audio_demo, "erase time %d", tlk_benchmark_elapsed(&span));

        tlk_gpio_pin_toggle(PINMUX_LED_1_PORT, PINMUX_LED_1_PIN);
        tlk_benchmark_begin(&span);

        tlk_audio_input_enable();

        while (!is_rx_done)
        {
        }
        tlk_benchmark_end(&span);

        TLK_LOG_INFO(audio_demo, "rx time %d", tlk_benchmark_elapsed(&span));

        tlk_gpio_pin_toggle(PINMUX_LED_1_PORT, PINMUX_LED_1_PIN);

        tlk_benchmark_begin(&span);

        fill_buffer();

        tlk_audio_output_enable();

        while (!is_tx_done)
        {
        }
        tlk_benchmark_end(&span);

        TLK_LOG_INFO(audio_demo, "tx time %d", tlk_benchmark_elapsed(&span));
    }
}

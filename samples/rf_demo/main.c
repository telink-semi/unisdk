#include "main.h"
#include "core/include/tlk_core.h"
#include "system/debug/log/tlk_log.h"
#include "tlk_board_pinout.h"

#if CONFIG_TLK_MEMORY_STACK_SIZE < 3
#error                                                                                             \
    "This sample may require more than 2KB of stack. Increase the stack size in the system configuration."
#endif

TLK_LOG_CREATE(rf_demo, "RF_DEMO");

#define PING_TIMEOUT_MS 1000

uint8_t rx_buffer[300] __attribute__((aligned(4)));
uint8_t tx_buffer[TX_PAYLOAD_SIZE + 6] __attribute__((aligned(4))) = {
    0,  0,  0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
    14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32};

struct tlk_rf_packet* tx_packet = (struct tlk_rf_packet*) tx_buffer;

uint32_t ping;
uint32_t sent_packets;
uint32_t success_packets;

void rf_configure(enum tlk_rf_mode mode, enum tlk_rf_bitrate bitrate)
{
    struct tlk_rf_config rf_config = {
        .bitrate       = bitrate,
        .power         = 4,
        .channel_type  = TLK_RF_CHANNEL_COMMON,
        .handling_mode = TLK_IF_DISABLED_ELSE(
            CONFIG_TLK_RF_DEMO_POLLING_MODE, (TLK_RF_HANDLING_IRQ), (TLK_RF_HANDLING_POLLING)),
        .irq_mask = TLK_RF_IRQ_ALL,
    };

    union tlk_rf_mode_config rf_mode_config;

    switch (mode)
    {
    case TLK_RF_MODE_BLE:
        rf_config.channel                     = 17;
        rf_config.channel_type                = TLK_RF_CHANNEL_BLE;
        rf_mode_config.ble_config.access_code = ACCESS_CODE;
        break;
    case TLK_RF_MODE_ZIGBEE:
        rf_config.channel = 40;
        break;
    case TLK_RF_MODE_PRIVATE:
        rf_config.channel                             = 17;
        rf_mode_config.private_config.access_code     = ACCESS_CODE;
        rf_mode_config.private_config.preamble_length = TLK_IF_ENABLED_ELSE(
            CONFIG_TLK_RF_DEMO_BITRATE_2M,
            (0x43),
            (TLK_IF_ENABLED_ELSE(CONFIG_TLK_RF_DEMO_BITRATE_1M, (0x42), (0x41))));
        break;
    case TLK_RF_MODE_HYBEE:
        rf_config.channel = 40;
        break;
    }

    tlk_rf_configure(mode, &rf_config, &rf_mode_config);
}

int main(void)
{
    tlk_core_interrupt_enable();

    tlk_gpio_configure(UNISDK_BOARD_LED_2_PORT, UNISDK_BOARD_LED_2_PIN, TLK_GPIO_OUTPUT);
    tlk_gpio_configure(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, TLK_GPIO_OUTPUT);

    tlk_gpio_pin_write(UNISDK_BOARD_LED_1_PORT, UNISDK_BOARD_LED_1_PIN, 1);

    rf_configure(CONFIG_TLK_RF_DEMO_MODE, CONFIG_TLK_RF_DEMO_BITRATE);

#if TLK_IS_ENABLED(CONFIG_TLK_RF_DEMO_ROLE_TRANSMITTER)

    transmitter_loop();

#elif TLK_IS_ENABLED(CONFIG_TLK_RF_DEMO_ROLE_RECEIVER)

    receiver_loop();

#endif

    return 0;
}

void on_ping_sent(void)
{
    TLK_LOG_INFO(rf_demo, "");
    TLK_LOG_INFO(rf_demo, "[%u] Ping sent. Waiting for response.", GET_PDU(tx_packet)->payload[0]);
}

void on_ping_received(void)
{
    TLK_LOG_INFO(rf_demo, "[%u] Received!", GET_PDU(tlk_rf_instance.rx_packet)->payload[0]);
}

void on_pong_sent(void)
{
    TLK_LOG_INFO(rf_demo, "[%u] Pong sent.", GET_PDU(tx_packet)->payload[0]);

    tlk_gpio_pin_toggle(UNISDK_BOARD_LED_2_PORT, UNISDK_BOARD_LED_2_PIN);
}

void on_pong_received(void)
{
    uint8_t* tx_payload = GET_PDU(tx_packet)->payload;
    uint8_t* rx_payload = GET_PDU(tlk_rf_instance.rx_packet)->payload;

    sent_packets++;

    if (ping >= TLK_MS_TO_US(PING_TIMEOUT_MS) || tlk_rf_instance.rx_timeout_error)
    {
        TLK_LOG_INFO(rf_demo, "Error: Ping timeout.");
    }
    else
    {
        if (tx_payload[0] == rx_payload[0])
        {
            TLK_LOG_INFO(rf_demo, "[%u] Received!", rx_payload[0]);
            TLK_LOG_INFO(rf_demo, "Time: %-4u us | RSSI: %i", ping, tlk_rf_instance.rx_rssi);

            success_packets++;
        }
        else
        {
            TLK_LOG_INFO(rf_demo,
                         "Error: Diff in packets. Expected [%u], but got [%u].",
                         tx_payload[0],
                         rx_payload[0]);
        }
    }

    TLK_LOG_INFO(rf_demo, "Count: %-6u | Success count: %u", sent_packets, success_packets);

    tlk_gpio_pin_toggle(UNISDK_BOARD_LED_2_PORT, UNISDK_BOARD_LED_2_PIN);
}

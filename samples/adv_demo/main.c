#include "core/include/tlk_gpio.h"
#include "core/include/tlk_rf.h"
#include "core/include/tlk_sys.h"
#include "tlk_core.h"
#include "common/include/tlk_utils.h"
#include "tlk_board_pinout.h"

#define TX_DELAY_MS     100

#define TX_BUFFER_SIZE (sizeof(tx_buffer) / sizeof(tx_buffer[0]))
#define TX_PDU_SIZE (TX_BUFFER_SIZE - sizeof(struct tlk_rf_packet))
#define TX_PAYLOAD_SIZE (TX_PDU_SIZE - sizeof(struct tlk_rf_ble_pdu))

uint8_t tx_buffer[] __attribute__((aligned(4))) = {
    /* DMA length */
    0x00, 0x00, 0x00, 0x00,
    /* PDU header */
    0x02, 0x00,
    /* Advertiser address */
    0x23, 0xe3, 0x03, 0xb4, 0xcf, 0x3c,
    /* Flags */
    0x02, 0x01, 0x05,
    /* Complete local name */
    0x09, 0x09, 'a', 'd', 'v', '_', 'd', 'e', 'm', 'o',
    /* Appearance */
    0x03, 0x19, 0x80, 0x01,
};

struct tlk_rf_packet *tx_packet = (struct tlk_rf_packet *)(tx_buffer);

void rf_configure(void)
{
    struct tlk_rf_config rf_config = {
        .bitrate = TLK_RF_BITRATE_1M,
        .power = 4,
        .channel_type = TLK_RF_CHANNEL_BLE,
        .channel = 37,
    };

    union tlk_rf_mode_config rf_mode_config = {
        .ble_config = { .access_code = 0xd6be898e }
    };

    tlk_rf_configure(TLK_RF_MODE_BLE, &rf_config, &rf_mode_config);
}

int main(void) 
{
    tlk_core_interrupt_enable();

    tlk_gpio_configure(UNISDK_BOARD_LED_2_PORT, UNISDK_BOARD_LED_2_PIN, TLK_GPIO_OUTPUT);

    rf_configure();

    tlk_rf_set_tx_fifo(tx_buffer, 0, TX_BUFFER_SIZE);

    ((struct tlk_rf_ble_pdu *)(tx_packet->pdu))->payload_size = TX_PAYLOAD_SIZE;

    while (1) {
        tlk_rf_prepare_tx_packet(tx_packet, TX_PDU_SIZE);

        tlk_gpio_pin_toggle(UNISDK_BOARD_LED_2_PORT, UNISDK_BOARD_LED_2_PIN);

        tlk_rf_start_stx(tlk_rf_get_tick());

        while (!tlk_rf_instance.tx_end) { }

        tlk_sys_delay(TLK_M2USEC(TX_DELAY_MS));
    }
}
#include "core/cpp_wrappers/tlk_gpio.hpp"
#include "core/cpp_wrappers/tlk_rf.hpp"

#ifdef __cplusplus
extern "C"
{
#endif
#include "common/include/tlk_utils.h"

#include "core/include/tlk_core.h"
#include "core/include/tlk_sys.h"
#include "tlk_board_pinout.h"
#ifdef __cplusplus
}
#endif

#define TX_DELAY_MS 100

#define TX_BUFFER_SIZE  (sizeof(tx_buffer) / sizeof(tx_buffer[0]))
#define TX_PDU_SIZE     (TX_BUFFER_SIZE - sizeof(struct tlk_rf_packet))
#define TX_PAYLOAD_SIZE (TX_PDU_SIZE - sizeof(struct tlk_rf_ble_pdu))

uint8_t tx_buffer[] __attribute__((aligned(4))) = {
    /* DMA length */
    0x00,
    0x00,
    0x00,
    0x00,
    /* PDU header */
    0x02,
    0x00,
    /* Advertiser address */
    0x23,
    0xe3,
    0x03,
    0xb4,
    0xcf,
    0x3c,
    /* Flags */
    0x02,
    0x01,
    0x05,
    /* Complete local name */
    0x09,
    0x09,
    'a',
    'd',
    'v',
    '_',
    'd',
    'e',
    'm',
    'o',
    /* Appearance */
    0x03,
    0x19,
    0x80,
    0x01,
};

int main(void)
{
    tlk_core_interrupt_enable();

    GPIODriver led(UNISDK_BOARD_LED_2_PORT);
    led.configure(UNISDK_BOARD_LED_2_PIN, TLK_GPIO_OUTPUT);

    tlk_rf_config rf_config = {};
    rf_config.bitrate       = TLK_RF_BITRATE_1M;
    rf_config.power         = 4;
    rf_config.channel_type  = TLK_RF_CHANNEL_BLE;
    rf_config.channel       = 37;
    rf_config.irq_mask      = TLK_RF_IRQ_ALL;

    tlk_rf_mode_config rf_mode_config = {.ble_config = {.access_code = 0xd6be898e}};

    RfDriver::configure(TLK_RF_MODE_BLE, &rf_config, &rf_mode_config);

    RfDriver::setTxFifo(tx_buffer, 0, TX_BUFFER_SIZE);

    tlk_rf_packet* tx_packet = reinterpret_cast<tlk_rf_packet*>(tx_buffer);
    reinterpret_cast<tlk_rf_ble_pdu*>(tx_packet->pdu)->payload_size = TX_PAYLOAD_SIZE;

    while (1)
    {
        RfDriver::prepareTxPacket(tx_packet, TX_PDU_SIZE);

        led.toggle(UNISDK_BOARD_LED_2_PIN);

        RfDriver::startStx(RfDriver::getTick());

        while (!RfDriver::instance().tx_end)
        {
            RfDriver::irqProcess();
        }

        tlk_sys_delay(TLK_MS_TO_US(TX_DELAY_MS));
    }
}

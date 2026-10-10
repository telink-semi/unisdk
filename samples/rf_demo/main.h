#ifndef TLK_RF_DEMO_H_
#define TLK_RF_DEMO_H_

#include "api/include/tlk_time.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_rf.h"
#include "core/include/tlk_sys.h"
#include "properties/tlk_rf.h"
#include <string.h>

#if TLK_IS_ENABLED(CONFIG_TLK_RF_DEMO_PM)
#include "api/include/tlk_sleep.h"
#endif

#define TX_FIFO_DEPTH 0
#define TX_FIFO_SIZE  32
#define RX_FIFO_COUNT 2
#define RX_FIFO_SIZE  64

#define PING_TIMEOUT_MS 1000

#define ACCESS_CODE 0x29417671

#define TX_PAYLOAD_SIZE 32
#define TX_PDU_SIZE     (TX_PAYLOAD_SIZE + sizeof(PACKET_PDU_TYPE))
#define TX_PACKET_SIZE  (TX_PDU_SIZE + sizeof(struct tlk_rf_packet))

/**
 * Usually, you know which protocol is used in the project, and casting of pdu is quite simple.
 * But in this demo, it is configurable, so we need to switch between these types.
 */
#define PACKET_PDU_TYPE                                                                            \
    TLK_IF_ENABLED_ELSE(                                                                           \
        CONFIG_TLK_RF_DEMO_MODE_BLE,                                                               \
        (struct tlk_rf_ble_pdu),                                                                   \
        (TLK_IF_ENABLED_ELSE(                                                                      \
            CONFIG_TLK_RF_DEMO_MODE_ZIGBEE,                                                        \
            (struct tlk_rf_zigbee_pdu),                                                            \
            (TLK_IF_ENABLED_ELSE(                                                                  \
                CONFIG_TLK_RF_DEMO_MODE_HYBEE,                                                     \
                (struct tlk_rf_hybee_pdu),                                                         \
                (TLK_IF_ENABLED(CONFIG_TLK_RF_DEMO_MODE_PRIVATE,                                   \
                                (TLK_IF_ENABLED_ELSE(CONFIG_TLK_RF_DEMO_SB_FORMAT,                 \
                                                     (struct rf_demo_private_sb_pdu),              \
                                                     (struct tlk_rf_private_esb_pdu))))))))))

#define GET_PDU(packet) ((PACKET_PDU_TYPE*) ((packet)->pdu))

#define PACKET_PAYLOAD_SIZE                                                                        \
    (TX_PAYLOAD_SIZE TLK_IF_ENABLED(CONFIG_TLK_RF_DEMO_MODE_ZIGBEE, (+TLK_RF_ZIGBEE_CRC_SIZE))     \
         TLK_IF_ENABLED(CONFIG_TLK_RF_DEMO_MODE_HYBEE, (+TLK_RF_HYBEE_CRC_SIZE)))

/**
 * Private SB format has no predefined PDU struct, since PDU contains only payload,
 * and we cannot define the struct which contains only uint8_t payload[],
 * because then the struct will have 0 size, which is not allowed by the C standard.
 * However, this demo expects to have a PDU type with a payload field,
 * so to make it compatible, we define a custom dummy PDU type for demo.
 * The customer may define their own struct with meaningful fields for the application.
 */
struct rf_demo_private_sb_pdu
{
    uint8_t command;   // example of some useful data
    uint8_t payload[]; // the field that is used in the demo
} __attribute__((packed));

extern struct tlk_rf_packet* tx_packet;

extern uint8_t rx_buffer[300] __attribute__((aligned(4)));
extern uint8_t tx_buffer[TX_PAYLOAD_SIZE + 6] __attribute__((aligned(4)));

extern uint32_t ping;
extern uint32_t sent_packets;
extern uint32_t success_packets;

void transmitter_loop(void);
void receiver_loop(void);

void rf_configure(enum tlk_rf_mode mode, enum tlk_rf_bitrate bitrate);

void on_ping_sent(void);
void on_ping_received(void);
void on_pong_sent(void);
void on_pong_received(void);

#endif

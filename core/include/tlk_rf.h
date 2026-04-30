#ifndef TLK_INCLUDE_DRIVERS_RF_H_
#define TLK_INCLUDE_DRIVERS_RF_H_

#include "properties/tlk_rf.h"

#define TLK_RF_US_TO_TICK(us) (us * UNISDK_RF_TICK_1US)

enum tlk_rf_mode
{
    TLK_RF_MODE_BLE,
    TLK_RF_MODE_ZIGBEE,
    TLK_RF_MODE_PRIVATE,
    TLK_RF_MODE_HYBEE,
};

enum tlk_rf_bitrate 
{
    TLK_RF_BITRATE_125K,
    TLK_RF_BITRATE_250K,
    TLK_RF_BITRATE_500K,
    TLK_RF_BITRATE_1M,
    TLK_RF_BITRATE_2M,
};

enum tlk_rf_status
{
    TLK_RF_STATUS_OK,
    TLK_RF_STATUS_INVALID_BITRATE,
    TLK_RF_STATUS_INVALID_STATE,
    TLK_RF_STATUS_INVALID_POWER,
};



#define TLK_RF_BLE_CRC_SIZE 3
#define TLK_RF_ZIGBEE_CRC_SIZE 2
#define TLK_RF_PRIVATE_CRC_SIZE 2
#define TLK_RF_HYBEE_CRC_SIZE 2

#define TLK_RF_GET_RX_PACKET_TAIL(packet) \
     (struct tlk_rf_rx_packet_tail *)((uint8_t *)(packet) + sizeof(struct tlk_rf_packet) + (packet)->dma_size - sizeof(struct tlk_rf_rx_packet_tail))

struct tlk_rf_packet {
    uint32_t dma_size;
    uint8_t pdu[];
} __attribute__((packed));

struct tlk_rf_ble_pdu {
    union {
        uint16_t header;
        struct {
            uint8_t flags;
            uint8_t payload_size;
        };
    };
    uint8_t payload[];
} __attribute__((packed));

struct tlk_rf_zigbee_pdu {
    uint8_t payload_size;
    uint8_t payload[];
} __attribute__((packed));

struct tlk_rf_private_pdu {
    uint8_t payload_size;
    uint8_t payload[];
} __attribute__((packed));

struct tlk_rf_hybee_pdu {
    uint8_t payload_size;
    uint8_t payload[];
} __attribute__((packed));

struct tlk_rf_rx_packet_tail {
    uint32_t timestamp;
    int16_t frequency_offset;
    uint8_t rssi;
    union {
        uint8_t rx_status;
        struct {
            uint8_t crc_error : 1;
            uint8_t sfd_error : 1;
            uint8_t ll_error : 1;
            uint8_t power_error : 1;
            uint8_t long_range_125k_indicator : 1;
            uint8_t reserved : 2;
            uint8_t nordic_noack_indicator : 1;
        };
    };
} __attribute__((packed));



typedef void (*tlk_rf_tx_end_handler)(void);
typedef void (*tlk_rf_rx_end_handler)(void);
typedef void (*tlk_rf_rx_timeout_handler)(void);
typedef void (*tlk_rf_cmd_end_handler)(void);

// todo: organize the data in this struct
struct tlk_rf_module
{
    volatile union {
        uint8_t tx_state;
        struct {
            uint8_t tx_end : 1;
            uint8_t tx_ok : 1;
            uint8_t _tx_reserved : 6;
        };
    };
    volatile union {
        uint8_t rx_state;
        struct {
            uint8_t rx_end : 1;
            uint8_t rx_ok : 1;
            uint8_t rx_timeout_error : 1;
            uint8_t rx_crc_error : 1;
            uint8_t _rx_reserved : 4;
        };
    };

    volatile uint8_t rx_len;
    volatile int8_t rx_rssi;

    enum tlk_rf_mode mode;

    union {
        uint8_t *volatile rx_packet_raw; // The pointer to the received packet in the FIFO
        struct tlk_rf_packet *volatile rx_packet;
    };

    tlk_rf_tx_end_handler tx_end_handler;
    tlk_rf_rx_end_handler rx_end_handler;
    tlk_rf_rx_timeout_handler rx_timeout_handler;
    tlk_rf_cmd_end_handler cmd_end_handler;

    struct {
        uint8_t initialized : 1;
        uint8_t _reserved : 7;
    };
};

struct tlk_rf_fifo_config {
    struct {
        uint8_t *buffer;
        uint8_t depth;
        uint16_t size;
    } tx;
    struct {
        uint8_t *buffer;
        uint8_t count;
        uint16_t size;
    } rx;
};

enum tlk_rf_channel_type {
    TLK_RF_CHANNEL_COMMON,
    TLK_RF_CHANNEL_BLE,
};

struct tlk_rf_config {
    enum tlk_rf_bitrate bitrate;
    int8_t power;
    int8_t channel;
    enum tlk_rf_channel_type channel_type;
};

struct tlk_rf_ble_config {
    uint32_t access_code;
};

struct tlk_rf_private_config {
    uint32_t access_code;
    uint8_t preamble_length;
};

union tlk_rf_mode_config {
    struct tlk_rf_ble_config ble_config;
    struct tlk_rf_private_config private_config;
};

//TODO: add the configs for zigbee and hybee



/**
 * @brief Used to prepare the packet for tx.
 * 
 * @param packet - tx packet address.
 * @param pdu_size - tx pdu size.
 *
 * @return None.
 */
void tlk_rf_prepare_tx_packet(struct tlk_rf_packet *packet, uint8_t pdu_size);

/**
 * @brief Used to calculate the payload size from the received packet size.
 * 
 * @param mode - RF mode for the received packet.
 * @param packet_size - received packet size.
 *
 * @return Payload size. Consider that it includes crc in Zigbee and Hybee modes.
 */
uint8_t tlk_rf_rx_packet_size_to_payload_size(enum tlk_rf_mode mode, uint8_t packet_size);



/**
 * @brief Sets the bitrate for the current RF mode.
 * 
 * @param bitrate - desired bitrate.
 *
 * @return TLK_RF_STATUS_INVALID_BITRATE in case of unsupported bitrate, otherwise TLK_RF_STATUS_OK.
 */
enum tlk_rf_status tlk_rf_set_bitrate(enum tlk_rf_bitrate bitrate);

/**
 * @brief Sets the RF power level.
 * 
 * @param power - desired power level in dBm.
 *
 * @return TLK_RF_STATUS_INVALID_POWER in case of an invalid power value, otherwise TLK_RF_STATUS_OK.
 */
enum tlk_rf_status tlk_rf_set_power(int8_t power);

/**
 * @brief Sets the frequency-based channel. Actual channel is 2400 + channel.
 * 
 * @param channel - desired channel to set.
 *
 * @return None.
 */
void tlk_rf_set_channel(int8_t channel);

/**
 * @brief Sets the BLE channel according to the protocol specification.
 * 
 * @param channel - BLE channel.
 *
 * @return None.
 */
void tlk_rf_set_ble_channel(int8_t ble_channel);

/**
 * @brief Sets the access code.
 * 
 * @param access_code - access code.
 *
 * @return None.
 */
void tlk_rf_set_access_code(uint32_t access_code);

/**
 * @brief Sets the preamble length. Used in the private mode.
 * 
 * @param preamble_length - preamle length.
 *
 * @return None.
 */
void tlk_rf_set_preamble_length(uint8_t preamble_length);



/**
 * @brief Used for the full RF driver configuration.
 * 
 * @param mode - desired RF mode.
 * @param rf_config - general RF parameters.
 * @param rf_mode_config - mode-specific RF parameters.
 *
 * @return Configuration status.
 */
enum tlk_rf_status tlk_rf_configure(enum tlk_rf_mode mode, struct tlk_rf_config *rf_config, union tlk_rf_mode_config *rf_mode_config);



/**
 * @brief Sets the tx end handler, which is called on tx end irq.
 *
 * @param handler - tx end handler.
 * 
 * @return None.
 */
void tlk_rf_set_tx_end_handler(tlk_rf_tx_end_handler handler);

/**
 * @brief Sets the rx end handler, which is called on rx end irq.
 *
 * @note This handler is called regardless of the validity of the rx packet.
 * @note The rx timeout event has a separate handler.
 *
 * @param handler - rx end handler.
 * 
 * @return None.
 */
void tlk_rf_set_rx_end_handler(tlk_rf_rx_end_handler handler);

/**
 * @brief Sets the rx timeout handler, which is called on rx timeout irq.
 *
 * @param handler - rx timeout handler.
 * 
 * @return None.
 */
void tlk_rf_set_rx_timeout_handler(tlk_rf_rx_timeout_handler handler);

/**
 * @brief Sets the cmd end handler, which is called on cmd end irq.
 *
 * This handler is called on the end of any tx/rx operation, 
 * except for those caused by the set_xx_state functions. 
 *
 * @param handler - cmd end handler.
 * 
 * @return None.
 */
void tlk_rf_set_cmd_end_handler(tlk_rf_cmd_end_handler handler);

/**
 * @brief Sets the tx buffer. Need to be called before any tx operation.
 *
 * The buffer size should not be less than FIFO count times FIFO size.
 * 
 * @param buffer - 4-byte aligned address for the tx buffer.
 * @param depth - Depth of the FIFO. The FIFO count equals 2^depth.
 * @param size - The size of each FIFO in bytes.
 *
 * @return None.
 */
void tlk_rf_set_tx_fifo(uint8_t* buffer, uint8_t depth, uint16_t size);

/**
 * @brief Sets the rx buffer. Need to be called before any rx operation.
 *
 * The buffer size should not be less than FIFO count times FIFO size.
 * 
 * @param buffer - 4-byte aligned address for the rx buffer.
 * @param count - The count of the FIFOs.
 * @param size - The size of each FIFO in bytes.
 *
 * @return None.
 */
void tlk_rf_set_rx_fifo(uint8_t* buffer, uint8_t count, uint16_t size);

/**
 * @brief Sets the tx settle time.
 * 
 * @param time_us - tx settle time in us.
 *
 * @return None.
 */
void tlk_rf_set_tx_settle_time(uint16_t time_us);

/**
 * @brief Sets the rx settle time.
 * 
 * @param time_us - rx settle time in us.
 *
 * @return None.
 */
void tlk_rf_set_rx_settle_time(uint16_t time_us);



/**
 * @brief Enables manual transition to tx state.
 * 
 * @return None.
 */
void tlk_rf_set_tx_state(void);

/**
 * @brief Enables manual transition to rx state.
 * 
 * @return None.
 */
void tlk_rf_set_rx_state(void);

/**
 * @brief Resets RF tx/rx state.
 * 
 * @return None.
 */
void tlk_rf_set_idle_state(void);



/**
 * @brief Used to get the rf baseband timer tick.
 * 
 * @return Current rf tick.
 */
uint32_t tlk_rf_get_tick(void);

/**
 * @brief This function serves to trigger stx.
 *
 * @param tick - Trigger tx when the current tick is greater than or equal to the provided.
 *
 * @return None.
 */
void tlk_rf_start_stx(uint32_t tick);

/**
 * @brief This function serves to trigger srx.
 *
 * @param tick - Trigger rx when the current tick is greater than or equal to the provided.
 * @param timeout - The timeout for the rx operation in us.
 *
 * @return None.
 */
void tlk_rf_start_srx(uint32_t tick, uint32_t timeout);

/**
 * @brief This function serves to trigger stx2rx.
 *
 * @param tick - Trigger tx when the current tick is greater than or equal to the provided.
 * @param timeout - The timeout for the rx operation in us.
 * @param delay - The gap between the tx and rx in us.
 *
 * @return None.
 */
void tlk_rf_start_stx2rx(uint32_t tick, uint32_t timeout, uint16_t delay);

/**
 * @brief This function serves to trigger srx2tx.
 *
 * @param tick - Trigger rx when the current tick is greater than or equal to the provided.
 * @param timeout - The timeout for the rx operation in us.
 * @param delay - The gap between the rx and tx in us.
 *
 * @return None.
 */
void tlk_rf_start_srx2tx(uint32_t tick, uint32_t timeout, uint16_t delay);

/**
 * @brief This function serves to trigger btx. 
 *
 * In this mode, RF module stays in tx status until a packet is sent 
 * and then goes to rx until ACK packet received or a timeout expires.
 *
 * @param tick - Trigger tx when the current tick is greater than or equal to the provided.
 * @param timeout - The timeout for the rx operation in us.
 * @param delay - The gap between the tx and rx in us.
 *
 * @return None.
 */
void tlk_rf_start_btx(uint32_t tick, uint32_t timeout, uint16_t delay);

/**
 * @brief This function serves to trigger brx. 
 *
 * In this mode, RF module stays in rx status until a packet is received 
 * or timeout expires. Once received a packet, it goes to tx for sending ACK packet.
 *
 * @param tick - Trigger rx when the current tick is greater than or equal to the provided.
 * @param timeout - The timeout for the rx operation in us.
 * @param delay - The gap between the rx and tx in us.
 *
 * @return None.
 */
void tlk_rf_start_brx(uint32_t tick, uint32_t timeout, uint16_t delay);

/**
 * @brief This function serves to trigger ptx.
 *
 * @param tick - Trigger tx when the current tick is greater than or equal to the provided.
 * @param timeout - The timeout for the ACK rx operation in us.
 * @param delay - The gap between the tx and ACK rx in us.
 *
 * @return None.
 */
void tlk_rf_start_ptx(uint32_t tick, uint32_t timeout, uint16_t delay);

/**
 * @brief This function serves to trigger prx.
 *
 * @param tick - Trigger rx when the current tick is greater than or equal to the provided.
 * @param timeout - The timeout for the rx operation in us.
 * @param delay - The gap between the rx and ACK tx in us.
 * @return None.
 */
void tlk_rf_start_prx(uint32_t tick, uint32_t timeout, uint16_t delay);

/**
 * @brief Used to stop any operation of the auto mode.
 * 
 * @return None.
 *
 * @note This function should not be called during active rx.
 */
void tlk_rf_stop_tx_rx_auto(void);



extern struct tlk_rf_module tlk_rf_instance;

#endif

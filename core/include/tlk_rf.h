#ifndef TLK_INCLUDE_DRIVERS_RF_H_
#define TLK_INCLUDE_DRIVERS_RF_H_

#include "properties/tlk_rf.h"

#define TLK_RF_US_TO_TICK(us) ((us) * UNISDK_RF_TICK_1US)

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
    TLK_RF_STATUS_UNSUPPORTED,
    TLK_RF_STATUS_INVALID_BITRATE,
    TLK_RF_STATUS_INVALID_STATE,
    TLK_RF_STATUS_INVALID_POWER,
};

enum tlk_rf_handling_mode
{
    TLK_RF_HANDLING_IRQ,
    TLK_RF_HANDLING_POLLING,
};

enum tlk_rf_irq
{
    TLK_RF_IRQ_RX            = TLK_BIT(0),  /* packet received */
    TLK_RF_IRQ_TX            = TLK_BIT(1),  /* packet sent */
    TLK_RF_IRQ_RX_TIMEOUT    = TLK_BIT(2),  /* RX timeout in auto mode */
    TLK_RF_IRQ_RX_CRC_2      = TLK_BIT(4),  /* CRC error twice in a row during BTX/BRX/PTX/PRX RX */
    TLK_RF_IRQ_CMD           = TLK_BIT(5),  /* auto mode operation completed */
    TLK_RF_IRQ_TX_RETRYCNT   = TLK_BIT(7),  /* PTX retry count exceeded */
    TLK_RF_IRQ_TX_DS         = TLK_BIT(8),  /* PTX/PRX sent payload with non-zero length */
    TLK_RF_IRQ_RX_DR         = TLK_BIT(9),  /* PTX/PRX/SRX received packet with non-zero length */
    TLK_RF_IRQ_FIRST_TIMEOUT = TLK_BIT(10), /* BRX/PRX/SRX/SRT first packet RX timeout */
    TLK_RF_IRQ_INVALID_PID   = TLK_BIT(11), /* PTX/PRX received invalid PID */
    TLK_RF_IRQ_ALL           = 0xfb7,
};

#define __TLK_RF_DECLARE_FS_TX_TIME(time) TLK_RF_FS_TX_TIME_##time##US = time,
#define _TLK_RF_DECLARE_FS_TX_TIME(time)  __TLK_RF_DECLARE_FS_TX_TIME(time)
#define TLK_RF_DECLARE_FS_TX_TIME(n)      _TLK_RF_DECLARE_FS_TX_TIME(UNISDK_RF_FS_TX_TIME_##n)

#define __TLK_RF_DECLARE_FS_RX_TIME(time) TLK_RF_FS_RX_TIME_##time##US = time,
#define _TLK_RF_DECLARE_FS_RX_TIME(time)  __TLK_RF_DECLARE_FS_RX_TIME(time)
#define TLK_RF_DECLARE_FS_RX_TIME(n)      _TLK_RF_DECLARE_FS_RX_TIME(UNISDK_RF_FS_RX_TIME_##n)

enum tlk_rf_fs_tx_time
{
    TLK_RF_FS_TX_NONE     = UNISDK_RF_FS_TX_DEFAULT,
    TLK_RF_FS_TX_TIME_MIN = UNISDK_RF_FS_TX_TIME_0,
    TLK_FOR_CALL(UNISDK_RF_FS_TX_TIME_COUNT, TLK_RF_DECLARE_FS_TX_TIME)
};

enum tlk_rf_fs_rx_time
{
    TLK_RF_FS_RX_NONE     = UNISDK_RF_FS_RX_DEFAULT,
    TLK_RF_FS_RX_TIME_MIN = UNISDK_RF_FS_RX_TIME_0,
    TLK_FOR_CALL(UNISDK_RF_FS_RX_TIME_COUNT, TLK_RF_DECLARE_FS_RX_TIME)
};

typedef void (*tlk_rf_irq_handler_t)(enum tlk_rf_irq);

#define TLK_RF_BLE_CRC_SIZE     3
#define TLK_RF_ZIGBEE_CRC_SIZE  2
#define TLK_RF_PRIVATE_CRC_SIZE 2
#define TLK_RF_HYBEE_CRC_SIZE   2

#define TLK_RF_GET_RX_PACKET_TAIL(packet)                                                          \
    (struct tlk_rf_rx_packet_tail*) ((uint8_t*) (packet) + sizeof(struct tlk_rf_packet) +          \
                                     (packet)->dma_size - sizeof(struct tlk_rf_rx_packet_tail))

struct tlk_rf_packet
{
    uint32_t dma_size;
    uint8_t  pdu[];
} __attribute__((packed));

struct tlk_rf_ble_pdu
{
    union
    {
        uint16_t header;
        struct
        {
            uint8_t flags;
            uint8_t payload_size;
        };
    };
    uint8_t payload[];
} __attribute__((packed));

struct tlk_rf_zigbee_pdu
{
    uint8_t payload_size;
    uint8_t payload[];
} __attribute__((packed));

struct tlk_rf_private_pdu
{
    uint8_t payload_size;
    uint8_t payload[];
} __attribute__((packed));

struct tlk_rf_hybee_pdu
{
    uint8_t payload_size;
    uint8_t payload[];
} __attribute__((packed));

struct tlk_rf_rx_packet_tail
{
    uint32_t timestamp;
    int16_t  frequency_offset;
    uint8_t  rssi;
    union
    {
        uint8_t rx_status;
        struct
        {
            uint8_t crc_error                 : 1;
            uint8_t sfd_error                 : 1;
            uint8_t ll_error                  : 1;
            uint8_t power_error               : 1;
            uint8_t long_range_125k_indicator : 1;
            uint8_t reserved                  : 2;
            uint8_t nordic_noack_indicator    : 1;
        };
    };
} __attribute__((packed));

// todo: organize the data in this struct
struct tlk_rf_module
{
    volatile union
    {
        uint8_t tx_state;
        struct
        {
            uint8_t tx_end       : 1;
            uint8_t tx_ok        : 1;
            uint8_t _tx_reserved : 6;
        };
    };
    volatile union
    {
        uint8_t rx_state;
        struct
        {
            uint8_t rx_end           : 1;
            uint8_t rx_ok            : 1;
            uint8_t rx_timeout_error : 1;
            uint8_t rx_crc_error     : 1;
            uint8_t _rx_reserved     : 4;
        };
    };

    volatile uint8_t rx_len;
    volatile int8_t  rx_rssi;

    enum tlk_rf_mode mode;

    union
    {
        uint8_t* volatile rx_packet_raw; // The pointer to the received packet in the FIFO
        struct tlk_rf_packet* volatile rx_packet;
    };

    tlk_rf_irq_handler_t irq_handler;
    struct
    {
        enum tlk_rf_fs_tx_time tx_time;
        enum tlk_rf_fs_rx_time rx_time;
    } fs;

    struct
    {
        uint8_t initialized   : 1;
        uint8_t fs_tx_enabled : 1;
        uint8_t fs_rx_enabled : 1;
        uint8_t _reserved     : 7;
    };
};

struct tlk_rf_fifo_config
{
    struct
    {
        uint8_t* buffer;
        uint8_t  depth;
        uint16_t size;
    } tx;
    struct
    {
        uint8_t* buffer;
        uint8_t  count;
        uint16_t size;
    } rx;
};

enum tlk_rf_channel_type
{
    TLK_RF_CHANNEL_COMMON,
    TLK_RF_CHANNEL_BLE,
};

struct tlk_rf_config
{
    enum tlk_rf_bitrate       bitrate;
    int8_t                    power;
    int8_t                    channel;
    enum tlk_rf_channel_type  channel_type;
    enum tlk_rf_handling_mode handling_mode;
    enum tlk_rf_irq           irq_mask;
};

struct tlk_rf_ble_config
{
    uint32_t access_code;
};

struct tlk_rf_private_config
{
    uint32_t access_code;
    uint8_t  preamble_length;
};

union tlk_rf_mode_config
{
    struct tlk_rf_ble_config     ble_config;
    struct tlk_rf_private_config private_config;
};

// TODO: add the configs for zigbee and hybee

extern struct tlk_rf_module tlk_rf_instance;

/**
 * @brief Used to prepare the packet for tx.
 *
 * @param packet - tx packet address.
 * @param pdu_size - tx pdu size.
 *
 * @return None.
 */
void tlk_rf_prepare_tx_packet(struct tlk_rf_packet* packet, uint8_t pdu_size);

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
 * @return TLK_RF_STATUS_INVALID_POWER in case of an invalid power value, otherwise
 * TLK_RF_STATUS_OK.
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
enum tlk_rf_status tlk_rf_configure(enum tlk_rf_mode mode, struct tlk_rf_config* rf_config,
                                    union tlk_rf_mode_config* rf_mode_config);

/**
 * @brief Reset baseband.
 *
 * @return None.
 */
void tlk_rf_reset_baseband(void);

/**
 * @brief Reset register values.
 *
 * @return None.
 */
void tlk_rf_reset_registers(void);

/**
 * @brief Sets the irq handler.
 *
 * @param handler - irq handler.
 *
 * @return None.
 */
void tlk_rf_set_irq_handler(tlk_rf_irq_handler_t handler);

/**
 * @brief Enables the tracking of the corresponding irqs.
 *
 * @param mask - irqs to enable.
 *
 * @return TLK_RF_STATUS_UNSUPPORTED if the given irq is unsupported, otherwise TLK_RF_STATUS_OK.
 */
enum tlk_rf_status tlk_rf_irq_enable(enum tlk_rf_irq mask);

/**
 * @brief Disables the tracking of the corresponding irqs.
 *
 * @param mask - irqs to disable.
 *
 * @return TLK_RF_STATUS_UNSUPPORTED if the given irq is unsupported, otherwise TLK_RF_STATUS_OK.
 */
enum tlk_rf_status tlk_rf_irq_disable(enum tlk_rf_irq mask);

/**
 * @brief Gets the mask of the currently enabled irqs.
 *
 * @return Currently enabled irqs.
 */
enum tlk_rf_irq tlk_rf_irq_get_mask(void);

/**
 * @brief Gets the current irq status.
 *
 * @return Currently fired irqs.
 */
enum tlk_rf_irq tlk_rf_irq_get_status(void);

/**
 * @brief Handles passed irqs and clears status.
 *
 * @param mask - irqs to handle.
 *
 * @return None.
 */
void tlk_rf_irq_handle(enum tlk_rf_irq mask);

/**
 * @brief Retrieves fired irqs and handles them. Used for TLK_RF_HANDLING_POLLING mode.
 *
 * @return None.
 */
void tlk_rf_irq_process(void);

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
 * @brief Starts the packet transmission. Should be called only after tlk_rf_set_tx_state and delay.
 *
 * @return None.
 */
void tlk_rf_start_tx_state(void);

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
 * @note This function must be called once the tx/rx state setting action is done.
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
 *
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

/**
 * @brief Configure the fast settle feature.
 *
 * @param tx_time - TX fast settle time.
 * @param rx_time - RX fast settle time.
 *
 * @return TLK_RF_STATUS_OK on success.
 */
enum tlk_rf_status tlk_rf_fs_configure(enum tlk_rf_fs_tx_time tx_time,
                                       enum tlk_rf_fs_rx_time rx_time);

/**
 * @brief Calibrate the fast settle internal parameters for the given time.
 *
 * @param tx_time - TX fast settle time.
 * @param rx_time - RX fast settle time.
 *
 * @return TLK_RF_STATUS_OK on success.
 */
enum tlk_rf_status tlk_rf_fs_calibrate(enum tlk_rf_fs_tx_time tx_time,
                                       enum tlk_rf_fs_rx_time rx_time);

/**
 * @brief Enable the TX fast settle.
 *
 * @return None.
 */
void tlk_rf_fs_tx_enable(void);

/**
 * @brief Enable the RX fast settle.
 *
 * @return None.
 */
void tlk_rf_fs_rx_enable(void);

/**
 * @brief Disable the TX fast settle.
 *
 * @return None.
 */
void tlk_rf_fs_tx_disable(void);

/**
 * @brief Disable the RX fast settle.
 *
 * @return None.
 */
void tlk_rf_fs_rx_disable(void);

#endif

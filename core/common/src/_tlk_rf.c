#include "common/include/tlk_init.h"
#include "core/include/tlk_rf.h"

/* Undo the public read-only alias: this file is one of the RF driver's owners. */
#undef tlk_rf_instance
extern struct tlk_rf_module tlk_rf_instance;

struct tlk_rf_config             tlk_rf_config;
union tlk_rf_mode_config         tlk_rf_mode_config;
extern struct tlk_rf_fifo_config tlk_rf_fifo_config;

void tlk_rf_tx_prepare_packet(struct tlk_rf_packet* packet, uint8_t pdu_size)
{
    packet->dma_size = ((pdu_size + 3) / 4) | ((pdu_size % 4) << 22);
}

uint8_t tlk_rf_rx_packet_size_to_payload_size(enum tlk_rf_mode mode, uint8_t packet_size)
{
    uint8_t delta_size = sizeof(struct tlk_rf_packet);

    switch (mode)
    {
    case TLK_RF_MODE_BLE:
        delta_size += sizeof(struct tlk_rf_ble_pdu);
        delta_size += TLK_RF_BLE_CRC_SIZE;
        break;
    case TLK_RF_MODE_ZIGBEE:
        delta_size += sizeof(struct tlk_rf_zigbee_pdu);
        break;
    case TLK_RF_MODE_PRIVATE:
        if (tlk_rf_mode_config.private_config.format == TLK_RF_PRIVATE_ESB)
        {
            delta_size += sizeof(struct tlk_rf_private_esb_pdu);
        }
        delta_size += TLK_RF_PRIVATE_CRC_SIZE;
        break;
    case TLK_RF_MODE_HYBEE:
        delta_size += sizeof(struct tlk_rf_hybee_pdu);
        break;
    }

    delta_size += sizeof(struct tlk_rf_rx_packet_tail);

    return packet_size - delta_size;
}

void tlk_rf_irq_set_handler(tlk_rf_irq_handler_t handler)
{
    tlk_rf_instance.irq_handler = handler;
}

#if TLK_IS_ENABLED(CONFIG_TLK_RF_PM_DEVICE)

extern void tlk_rf_fs_set_calibration(enum tlk_rf_fs_tx_time tx_time,
                                      enum tlk_rf_fs_rx_time rx_time);

void tlk_rf_reinitialize(void)
{
    // We may go to sleep before any RF configuration, so in such a case, there is nothing to
    // restore
    if (!tlk_rf_instance.initialized)
    {
        return;
    }

    // Otherwise, manually mark that all registers were reset
    tlk_rf_instance.initialized = false;

    enum tlk_rf_status status =
        tlk_rf_configure(tlk_rf_instance.mode, &tlk_rf_config, &tlk_rf_mode_config);

    tlk_rf_tx_fifo_set(
        tlk_rf_fifo_config.tx.buffer, tlk_rf_fifo_config.tx.depth, tlk_rf_fifo_config.tx.size);
    tlk_rf_rx_fifo_set(
        tlk_rf_fifo_config.rx.buffer, tlk_rf_fifo_config.rx.count, tlk_rf_fifo_config.rx.size);

    enum tlk_rf_fs_tx_time tx_time = tlk_rf_instance.fs.tx_time;
    enum tlk_rf_fs_rx_time rx_time = tlk_rf_instance.fs.rx_time;

    if (tx_time != TLK_RF_FS_TX_NONE || rx_time != TLK_RF_FS_RX_NONE)
    {
        tlk_rf_fs_set_calibration(tx_time, rx_time);
        tlk_rf_fs_configure(tx_time, rx_time);

        if (tlk_rf_instance.fs_tx_enabled)
        {
            tlk_rf_fs_tx_enable();
        }

        if (tlk_rf_instance.fs_rx_enabled)
        {
            tlk_rf_fs_rx_enable();
        }
    }

    if (status != TLK_RF_STATUS_OK)
    {
        // TODO: add error log
    }
}

TLK_REGISTER_AFTER_SUSPEND(tlk_rf_reinitialize, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)
TLK_REGISTER_AFTER_SLEEP(tlk_rf_reinitialize, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)

#endif

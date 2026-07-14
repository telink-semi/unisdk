#include "main.h"

#if TLK_IS_ENABLED(CONFIG_TLK_RF_DEMO_EXCHANGE_MANUAL)

#if TLK_IS_ENABLED(CONFIG_TLK_RF_DEMO_MODE_SWITCHING)

static bool is_switched;

static void switch_mode(void)
{
    if (is_switched)
    {
        rf_configure(CONFIG_TLK_RF_DEMO_MODE, CONFIG_TLK_RF_DEMO_BITRATE);
    }
    else
    {
        rf_configure(CONFIG_TLK_RF_DEMO_SECOND_MODE, CONFIG_TLK_RF_DEMO_SECOND_BITRATE);
    }

    is_switched = !is_switched;
}

#endif

#if TLK_IS_ENABLED(CONFIG_TLK_RF_DEMO_CHANNEL_SWITCHING)

static int8_t current_channel = 17;

static void switch_channel(void)
{
    if (current_channel == 17)
    {
        current_channel = 25;
    }
    else
    {
        current_channel = 17;
    }

    tlk_rf_set_ble_channel(current_channel);
}

static void fast_settle_init(void)
{
    tlk_rf_fs_calibrate(TLK_RF_FS_TX_TIME_MIN, TLK_RF_FS_RX_TIME_MIN);
    tlk_rf_fs_configure(TLK_RF_FS_TX_TIME_MIN, TLK_RF_FS_RX_TIME_MIN);

    tlk_rf_fs_rx_enable();
    tlk_rf_fs_tx_enable();
}

#endif

void transmitter_loop(void)
{
    TLK_IF_ENABLED(CONFIG_TLK_RF_DEMO_CHANNEL_SWITCHING, (fast_settle_init()));

    tlk_rf_set_tx_fifo(tx_buffer, TX_FIFO_DEPTH, TX_FIFO_SIZE);
    tlk_rf_set_rx_fifo(rx_buffer, RX_FIFO_COUNT, RX_FIFO_SIZE);

    while (1)
    {
        TLK_IF_ENABLED(CONFIG_TLK_RF_DEMO_CHANNEL_SWITCHING, (switch_channel()));

        tlk_rf_prepare_tx_packet(tx_packet, TX_PDU_SIZE);
        GET_PDU(tx_packet)->payload_size = PACKET_PAYLOAD_SIZE;
        GET_PDU(tx_packet)->payload[0]++;

        on_ping_sent();

        tlk_rf_set_tx_state();
        tlk_api_delay(tlk_rf_instance.fs.tx_time);
        tlk_rf_start_tx_state();

        ping = tlk_api_micros();

        while (!tlk_rf_instance.tx_end)
        {
            TLK_IF_ENABLED(CONFIG_TLK_RF_DEMO_POLLING_MODE, (tlk_rf_irq_process()));
        }

        tlk_rf_set_idle_state();

        tlk_rf_set_rx_state();
        tlk_api_delay(tlk_rf_instance.fs.rx_time);

        while (!tlk_rf_instance.rx_end && tlk_api_micros() - ping < TLK_MS_TO_US(PING_TIMEOUT_MS))
        {
            TLK_IF_ENABLED(CONFIG_TLK_RF_DEMO_POLLING_MODE, (tlk_rf_irq_process()));
        }

        tlk_rf_set_idle_state();

        ping = tlk_api_micros() - ping;

        on_pong_received();

#if TLK_IS_ENABLED(CONFIG_TLK_RF_DEMO_PM)
        /* The delay just to see that chip is active. */
        tlk_api_delay(TLK_MS_TO_US(250));

        tlk_api_sleep(1000 + 500);
#else
        tlk_api_delay(TLK_MS_TO_US(1000));
#endif

        TLK_IF_ENABLED(CONFIG_TLK_RF_DEMO_MODE_SWITCHING, (switch_mode()));
    }
}

void receiver_loop(void)
{
    TLK_IF_ENABLED(CONFIG_TLK_RF_DEMO_CHANNEL_SWITCHING, (fast_settle_init()));

    tlk_rf_set_tx_fifo(tx_buffer, TX_FIFO_DEPTH, TX_FIFO_SIZE);
    tlk_rf_set_rx_fifo(rx_buffer, RX_FIFO_COUNT, RX_FIFO_SIZE);

    while (1)
    {
        TLK_IF_ENABLED(CONFIG_TLK_RF_DEMO_CHANNEL_SWITCHING, (switch_channel()));

        tlk_rf_set_rx_state();
        tlk_api_delay(tlk_rf_instance.fs.rx_time);

        while (!tlk_rf_instance.rx_end)
        {
            TLK_IF_ENABLED(CONFIG_TLK_RF_DEMO_POLLING_MODE, (tlk_rf_irq_process()));
        }

        tlk_rf_set_idle_state();

        on_ping_received();

        memcpy(tx_buffer, tlk_rf_instance.rx_packet, TX_PACKET_SIZE);
        tlk_rf_prepare_tx_packet(tx_packet, TX_PDU_SIZE);
        GET_PDU(tx_packet)->payload_size = PACKET_PAYLOAD_SIZE;

        tlk_rf_set_tx_state();
        tlk_api_delay(tlk_rf_instance.fs.tx_time);
        tlk_rf_start_tx_state();

        while (!tlk_rf_instance.tx_end)
        {
            TLK_IF_ENABLED(CONFIG_TLK_RF_DEMO_POLLING_MODE, (tlk_rf_irq_process()));
        }

        tlk_rf_set_idle_state();

        on_pong_sent();

#if TLK_IS_ENABLED(CONFIG_TLK_RF_DEMO_PM)
        tlk_api_sleep(PING_TIMEOUT_MS);
#endif

        TLK_IF_ENABLED(CONFIG_TLK_RF_DEMO_MODE_SWITCHING, (switch_mode()));
    }
}

#endif

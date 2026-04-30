#include "main.h"

// The delay set between the rx and tx for receiver to be able to copy rx packet into tx buffer
#define RT_DELAY_US 100

#if IS_ENABLED(CONFIG_TLK_RF_DEMO_EXCHANGE_STR_SRT)

void transmitter_loop(void) 
{
    tlk_rf_set_tx_fifo(tx_buffer, TX_FIFO_DEPTH, TX_FIFO_SIZE);
    tlk_rf_set_rx_fifo(rx_buffer, RX_FIFO_COUNT, RX_FIFO_SIZE);

    while (1) {
        tlk_rf_prepare_tx_packet(tx_packet, TX_PDU_SIZE);
        GET_PDU(tx_packet)->payload_size = PACKET_PAYLOAD_SIZE;
        GET_PDU(tx_packet)->payload[0]++;

        on_ping_sent();

        tlk_rf_start_stx2rx(tlk_rf_get_tick(), UNISDK_RF_MAX_TIMEOUT_STX2RX, 0);

        ping = tlk_api_micros();
        
        while (!tlk_rf_instance.tx_end) { }

        while (!tlk_rf_instance.rx_end) { }

        ping = tlk_api_micros() - ping - RT_DELAY_US;
        
        on_pong_received();

#if IS_ENABLED(CONFIG_TLK_RF_DEMO_PM)
        /* The delay just to see that chip is active. */
        tlk_api_delay(TLK_M2USEC(250));

        tlk_api_sleep(1000 + 500);
#else
        tlk_api_delay(TLK_M2USEC(1000));
#endif
    }
}

void receiver_loop(void)
{
    tlk_rf_set_tx_fifo(tx_buffer, TX_FIFO_DEPTH, TX_FIFO_SIZE);
    tlk_rf_set_rx_fifo(rx_buffer, RX_FIFO_COUNT, RX_FIFO_SIZE);

    while (1) {
        do {
            tlk_rf_start_srx2tx(tlk_rf_get_tick(), UNISDK_RF_MAX_TIMEOUT_SRX2TX, RT_DELAY_US);

            while (!tlk_rf_instance.rx_end) { }
        } while (!tlk_rf_instance.rx_ok);

        memcpy(tx_buffer, tlk_rf_instance.rx_packet, TX_PACKET_SIZE);
        tlk_rf_prepare_tx_packet(tx_packet, TX_PDU_SIZE);
        GET_PDU(tx_packet)->payload_size = PACKET_PAYLOAD_SIZE;

        on_ping_received();
        
        while (!tlk_rf_instance.tx_end) { }

        on_pong_sent();

#if IS_ENABLED(CONFIG_TLK_RF_DEMO_PM)
        tlk_api_sleep(PING_TIMEOUT_MS);
#endif
    }
}

#endif
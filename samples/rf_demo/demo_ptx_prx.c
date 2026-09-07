#include "main.h"

// The delay set between the rx and tx for receiver to be able to copy rx packet into tx buffer
#define RT_DELAY_US 100

#if TLK_IS_ENABLED(CONFIG_TLK_RF_DEMO_EXCHANGE_PTX_PRX)

void transmitter_loop(void)
{
    tlk_rf_tx_fifo_set(tx_buffer, TX_FIFO_DEPTH, TX_FIFO_SIZE);
    tlk_rf_rx_fifo_set(rx_buffer, RX_FIFO_COUNT, RX_FIFO_SIZE);

#if TLK_IS_ENABLED(CONFIG_TLK_RF_DEMO_PIPE_SWITCHING)
    uint8_t code[] = {0x00, 0x11, 0x22, 0x33, 0x44};

    tlk_rf_set_access_code_length(5);

    for (uint8_t i = 0; i < UNISDK_RF_PIPE_COUNT; i++)
    {
        code[0] = i;
        tlk_rf_pipe_set_access_code(TLK_RF_PIPE(i), code);
    }

    tlk_rf_rx_pipe_enable(TLK_RF_PIPE_ALL);
#endif

    while (1)
    {
        tlk_rf_tx_prepare_packet(tx_packet, TX_PDU_SIZE);
#if TLK_IS_DISABLED(CONFIG_TLK_RF_DEMO_SB_FORMAT)
        GET_PDU(tx_packet)->payload_size = PACKET_PAYLOAD_SIZE;
#endif
        GET_PDU(tx_packet)->payload[0]++;

#if TLK_IS_ENABLED(CONFIG_TLK_RF_DEMO_PIPE_SWITCHING)
        tlk_rf_tx_pipe_set(TLK_RF_PIPE(GET_PDU(tx_packet)->payload[0] % UNISDK_RF_PIPE_COUNT));
#endif

        on_ping_sent();

        tlk_rf_start_ptx(tlk_rf_get_tick(), UNISDK_RF_MAX_TIMEOUT_PTX, 0);

        ping = tlk_api_time_get_micros();

        while (!tlk_rf_instance.tx_end)
        {
        }

        while (!tlk_rf_instance.rx_end)
        {
        }

        ping = tlk_api_time_get_micros() - ping - RT_DELAY_US;

        on_pong_received();

#if TLK_IS_ENABLED(CONFIG_TLK_RF_DEMO_PM)
        /* The delay just to see that chip is active. */
        tlk_api_time_delay(TLK_MS_TO_US(250));

        tlk_api_sleep(1000 + 500);
#else
        tlk_api_time_delay(TLK_MS_TO_US(1000));
#endif
    }
}

void receiver_loop(void)
{
    tlk_rf_tx_fifo_set(tx_buffer, TX_FIFO_DEPTH, TX_FIFO_SIZE);
    tlk_rf_rx_fifo_set(rx_buffer, RX_FIFO_COUNT, RX_FIFO_SIZE);

#if TLK_IS_ENABLED(CONFIG_TLK_RF_DEMO_PIPE_SWITCHING)
    uint8_t code[] = {0x00, 0x11, 0x22, 0x33, 0x44};

    tlk_rf_set_access_code_length(5);

    for (uint8_t i = 0; i < UNISDK_RF_PIPE_COUNT; i++)
    {
        code[0] = i;
        tlk_rf_pipe_set_access_code(TLK_RF_PIPE(i), code);
    }

    tlk_rf_rx_pipe_enable(TLK_RF_PIPE_ALL);
#endif

    while (1)
    {
        do
        {
            tlk_rf_start_prx(tlk_rf_get_tick(), UNISDK_RF_MAX_TIMEOUT_PRX, RT_DELAY_US);

            while (!tlk_rf_instance.rx_end)
            {
            }
        } while (!tlk_rf_instance.rx_ok);

        memcpy(tx_buffer, tlk_rf_instance.rx_packet, TX_PACKET_SIZE);
        tlk_rf_tx_prepare_packet(tx_packet, TX_PDU_SIZE);
#if TLK_IS_DISABLED(CONFIG_TLK_RF_DEMO_SB_FORMAT)
        GET_PDU(tx_packet)->payload_size = PACKET_PAYLOAD_SIZE;
#endif

        on_ping_received();

        while (!tlk_rf_instance.tx_end)
        {
        }

        on_pong_sent();

#if TLK_IS_ENABLED(CONFIG_TLK_RF_DEMO_PM)
        tlk_api_sleep(PING_TIMEOUT_MS);
#endif
    }
}

#endif

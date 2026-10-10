#ifndef RF_DRIVER_HPP
#define RF_DRIVER_HPP

#include <cstddef>
#include <cstdint>

#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_rf.h"
#ifdef __cplusplus
}
#endif

class RfDriver
{
  public:
    RfDriver() = delete;

    static tlk_rf_status configure(tlk_rf_mode mode, tlk_rf_config* rf_config,
                                   tlk_rf_mode_config* rf_mode_config)
    {
        return tlk_rf_configure(mode, rf_config, rf_mode_config);
    }

    static void prepareTxPacket(tlk_rf_packet* packet, uint8_t pdu_size)
    {
        tlk_rf_prepare_tx_packet(packet, pdu_size);
    }

    static uint8_t rxPacketSizeToPayloadSize(tlk_rf_mode mode, uint8_t packet_size)
    {
        return tlk_rf_rx_packet_size_to_payload_size(mode, packet_size);
    }

    static tlk_rf_status setBitrate(tlk_rf_bitrate bitrate)
    {
        return tlk_rf_set_bitrate(bitrate);
    }

    static tlk_rf_status setPower(int8_t power)
    {
        return tlk_rf_set_power(power);
    }

    static void setChannel(int8_t channel)
    {
        tlk_rf_set_channel(channel);
    }

    static void setBleChannel(int8_t ble_channel)
    {
        tlk_rf_set_ble_channel(ble_channel);
    }

    static void setAccessCode(uint32_t access_code)
    {
        tlk_rf_set_access_code(access_code);
    }

    static void setPreambleLength(uint8_t preamble_length)
    {
        tlk_rf_set_preamble_length(preamble_length);
    }

    static void resetBaseband()
    {
        tlk_rf_reset_baseband();
    }

    static void resetRegisters()
    {
        tlk_rf_reset_registers();
    }

    static void setIrqHandler(tlk_rf_irq_handler_t handler)
    {
        tlk_rf_set_irq_handler(handler);
    }

    static tlk_rf_status irqEnable(tlk_rf_irq mask)
    {
        return tlk_rf_irq_enable(mask);
    }

    static tlk_rf_status irqDisable(tlk_rf_irq mask)
    {
        return tlk_rf_irq_disable(mask);
    }

    static tlk_rf_irq irqGetMask()
    {
        return tlk_rf_irq_get_mask();
    }

    static tlk_rf_irq irqGetStatus()
    {
        return tlk_rf_irq_get_status();
    }

    static void irqHandle(tlk_rf_irq mask)
    {
        tlk_rf_irq_handle(mask);
    }

    static void irqProcess()
    {
        tlk_rf_irq_process();
    }

    static void setTxFifo(uint8_t* buffer, uint8_t depth, uint16_t size)
    {
        tlk_rf_set_tx_fifo(buffer, depth, size);
    }

    static void setRxFifo(uint8_t* buffer, uint8_t count, uint16_t size)
    {
        tlk_rf_set_rx_fifo(buffer, count, size);
    }

    static void setTxSettleTime(uint16_t time_us)
    {
        tlk_rf_set_tx_settle_time(time_us);
    }

    static void setRxSettleTime(uint16_t time_us)
    {
        tlk_rf_set_rx_settle_time(time_us);
    }

    static void startTx()
    {
        tlk_rf_start_tx_state();
    }

    static void setTxState()
    {
        tlk_rf_set_tx_state();
    }

    static void setRxState()
    {
        tlk_rf_set_rx_state();
    }

    static void setIdleState()
    {
        tlk_rf_set_idle_state();
    }

    static uint32_t getTick()
    {
        return tlk_rf_get_tick();
    }

    static uint32_t usToTick(uint32_t us)
    {
        return TLK_RF_US_TO_TICK(us);
    }

    static tlk_rf_rx_packet_tail* getRxPacketTail(tlk_rf_packet* packet)
    {
        return TLK_RF_GET_RX_PACKET_TAIL(packet);
    }

    static void startStx(uint32_t tick)
    {
        tlk_rf_start_stx(tick);
    }

    static void startSrx(uint32_t tick, uint32_t timeout)
    {
        tlk_rf_start_srx(tick, timeout);
    }

    static void startStx2rx(uint32_t tick, uint32_t timeout, uint16_t delay)
    {
        tlk_rf_start_stx2rx(tick, timeout, delay);
    }

    static void startSrx2tx(uint32_t tick, uint32_t timeout, uint16_t delay)
    {
        tlk_rf_start_srx2tx(tick, timeout, delay);
    }

    static void startBtx(uint32_t tick, uint32_t timeout, uint16_t delay)
    {
        tlk_rf_start_btx(tick, timeout, delay);
    }

    static void startBrx(uint32_t tick, uint32_t timeout, uint16_t delay)
    {
        tlk_rf_start_brx(tick, timeout, delay);
    }

    static void startPtx(uint32_t tick, uint32_t timeout, uint16_t delay)
    {
        tlk_rf_start_ptx(tick, timeout, delay);
    }

    static void startPrx(uint32_t tick, uint32_t timeout, uint16_t delay)
    {
        tlk_rf_start_prx(tick, timeout, delay);
    }

    static void stopTxRxAuto()
    {
        tlk_rf_stop_tx_rx_auto();
    }

    static tlk_rf_status fsConfigure(tlk_rf_fs_tx_time tx_time, tlk_rf_fs_rx_time rx_time)
    {
        return tlk_rf_fs_configure(tx_time, rx_time);
    }

    static tlk_rf_status fsCalibrate(tlk_rf_fs_tx_time tx_time, tlk_rf_fs_rx_time rx_time)
    {
        return tlk_rf_fs_calibrate(tx_time, rx_time);
    }

    static void fsTxEnable()
    {
        tlk_rf_fs_tx_enable();
    }

    static void fsRxEnable()
    {
        tlk_rf_fs_rx_enable();
    }

    static void fsTxDisable()
    {
        tlk_rf_fs_tx_disable();
    }

    static void fsRxDisable()
    {
        tlk_rf_fs_rx_disable();
    }

    static tlk_rf_module& instance()
    {
        return tlk_rf_instance;
    }
};

#endif

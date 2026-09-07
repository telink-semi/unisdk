#include "core/include/tlk_analog.h"
#include "core/include/tlk_stimer.h"
#include "drivers/include/tlk_advanced_stimer.h"
#include "registers/tlk_analog.h"
#include "registers/tlk_stimer.h"

unsigned int tlk_g_track_32kcnt = 16;

/**
 * @brief   This interface is used to obtain 32k track count.
 * @param[in] mode - the enumeration of extended 32k stimer modes.
 * @return  32k track count.
 */
_tlk_attribute_ram_code_sec_ void
tlk_stimer_set_32k_count_mode(tlk_advanced_system_timer_32k_cnt_mode_e mode)
{
    tlk_stimer_reg.sys_timer_up = mode;
}

/**
 * @brief   This interface is used to configure the 32k track mode.
 *          (2^(16-(cnt>>4)))cycles of 32k clock indicates the tick number of the system timer.
 * @return  none.
 */
void tlk_stimer_set_32k_track_cnt(tlk_stimer_track_cnt_e cnt)
{
    tlk_stimer_reg.control_bit.cal_32k_mode = cnt;
    tlk_g_track_32kcnt                      = (1 << (16 - (cnt >> 4)));
}

/**
 * @brief   This interface is used to obtain 32k track count.
 * @return  32k track count.
 */
_tlk_attribute_ram_code_sec_ uint32_t tlk_stimer_get_32k_track_cnt(void)
{
    return tlk_g_track_32kcnt;
}

_tlk_attribute_ram_code_sec_ void tlk_stimer_set_32k_tick(uint32_t tick)
{
    tlk_stimer_set_32k_write(true); // enable 32k tick write function
    while (tlk_stimer_get_state() & FLD_SYSTEM_RD_BUSY)
        ;
    tlk_stimer_reg.set_32k_time = tick;
    tlk_stimer_set_state(FLD_SYSTEM_CMD_SYNC);
    /**
     * This delay time is about 1.38us under the calibrated 24M RC clock.
     * The minimum waiting time here is 3*pclk cycles+3*24M xtal cycles, a total of 0.25us,
     * wait 0.25us before you can use wr_busy signal for judgment, jianzhi suggested that this time
     * to 1us is enough. add by bingyu.li, confirmed by jianzhi.chen 20231115
     */
    // tlk_clock_cclk_delay(tlk_sys_clk.cclk);
    while (tlk_stimer_get_state() & FLD_SYSTEM_CMD_SYNC)
        ;
}

/**
 * @brief  This function serves to get the 32k tick currently.
 * @return the current 32k tick.
 */
_tlk_attribute_ram_code_sec_ unsigned int tlk_stimer_get_32k_tick(void)
{
#if TLK_IS_ENABLED(CONFIG_TLK_SYSTEM_TIMER_ADVANCED_USE_32K_TICK_ANALOG_READOUT)
    /*
     * modify by yi.bao,confirmed by guangjun at 20210105
     * Use digital register way to get 32k tick may read error tick,cause the wakeup time is
     * incorrect with the setting time,the sleep time will very little or very big,will not wakeup
     * on time.
     */
    unsigned int t0 = 0;
    unsigned int t1 = 0;

    t0 = tlk_analog_read_reg32(TLK_AREG_CLK_CNT_32K);
    while (1)
    {
        t1 = tlk_analog_read_reg32(TLK_AREG_CLK_CNT_32K);
        if ((t1 - t0) == 1)
        {
            return t1;
        }
        else if (t1 - t0)
        {
            t0 = t1;
        }
    }

#else
    // In the system timer auto mode, when writing a tick value to the system tick, if the writing
    // operation overlaps with the 32k rising edge, the writing operation will be unsuccessful. When
    // reading the 32k tick value, first wait for the rising edge to pass to avoid overlap with the
    // subsequent write tick value operation. modify by weihua.zhang, confirmed by jianzhi at
    // 20210126
    unsigned int timer_32k_tick;
    tlk_stimer_set_state(FLD_SYSTEM_CLR_RD_DONE);
    while ((tlk_stimer_get_state() & FLD_SYSTEM_CLR_RD_DONE) != 0)
        ;                            // wait rd_done = 0;
    tlk_stimer_set_32k_write(false); // 1:32k write mode; 0:32k read mode
    while ((tlk_stimer_get_state() & FLD_SYSTEM_CLR_RD_DONE) == 0)
        ; // wait rd_done = 1;
    timer_32k_tick = tlk_stimer_reg.read_32k_time;
    tlk_stimer_set_32k_write(true); // 1:32k write mode; 0:32k read mode
    return timer_32k_tick;
#endif
}

/**
 * @brief   This interface is used to obtain stimer 32k tracking cycles value.
 * @return  input capture value.
 */
_tlk_attribute_ram_code_sec_ uint32_t tlk_stimer_get_32k_tracking_cycles(void)
{
    return tlk_stimer_reg.cal_latch;
}

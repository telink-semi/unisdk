#include "core/include/tlk_clock.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_stimer.h"
#include "core/include/tlk_sys.h"
#include "registers/tlk_analog.h"
#include "registers/tlk_clock.h"
#include "registers/tlk_soc.h"

#define TLK_CLOCK_24M_XTAL_STABLE_LOOP_COUNT 10
#define TLK_CLOCK_24M_XTAL_STABLE_TIME_US    40
#define TLK_ALG_MODULE_MAX_CLK               30
#define TLK_CLOCK_32K_RC_STABLE_TIME_US      10000
#define TLK_CLOCK_24M_RC_STABLE_TIME_US      5000

extern uint8_t tlk_g_chip_version;

struct tlk_clock_config
{
    enum tlk_clock_32k_source clk_32k_source;
    uint16_t                  pll_frequency;
    uint8_t                   clk_24m_rc_use_count;
};

struct tlk_clock_sys_clk_config tlk_default_sys_clk = {
    .source     = TLK_CLOCK_24M_RC,
    .source_clk = 24,
    .cclk       = 24,
    .hclk       = 24,
    .pclk       = 24,
    .mspi_clk   = 24,
};

struct tlk_clock_sys_clk_config tlk_sys_clk = {
    .source = TLK_CLOCK_NONE,
};

struct tlk_clock_config tlk_clock = {

};

_tlk_attribute_ram_code_sec_ uint8_t tlk_clock_get_pll_reg_value(uint16_t pll_clk)
{
    switch (pll_clk)
    {
    case 192:
        return 0x10;
    case 240:
        return 0x12;
    case 288:
        return 0x14;
    default:
        return 0;
    }
}

_tlk_attribute_ram_code_sec_ void tlk_clock_pll_configure(uint16_t frequency)
{
    if (tlk_clock.pll_frequency == frequency)
    {
        return;
    }

    tlk_clock.pll_frequency = frequency;

    if (frequency == 0)
    {
        // power down PLL
        TLK_ANALOG_MODIFY(TLK_AREG_POWER_DOWN1, (value.bit.pd_bbpll_ldo = 1;));
        return;
    }

    uint8_t pll_reg_value = tlk_clock_get_pll_reg_value(frequency);

    TLK_ANALOG_MODIFY(TLK_AREG_BBPLL_CTRL6, (value.bit.bbpll_240M_div_ratio = pll_reg_value;));

    TLK_ANALOG_MODIFY(TLK_AREG_BBPLL_CTRL4, (value.bit.bbpll_240M_vco_itrim = pll_reg_value >> 5;));

    TLK_ANALOG_MODIFY(TLK_AREG_POWER_DOWN1, (value.bit.pd_bbpll_ldo = 0;));
    /*
        trim bit for VCO ibias: from default 101 to 010. Issue(TER-111)
        (add by jilong.liu, confirmed by yangya 20250107)
    */
    TLK_ANALOG_MODIFY(TLK_AREG_BBPLL_CTRL4, (value.bit.bbpll_240M_vco_itrim = 2;));

    enum tlk_clock_status status = tlk_clock_pll_is_ready();
    if (status != TLK_CLOCK_OK)
    {
        tlk_sys_reboot(TLK_SYS_SW_REBOOT_PLL_UNSTABLE);
    }
}

_tlk_attribute_ram_code_sec_ void tlk_clock_32k_configure(enum tlk_clock_32k_source source)
{
    TLK_ANALOG_MODIFY(TLK_AREG_XO_CTRL0, (value.bit.clk32k_sel = source;));

    TLK_ANALOG_MODIFY(TLK_AREG_POWER_DOWN0, (
        value.bit.pd_32k_rc = (source != TLK_CLOCK_32K_RC);
        value.bit.pd_32k_xtal = (source != TLK_CLOCK_32K_XTAL);
    ));

    tlk_clock.clk_32k_source = source;
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_32k_source tlk_clock_32k_get_source(void)
{
    return tlk_clock.clk_32k_source;
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_status
tlk_clock_sys_clk_configure(const struct tlk_clock_sys_clk_config* sys_clk_config)
{
    tlk_clock_24m_rc_acquire();

    if (tlk_sys_clk.source == TLK_CLOCK_24M_RC)
    {
        tlk_clock_24m_rc_release();
    }

    tlk_sys_clk = *sys_clk_config;

    if (sys_clk_config->source == TLK_CLOCK_PLL)
    {
        tlk_clock_pll_configure(sys_clk_config->source_clk);
    }

    // change to 24M rc first.
    tlk_clock_reg.cclk_set = (tlk_clock_reg.cclk_set & 0xc0) | (TLK_CLOCK_24M_RC << 4) | 1;

    // HCLK and PCLK should be set ahead of CCLK, ensure the HCLK and PCLK not exceed the max
    // CCLK(CCLK max 120M, HCLK max 60M, PCLK max 60M)
    uint8_t hclk_div = sys_clk_config->cclk / sys_clk_config->hclk;
    uint8_t pclk_div = sys_clk_config->hclk / sys_clk_config->pclk;
    tlk_clock_reg.busclk_ratio =
        (tlk_clock_reg.busclk_ratio & 0xf8) | ((hclk_div >> 1) << 2) | (pclk_div >> 1);

    // Configure the CCLK clock frequency.
    uint8_t cclk_div       = sys_clk_config->source_clk / sys_clk_config->cclk;
    tlk_clock_reg.cclk_set = (tlk_clock_reg.cclk_set & 0xc0) | (sys_clk_config->source << 4) |
                             cclk_div; // clock source. 0:rc 24m, 1:xtl_24m, 2:pll

    // mspi configuration
    uint8_t mspi_div        = sys_clk_config->source_clk / sys_clk_config->mspi_clk;
    tlk_clock_reg.mspi_mode = (tlk_clock_reg.mspi_mode & 0xc0) | (sys_clk_config->source << 4) |
                              mspi_div; // src:bit[5:4], div:bit[3:0]

    // The maximum pclk is 120. If it is greater than TLK_ALG_MODULE_MAX_CLK*2,
    // it should be divided into 4 frequencies; otherwise, it can be divided into two frequencies.
    uint8_t analog_div = (sys_clk_config->pclk > TLK_ALG_MODULE_MAX_CLK * 2) ? 1 : 0;
    tlk_analog_reg.dma_control_bit.div_mode = analog_div;

    if (sys_clk_config->source != TLK_CLOCK_PLL)
    {
        tlk_clock_pll_configure(0);
    }

    if (sys_clk_config->source != TLK_CLOCK_24M_RC)
    {
        tlk_clock_24m_rc_release();
    }

    return TLK_CLOCK_OK;
}

_tlk_attribute_ram_code_sec_ uint64_t tlk_clock_cclk_get_tick(void)
{
#if __riscv_xlen == 32
    do
    {
        uint32_t hi = tlk_read_csr(NDS_MCYCLEH);
        uint32_t lo = tlk_read_csr(NDS_MCYCLE);

        if (hi == tlk_read_csr(NDS_MCYCLEH))
        {
            return ((uint64_t) hi << 32) | lo;
        }
    } while (1);
#else
    return tlk_read_csr(NDS_MCYCLE);
#endif
}

_tlk_attribute_ram_code_sec_ void tlk_clock_cclk_delay(uint64_t delay_tick)
{
    uint64_t start = tlk_clock_cclk_get_tick();
    while (tlk_clock_cclk_get_tick() - start < delay_tick)
    {
    }
}

_tlk_attribute_ram_code_sec_ void tlk_clock_24m_xtal_enable(void)
{
    if ((tlk_g_chip_version != TLK_CHIP_VERSION_A0) && (tlk_g_chip_version != TLK_CHIP_VERSION_A1))
    {
        // Since the A2 version changed the default meaning of crystal ldo trim voltage, the default
        // voltage of crystal ldo trim of A2 chips is lower than the previous version, and some A2
        // chips do not work RF due to low crystal ldo trim voltage. So modify this configuration on
        // the A2 version of the chip to resolve the RF anomaly.Add by zhiwei,confirmed by
        // wenfeng.20241206
        TLK_ANALOG_MODIFY(TLK_AREG_XO_CTRL2, (value.bit.xo_ldo_trim_ana = 3;));
    }
    TLK_ANALOG_MODIFY(TLK_AREG_WAKEUP_SET, (value.bit.reset_xtal_quick_start_cnt = 1;));

    union tlk_areg_power_down0 pd_reg = TLK_ANALOG_READ(TLK_AREG_POWER_DOWN0);
    pd_reg.bit.pd_24m_xtal            = 1;
    TLK_ANALOG_WRITE(TLK_AREG_POWER_DOWN0, pd_reg);
    pd_reg.bit.pd_24m_xtal = 0;
    TLK_ANALOG_WRITE(TLK_AREG_POWER_DOWN0, pd_reg);
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_status tlk_clock_24m_xtal_is_ready(void)
{
    // To judge the stability of the crystal oscillator, xo_ready_ana is invalid,
    // so an alternative solution to judge is used: Because the clock source of the stimer is the
    // crystal oscillator, if the crystal oscillator does not vibrate, the tick value of the stimer
    // does not increase or increases very slowly (when there is interference). So first use 24M RC
    // to run the program and wait for a fixed time, calculate the number of ticks that the stimer
    // should increase during this time, and then read the tick value actually increased by the
    // stimer. When it reaches 50% of the calculated value, it proves that the crystal oscillator
    // has started.
    enum tlk_clock_status status = TLK_CLOCK_ERROR;

    // The timer works abnormally at low temperatures,
    // resulting in frequency bias because the capacitor is not fully added,
    // and the crystal is not connected to the system timer.
    // bit1 configuration capacitors are added on.
    // bit3 configuration crystals are connected to system timer.(added by weihua, confirmed by
    // wenfeng. 20241026)
    TLK_ANALOG_MODIFY(TLK_AREG_XO_CTRL2, (
        value.bit.xo_dyn_cap_ana = 0;
        value.bit.xo_cnt_off_ana = 1;
    ));

    tlk_stimer_enable();

    for (volatile uint8_t i = 0; i < TLK_CLOCK_24M_XTAL_STABLE_LOOP_COUNT; i++)
    {
        volatile uint32_t tick = tlk_stimer_get_tick();
        // Note: Ensure that the current clock is 24M RC, otherwise the nop running time
        // will not be TLK_CLOCK_24M_XTAL_STABLE_TIME_US, resulting in a later calculation error.
        // (add by weihua.zhang, confirmed by peng.sun 20230609)
        tlk_clock_cclk_delay(tlk_sys_clk.cclk * TLK_CLOCK_24M_XTAL_STABLE_TIME_US);

        if (tlk_stimer_get_tick() - tick >
            TLK_SYSTEM_TIMER_TICK_1US * TLK_CLOCK_24M_XTAL_STABLE_TIME_US / 2)
        {
            status = TLK_CLOCK_OK;
            break;
        }
    }

    tlk_stimer_disable();

    return status;
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_status tlk_clock_pll_is_ready(void)
{
    TLK_ANALOG_MODIFY(TLK_AREG_BBPLL_CTRL5, (value.bit.bbpll_240M_lock_en2x = 1;));
    TLK_ANALOG_MODIFY(TLK_AREG_BBPLL_CTRL6, (value.bit.bbpll_240M_lock_rst = 1;));

    // tl721x A0 version PLL has a issue(TER-9) that pll cannot get ready signal so here need a vco
    // trim process. A1 version has fixed this issue.
    if (tlk_g_chip_version == TLK_CHIP_VERSION_A0)
    {
        uint8_t pll_vco_itrim = 0;
        bool    trim_ok       = false;

        switch (tlk_sys_clk.source_clk)
        {
        case 192:
            pll_vco_itrim = 7;
            break;
        case 240:
            pll_vco_itrim = 5;
            break;
        case 288:
            pll_vco_itrim = 1;
            break;
        default:
            return TLK_CLOCK_ERROR;
        }

        /*
         * The default vco ibias value may not be sufficient to support PLL stability.
         * In this case, it is necessary to try adjusting the trim value until a stable PLL signal
         * is obtained. Note: If the obtained gear value here is relatively marginal, it may still
         * affect the stability of PLL in the case of large temperature changes in the future. The
         * tl721x chip select 240MHz as PLL frequency, so the start point vco is 101(5). (add by
         * jilong.liu, confirmed by yangya 20240130)
         */
        for (uint8_t vco_trim = pll_vco_itrim; vco_trim > 0; vco_trim--)
        {
            tlk_clock_cclk_delay(tlk_sys_clk.cclk * 20); // 20us, wait vco ibias trim stable
            for (uint8_t i = 0; i < 3; i++)
            {
                if (TLK_ANALOG_READ(TLK_AREG_BBPLL_CTRL8).bit.bbpll_lock_detector)
                {
                    trim_ok = true;
                    break;
                }
            }
            if (trim_ok)
            {
                break;
            }
            TLK_ANALOG_MODIFY(TLK_AREG_BBPLL_CTRL4, (value.bit.bbpll_240M_vco_itrim = vco_trim - 1;));
        }

        if (!trim_ok)
        {
            return TLK_CLOCK_ERROR;
        }
    }
    else
    {
        uint8_t pll_ok    = 0;
        uint8_t trim_time = 0;
        /*
         * afe0p8v_reg04<7:5>(bbpll_240M_vco_itrim<2:0>) trim bit for VCO ibias:
         * The results show that the central value of VCO is 2(0x40).
         * If the central value cannot get PLL ready, try to diverge the values towards both ends in
         * sequence. (add by jilong.liu, confirmed by yangya at 20250107)
         */
        uint8_t  pll_vco_value[4] = {3, 1, 4, 0};
        uint64_t start0           = tlk_clock_cclk_get_tick();
        uint64_t start1           = tlk_clock_cclk_get_tick();
        /*
         * The standard for judging the stability of PLL is that the ready flag bit read three times
         * in a row is 1. (add by jilong.liu, confirmed by wenfeng.lou 20241203)
         */
        while (pll_ok < 3)
        {
            if (TLK_ANALOG_READ(TLK_AREG_BBPLL_CTRL8).bit.bbpll_lock_detector)
            {
                pll_ok++;
            }
            else
            {
                pll_ok = 0;
            }
            /*
             * Each vco value check for 200us, if the PLL still not stable after completing one
             * round trim, reset the vco value to the central value and repeat the trim process
             * until it stabilizes or 10ms times out.
             */
            if (tlk_clock_cclk_get_tick() - start0 > 200 * tlk_sys_clk.cclk)
            {
                TLK_ANALOG_MODIFY(TLK_AREG_BBPLL_CTRL4, (
                    if (trim_time < 4) {
                        value.bit.bbpll_240M_vco_itrim = pll_vco_value[trim_time];
                        trim_time++;
                    } else {
                        value.bit.bbpll_240M_vco_itrim = 2; // the central value of VCO
                        trim_time = 0;
                    }
                ));

                start0 = tlk_clock_cclk_get_tick();
            }
            // increase timeout to 10ms in case.
            if (tlk_clock_cclk_get_tick() - start1 > 1000 * 10 * tlk_sys_clk.cclk)
            {
                return TLK_CLOCK_ERROR;
            }
        }
    }

    TLK_ANALOG_MODIFY(TLK_AREG_BBPLL_CTRL5, (value.bit.bbpll_240M_lock_en2x = 0;));
    TLK_ANALOG_MODIFY(TLK_AREG_BBPLL_CTRL6, (value.bit.bbpll_240M_lock_rst = 0;));

    return TLK_CLOCK_OK;
}

_tlk_attribute_ram_code_sec_ bool tlk_clock_24m_rc_acquire(void)
{
    if (tlk_clock.clk_24m_rc_use_count++ == 0)
    {
        TLK_ANALOG_MODIFY(TLK_AREG_POWER_DOWN0, (value.bit.pd_24m_rc = 0;));

        // the calibration of 24m RC should wait for 1us if just power it up.
        // (added by jilong.liu, confirmed by yangya at 20240805)
        tlk_clock_cclk_delay((unsigned long long) (2 * tlk_sys_clk.cclk));

        return false;
    }

    return true;
}

_tlk_attribute_ram_code_sec_ bool tlk_clock_24m_rc_release(void)
{
    if (--tlk_clock.clk_24m_rc_use_count == 0)
    {
        TLK_ANALOG_MODIFY(TLK_AREG_POWER_DOWN0, (value.bit.pd_24m_rc = 1;));

        return false;
    }

    return true;
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_status tlk_clock_32k_rc_calibrate(void)
{
    enum tlk_clock_status status = TLK_CLOCK_OK;

    TLK_ANALOG_MODIFY(TLK_AREG_CLK_CTRL1, (value.bit.rc_32k_cap_sel = 1;));

    union tlk_areg_cal_ctrl0 cal_ctrl = {.raw = 0xf6}; // cal_32k_en and cal_32k_test disabled
    TLK_ANALOG_WRITE(TLK_AREG_CAL_CTRL0, cal_ctrl);
    cal_ctrl.bit.cal_32k_en = 1;
    TLK_ANALOG_WRITE(TLK_AREG_CAL_CTRL0, cal_ctrl);

    TLK_WAIT_FOR_TRUE_OR_TIMEOUT((TLK_ANALOG_READ(TLK_AREG_CAL_CTRL3).bit.cal_32k_done),
                                 (TLK_CLOCK_32K_RC_STABLE_TIME_US),
                                 (status = TLK_CLOCK_ERROR;))

    tlk_analog_write_reg8(TLK_AREG_CLK_RC_32K_RES_H, tlk_analog_read_reg8(TLK_AREG_CAL_RC_32K_CAP));
    TLK_ANALOG_MODIFY(TLK_AREG_CLK_CTRL1,
        (value.bit.rc_32k_res_l = TLK_ANALOG_READ(TLK_AREG_CAL_CTRL2).bit.cal_rc_32k_res;));

    cal_ctrl.bit.cal_32k_en = 0;
    TLK_ANALOG_WRITE(TLK_AREG_CAL_CTRL0, cal_ctrl);

    TLK_ANALOG_MODIFY(TLK_AREG_CLK_CTRL1, (value.bit.rc_32k_cap_sel = 0;));

    return status;
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_status tlk_clock_24m_rc_calibrate(void)
{
    enum tlk_clock_status status = TLK_CLOCK_OK;

    tlk_clock_24m_rc_acquire();

    tlk_analog_write_reg8(TLK_AREG_CAL_24M_WAIT_LEN, 0x80);

    TLK_ANALOG_MODIFY(TLK_AREG_CLK_CTRL1, (value.bit.rc_24m_cap_sel = 1;));

    union tlk_areg_cal_ctrl1 cal_ctrl = {.raw = 0x0e}; // cal_24m_en disabled
    TLK_ANALOG_WRITE(TLK_AREG_CAL_CTRL1, cal_ctrl);
    cal_ctrl.bit.cal_24m_en = 1;
    TLK_ANALOG_WRITE(TLK_AREG_CAL_CTRL1, cal_ctrl);

    TLK_WAIT_FOR_TRUE_OR_TIMEOUT((TLK_ANALOG_READ(TLK_AREG_CAL_CTRL3).bit.cal_24m_done),
                                 (TLK_CLOCK_24M_RC_STABLE_TIME_US),
                                 (status = TLK_CLOCK_ERROR;))

    tlk_analog_write_reg8(TLK_AREG_CLK_RC_24M_CAP, tlk_analog_read_reg8(TLK_AREG_CAL_RC_24M_CAP));

    TLK_ANALOG_MODIFY(TLK_AREG_CLK_CTRL1, (value.bit.rc_24m_cap_sel = 0;));

    cal_ctrl.bit.cal_24m_en = 0;
    TLK_ANALOG_WRITE(TLK_AREG_CAL_CTRL1, cal_ctrl);

    tlk_clock_24m_rc_release();

    return status;
}

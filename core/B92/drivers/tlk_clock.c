#include "core/include/tlk_clock.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_stimer.h"
#include "core/include/tlk_sys.h"
#include "drivers/include/tlk_mspi.h"
#include "registers/tlk_analog.h"
#include "registers/tlk_clock.h"

#define TLK_CLOCK_24M_XTAL_STABLE_LOOP_COUNT 10
#define TLK_CLOCK_24M_XTAL_STABLE_TIME_US    40
#define TLK_CLOCK_32K_RC_STABLE_TIME_US      10000
#define TLK_CLOCK_24M_RC_STABLE_TIME_US      5000

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
    // something requires rc24m to be enabled for the correct pm, so for now, it is always enabled
    .clk_24m_rc_use_count = 1,
};

_tlk_attribute_ram_code_sec_ uint8_t tlk_clock_get_pll_reg_value(uint16_t pll_clk)
{
    switch (pll_clk)
    {
    case 48:
        return 0 | (16 << 2);
    case 54:
        return 0 | (17 << 2);
    case 60:
        return 0 | (18 << 2);
    case 66:
        return 0 | (19 << 2);
    case 96:
        return 1 | (16 << 2);
    case 108:
        return 1 | (17 << 2);
    case 120:
        return 1 | (18 << 2);
    case 132:
        return 1 | (19 << 2);
    case 192:
        return 2 | (16 << 2);
    case 216:
        return 2 | (17 << 2);
    case 240:
        return 2 | (18 << 2);
    case 264:
        return 2 | (19 << 2);
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
    TLK_ANALOG_MODIFY(TLK_AREG_BBPLL_CTRL0, (value.bit.bbpll_m_div_i = pll_reg_value >> 2;));
    TLK_ANALOG_MODIFY(TLK_AREG_POWER3, (value.bit.bbpll_freq_sel = pll_reg_value & 3;));

    // power up PLL
    TLK_ANALOG_MODIFY(TLK_AREG_POWER_DOWN1, (value.bit.pd_bbpll_ldo = 0;));

    // wait for PLL stable
    TLK_ANALOG_MODIFY(TLK_AREG_BBPLL_CTRL1, (value.bit.bbpll_lck_det_rstb = 1;));
    while (!(TLK_ANALOG_READ(TLK_AREG_XO_CAL_CTRL).bit.doubler_cal_done))
    {
    }
    TLK_ANALOG_MODIFY(TLK_AREG_BBPLL_CTRL1, (value.bit.bbpll_lck_det_rstb = 0;));

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

    // ensure mspi is not in busy status before change mspi clock
    tlk_mspi_stop_xip();

    // first cclk/mspi_clk switch to 24rc to avoid the risk of hclk/pclk/mspi_clk
    // exceeding its maximum configurable frequency for a short period of time
    // when switching different clock frequencies using this interface.
    tlk_clock_reg.clock_sel0_bit.sclk_sel       = 0b000; // cclk to 24M rc clock
    tlk_clock_reg.mspi_clk_set_bit.clkdiv_hs_rc = 0;     // mspi_clk to 24M rc clock

    if (sys_clk_config->source == TLK_CLOCK_PLL)
    {
        tlk_clock_pll_configure(sys_clk_config->source_clk);
    }
    else
    {
        tlk_clock_pll_configure(0);
    }

    // usb clock (192M/4 =48M) pll clock should be the multiple of 48, because USB clock is 48M.
    tlk_clock_reg.usb_clk = sys_clk_config->source_clk / 48;

    uint8_t mspi_clk_div = sys_clk_config->source_clk / sys_clk_config->mspi_clk;
    if (mspi_clk_div == 1)
    {
        tlk_clock_reg.mspi_clk_set_bit.clkdiv_hs_rc = 0;
    }
    else
    {
        tlk_clock_reg.mspi_clk_set_bit.clkmod = mspi_clk_div;
        // if the div is odd, should set two times to ensure the correct sequence.
        tlk_clock_reg.mspi_clk_set_bit.clkdiv_hs_rc = 1;
        tlk_clock_reg.mspi_clk_set_bit.clkdiv_hs_rc = 1;
    }
    tlk_mspi_set_xip_en();

    // hclk and pclk should be set ahead of cclk, ensure the hclk and pclk not exceed the max
    // clk(cclk max 96M, hclk max 48M, pclk max 48M)
    uint8_t hclk_div = sys_clk_config->cclk / sys_clk_config->hclk;
    if (hclk_div == 1)
    {
        tlk_clock_reg.hclk_pclk_bit.hclk_div = 0;
    }
    else
    {
        tlk_clock_reg.hclk_pclk_bit.hclk_div = 1;
    }

    // pclk can div1/div2/div4 from hclk.
    uint8_t pclk_div = sys_clk_config->hclk / sys_clk_config->pclk;
    if (pclk_div == 1)
    {
        tlk_clock_reg.hclk_pclk_bit.pclk_div = 0;
    }
    else
    {
        tlk_clock_reg.hclk_pclk_bit.pclk_div = pclk_div / 2;
    }

    // select cclk source(TLK_RC24M/TLK_PAD24M/TLK_PAD_PLL_DIV/TLK_PAD_PLL)
    uint8_t cclk_div = sys_clk_config->source_clk / sys_clk_config->cclk;
    uint8_t source_reg_value;
    if (sys_clk_config->source == TLK_CLOCK_PLL)
    {
        if (cclk_div != 1)
        {
            tlk_clock_reg.clock_sel0_bit.sclk_div = cclk_div;
            source_reg_value                      = 2;
        }
        else
        {
            source_reg_value = 3;
        }
    }
    else if (sys_clk_config->source == TLK_CLOCK_24M_XTAL)
    {
        source_reg_value = 1;
    }
    else
    {
        source_reg_value = 0;
    }

    tlk_clock_reg.clock_sel0_bit.sclk_sel = source_reg_value;

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
    union tlk_areg_bbpll_ctrl1 ctrl = TLK_ANALOG_READ(TLK_AREG_BBPLL_CTRL1);
    ctrl.bit.bbpll_lck_det_rstb     = 1;
    TLK_ANALOG_WRITE(TLK_AREG_BBPLL_CTRL1, ctrl);

    for (volatile uint8_t i = 0; i < 3; i++)
    {
        uint8_t pll_done_cnt = 0;
        // This delay time is about 20.38us under the calibrated 24M RC clock.
        tlk_clock_cclk_delay(tlk_sys_clk.cclk * 20);

        for (volatile uint8_t j = 0; j < 3; j++)
        {
            if (TLK_ANALOG_READ(TLK_AREG_XO_CAL_CTRL).bit.doubler_cal_done)
            {
                pll_done_cnt++;
            }
        }

        if (pll_done_cnt == 3)
        {
            ctrl.bit.bbpll_lck_det_rstb = 0;
            TLK_ANALOG_WRITE(TLK_AREG_BBPLL_CTRL1, ctrl);

            return TLK_CLOCK_OK;
        }
        else
        {
            if (i == 0)
            {
                // The high temperature test found that setting the bbpll ldo in the default voltage
                // gear will cause the chip to crash. After debug, it was found that when the bbpll
                // ldo is switched, the clock will not be locked under certain voltages. Therefore,
                // what is done here Detect whether the bbpll clock is locked during processing, and
                // if not, switch to a voltage gear. (add by weihua.zhang, confirmed by wenfeng.lou
                // 20220720)
                union tlk_areg_power0 reg = {
                    .bit = {.bbpll_ldo_trim = 0b110, .ana_ldo_trim = 0b100}};
                TLK_ANALOG_WRITE(TLK_AREG_POWER0, reg);
            }
            else if (i == 1)
            {
                union tlk_areg_power0 reg = {
                    .bit = {.bbpll_ldo_trim = 0b011, .ana_ldo_trim = 0b100}};
                TLK_ANALOG_WRITE(TLK_AREG_POWER0, reg);
            }
            else
            {
                // The reason why the PLL is stable does not require a reboot:
                // If there is a problem with the PLL, even if it is restarted, the PLL will not be
                // good. This is different from the crystal oscillator. After rebooting, the crystal
                // oscillator still has a chance to oscillate. (add by weihua.zhang, confirmed by
                // wenfeng.lou 20220720)
                ctrl.bit.bbpll_lck_det_rstb = 0;
                TLK_ANALOG_WRITE(TLK_AREG_BBPLL_CTRL1, ctrl);
            }
        }
    }

    return TLK_CLOCK_ERROR;
}

_tlk_attribute_ram_code_sec_ bool tlk_clock_24m_rc_acquire(void)
{
    if (tlk_clock.clk_24m_rc_use_count++ == 0)
    {
        TLK_ANALOG_MODIFY(TLK_AREG_POWER_DOWN0, (value.bit.pd_24m_rc = 0;));

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

    TLK_ANALOG_MODIFY(TLK_AREG_CLK_CTRL0, (value.bit.rc_32k_cap_sel = 1;));

    union tlk_areg_cal_ctrl0 cal_ctrl = {.raw = 0xf6}; // cal_32k_en and cal_32k_test disabled
    TLK_ANALOG_WRITE(TLK_AREG_CAL_CTRL0, cal_ctrl);
    cal_ctrl.bit.cal_32k_en = 1;
    TLK_ANALOG_WRITE(TLK_AREG_CAL_CTRL0, cal_ctrl);

    TLK_WAIT_FOR_TRUE_OR_TIMEOUT((TLK_ANALOG_READ(TLK_AREG_CAL_CTRL3).bit.cal_32k_done),
                                 (TLK_CLOCK_32K_RC_STABLE_TIME_US),
                                 (status = TLK_CLOCK_ERROR;))

    tlk_analog_write_reg8(TLK_AREG_CLK_RC_32K_RES_H, tlk_analog_read_reg8(TLK_AREG_CAL_RC_32K_CAP));
    TLK_ANALOG_MODIFY(TLK_AREG_CLK_CTRL0,
        (value.bit.rc_32k_cap = TLK_ANALOG_READ(TLK_AREG_CAL_CTRL2).bit.cal_rc_32k_res;));

    cal_ctrl.bit.cal_32k_en = 0;
    TLK_ANALOG_WRITE(TLK_AREG_CAL_CTRL0, cal_ctrl);

    TLK_ANALOG_MODIFY(TLK_AREG_CLK_CTRL0, (value.bit.rc_32k_cap_sel = 0;));

    return status;
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_status tlk_clock_24m_rc_calibrate(void)
{
    enum tlk_clock_status status = TLK_CLOCK_OK;

    tlk_clock_24m_rc_acquire();

    tlk_analog_write_reg8(TLK_AREG_CAL_24M_WAIT_LEN, 0x80);

    TLK_ANALOG_MODIFY(TLK_AREG_CLK_CTRL0, (value.bit.rc_24m_cap_sel = 1;));

    union tlk_areg_cal_ctrl1 cal_ctrl = {.raw = 0x0e}; // cal_24m_en disabled
    TLK_ANALOG_WRITE(TLK_AREG_CAL_CTRL1, cal_ctrl);
    cal_ctrl.bit.cal_24m_en = 1;
    TLK_ANALOG_WRITE(TLK_AREG_CAL_CTRL1, cal_ctrl);

    TLK_WAIT_FOR_TRUE_OR_TIMEOUT((TLK_ANALOG_READ(TLK_AREG_CAL_CTRL3).bit.cal_24m_done),
                                 (TLK_CLOCK_24M_RC_STABLE_TIME_US),
                                 (status = TLK_CLOCK_ERROR;))

    tlk_analog_write_reg8(TLK_AREG_CLK_RC_24M_CAP, tlk_analog_read_reg8(TLK_AREG_CAL_RC_24M_CAP));

    TLK_ANALOG_MODIFY(TLK_AREG_CLK_CTRL0, (value.bit.rc_24m_cap_sel = 0;));

    cal_ctrl.bit.cal_24m_en = 0;
    TLK_ANALOG_WRITE(TLK_AREG_CAL_CTRL1, cal_ctrl);

    tlk_clock_24m_rc_release();

    return status;
}

#include "core/include/tlk_clock.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_stimer.h"
#include "core/include/tlk_sys.h"
#include "tlk_core.h"
#include "registers/tlk_analog_afe1v_reg.h"
#include "registers/tlk_analog_afe3v_reg.h"
#include "registers/tlk_clock.h"

#define TLK_CLOCK_24M_XTAL_STABLE_LOOP_COUNT 10
#define TLK_CLOCK_24M_XTAL_STABLE_TIME_US 40
#define TLK_CLOCK_32K_RC_STABLE_TIME_US 10000 
#define TLK_CLOCK_24M_RC_STABLE_TIME_US 5000

struct tlk_clock_config 
{
    enum tlk_clock_32k_source clk_32k_source;
    uint8_t clk_24m_rc_use_count;
};

struct tlk_clock_sys_clk_config tlk_default_sys_clk = {
    .source = TLK_CLOCK_24M_RC,
    .source_clk = 24,
    .cclk = 24,
    .hclk = 24, 
    .pclk = 24, 
    .mspi_clk = 24,
};

struct tlk_clock_sys_clk_config tlk_sys_clk = {
    .source = TLK_CLOCK_NONE,
};

struct tlk_clock_config tlk_clock = {
    
};

_tlk_attribute_ram_code_sec_ uint8_t tlk_clock_get_pll_reg_value(uint16_t pll_clk) 
{
    switch (pll_clk) {
    case 96:
        return 0x00;
    case 192:
        return 0x40;
    default:
        return 0;
    }
}

_tlk_attribute_ram_code_sec_ void tlk_clock_32k_configure(enum tlk_clock_32k_source source)
{
    uint8_t sel_32k = tlk_analog_read_reg8(areg_aon_0x4e & (~FLD_CLK32K_SEL));
    uint8_t power_32k = tlk_analog_read_reg8(areg_aon_0x05) & (~(FLD_32K_RC_PD | FLD_32K_XTAL_PD));
    tlk_analog_write_reg8(areg_aon_0x4e, sel_32k | (source << 7));
    if (source) {
        tlk_analog_write_reg8(areg_aon_0x05, power_32k | FLD_32K_RC_PD);   //32k xtal
    } else {
        tlk_analog_write_reg8(areg_aon_0x05, power_32k | FLD_32K_XTAL_PD); //32k rc
    }

    tlk_clock.clk_32k_source = source;
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_32k_source tlk_clock_32k_get_source(void)
{
    return tlk_clock.clk_32k_source;
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_status tlk_clock_sys_clk_configure(const struct tlk_clock_sys_clk_config *sys_clk_config)
{
    tlk_clock_24m_rc_acquire();

    if (tlk_sys_clk.source == TLK_CLOCK_24M_RC) {
        tlk_clock_24m_rc_release();
    }

    tlk_sys_clk = *sys_clk_config;

    if (sys_clk_config->source == TLK_CLOCK_PLL) {
        uint8_t pll_reg_value = tlk_clock_get_pll_reg_value(sys_clk_config->source_clk);

        /* power on bbpll supply switch and select pll clk out */
        tlk_analog_write_reg8(0x81, (tlk_analog_read_reg8(0x81) & (~FLD_POWER_ON_BBPLL_SUPPLY_SWITCH)) | (pll_reg_value & 0x40));

        tlk_analog_write_reg8(areg_aon_0x06, tlk_analog_read_reg8(areg_aon_0x06) & (~FLD_PD_BBPLL_LDO)); //power up pll

        enum tlk_clock_status status = tlk_clock_pll_is_ready();
        if (status != TLK_CLOCK_OK) {
            tlk_sys_reboot(TLK_SYS_SW_REBOOT_PLL_UNSTABLE);
        }
    }

    //change to 24M rc first.
    tlk_clock_reg.cclk_set = (tlk_clock_reg.cclk_set & 0xc0) | (TLK_CLOCK_24M_RC << 4) | 1;

    //HCLK and PCLK should be set ahead of CCLK, ensure the HCLK and PCLK not exceed the max CCLK(CCLK max 120M, HCLK max 60M, PCLK max 60M)
    uint8_t hclk_div = sys_clk_config->cclk / sys_clk_config->hclk;
    uint8_t pclk_div = sys_clk_config->hclk / sys_clk_config->pclk;
    tlk_clock_reg.busclk_ratio = (tlk_clock_reg.busclk_ratio & 0xf8) | ((hclk_div >> 1) << 2) | (pclk_div >> 1);

    //Configure the CCLK clock frequency.
    uint8_t cclk_div = sys_clk_config->source_clk / sys_clk_config->cclk;
    tlk_clock_reg.cclk_set = (tlk_clock_reg.cclk_set & 0xc0) | (sys_clk_config->source << 4) | cclk_div; //clock source. 0:rc 24m, 1:xtl_24m, 2:pll
  
    // mspi configuration
    uint8_t mspi_div = sys_clk_config->source_clk / sys_clk_config->mspi_clk;
    tlk_clock_reg.mspi_mode = (tlk_clock_reg.mspi_mode & 0xc0) | (sys_clk_config->source << 4) | mspi_div; //src:bit[5:4], div:bit[3:0]

    if (sys_clk_config->source != TLK_CLOCK_24M_RC) {
        tlk_clock_24m_rc_release();
    }

    return TLK_CLOCK_OK;
}

_tlk_attribute_ram_code_sec_ uint64_t tlk_clock_cclk_get_tick(void)
{
#if __riscv_xlen == 32
    do {
        uint32_t hi = tlk_read_csr(NDS_MCYCLEH);
        uint32_t lo = tlk_read_csr(NDS_MCYCLE);

        if (hi == tlk_read_csr(NDS_MCYCLEH)) {
            return ((uint64_t)hi << 32) | lo;
        }
    } while (1);
#else
    return tlk_read_csr(NDS_MCYCLE);
#endif
}

_tlk_attribute_ram_code_sec_ void tlk_clock_cclk_delay(uint64_t delay_tick)
{
    uint64_t start = tlk_clock_cclk_get_tick();
    while (tlk_clock_cclk_get_tick() - start < delay_tick) { }
}

_tlk_attribute_ram_code_sec_ void tlk_clock_24m_xtal_enable(void)
{
    tlk_analog_write_reg8(0x65, tlk_analog_read_reg8(0x65) | 0x40); //0x65<6>: write 1 to reset xtal quick start cnt

    uint8_t ana_05 = tlk_analog_read_reg8(0x05); //0x05<3>: 24M_xtl_pd
    tlk_analog_write_reg8(0x05, ana_05 | 0x08); //<3>1b'1: Power down 24MHz XTL oscillator

    tlk_analog_write_reg8(0x05, ana_05 & 0xf7); //<3>1b'0: Power up 24MHz XTL oscillator
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_status tlk_clock_24m_xtal_is_ready(void)
{
    // To judge the stability of the crystal oscillator, xo_ready_ana is invalid, 
    // so an alternative solution to judge is used: Because the clock source of the stimer is the crystal oscillator,
    // if the crystal oscillator does not vibrate, the tick value of the stimer does not increase 
    // or increases very slowly (when there is interference). 
    // So first use 24M RC to run the program and wait for a fixed time, 
    // calculate the number of ticks that the stimer should increase during this time,
    // and then read the tick value actually increased by the stimer.
    // When it reaches 50% of the calculated value, it proves that the crystal oscillator has started.
    enum tlk_clock_status status = TLK_CLOCK_ERROR;

    tlk_stimer_enable();

    for (volatile uint8_t i = 0; i < TLK_CLOCK_24M_XTAL_STABLE_LOOP_COUNT; i++) {
        volatile uint32_t tick = tlk_stimer_get_tick();
        // Note: Ensure that the current clock is 24M RC, otherwise the nop running time 
        // will not be TLK_CLOCK_24M_XTAL_STABLE_TIME_US, resulting in a later calculation error.
        // (add by weihua.zhang, confirmed by peng.sun 20230609)
        tlk_clock_cclk_delay(tlk_sys_clk.cclk * TLK_CLOCK_24M_XTAL_STABLE_TIME_US);
        
        if (tlk_stimer_get_tick() - tick > TLK_SYSTEM_TIMER_TICK_1US * TLK_CLOCK_24M_XTAL_STABLE_TIME_US / 2) {
            status = TLK_CLOCK_OK;
            break;
        }
    }

    tlk_stimer_disable();

    return status;
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_status tlk_clock_pll_is_ready(void)
{
    tlk_analog_write_reg8(areg_0x81, tlk_analog_read_reg8(areg_0x81) | FLD_ENABLE_LOCK_DET_BBPLL);

    uint8_t pll_ok = 0;
    uint64_t start = tlk_clock_cclk_get_tick();
    /*
    * The standard for judging the stability of PLL is that the ready flag bit read three times in a row is 1.
    * (add by jilong.liu, confirmed by wenfeng.lou 20241203)
    */
    while (pll_ok < 3) {
        if (tlk_analog_read_reg8(areg_0x88) & FLD_BBPLL_LOCK_DETECTOR) {
            pll_ok++;
        } else {
            pll_ok = 0;
        }

        if (tlk_clock_cclk_get_tick() - start > 1000 * tlk_sys_clk.cclk) {
            return TLK_CLOCK_ERROR;
        }
    }

    tlk_analog_write_reg8(areg_0x81, tlk_analog_read_reg8(areg_0x81) & (~FLD_ENABLE_LOCK_DET_BBPLL));

    return TLK_CLOCK_OK;
}

_tlk_attribute_ram_code_sec_ bool tlk_clock_24m_rc_acquire(void)
{
    if (tlk_clock.clk_24m_rc_use_count++ == 0) {
        tlk_analog_write_reg8(areg_aon_0x05, tlk_analog_read_reg8(areg_aon_0x05) & ~(FLD_24M_RC_PD));

        // the calibration of 24m RC should wait for 1us if just power it up.
        // (added by jilong.liu, confirmed by yangya at 20240805)
        tlk_clock_cclk_delay((unsigned long long)(2 * tlk_sys_clk.cclk));

        return false;
    }

    return true;
}

_tlk_attribute_ram_code_sec_ bool tlk_clock_24m_rc_release(void)
{
    if (--tlk_clock.clk_24m_rc_use_count == 0) {
        tlk_analog_write_reg8(areg_aon_0x05, tlk_analog_read_reg8(areg_aon_0x05) | FLD_24M_RC_PD);

        return false;
    }

    return true;
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_status tlk_clock_32k_rc_calibrate(void)
{
    enum tlk_clock_status status = TLK_CLOCK_OK;

    tlk_analog_write_reg8(areg_aon_0x4f, tlk_analog_read_reg8(areg_aon_0x4f) | FLD_RC_32K_CAP_SEL);

    tlk_analog_write_reg8(areg_0xc6, FLD_CAL_32K_RC_DISABLE);
    tlk_analog_write_reg8(areg_0xc6, FLD_CAL_32K_RC_ENABLE);

    TLK_WAIT_FOR_TRUE_OR_TIMEOUT(
        (tlk_analog_read_reg8(areg_0xcf) & FLD_CAL_32K_DONE), 
        (TLK_CLOCK_32K_RC_STABLE_TIME_US), 
        (status = TLK_CLOCK_ERROR;)
    )

    tlk_analog_write_reg8(areg_aon_0x51, tlk_analog_read_reg8(areg_0xc9)); //write 32k res[13:6] into manual register
    tlk_analog_write_reg8(areg_aon_0x4f, 
        (tlk_analog_read_reg8(areg_aon_0x4f) & 0xc0) | tlk_analog_read_reg8(areg_0xca)); //write 32k res[5:0] into manual register

    tlk_analog_write_reg8(areg_0xc6, FLD_CAL_32K_RC_DISABLE);
    tlk_analog_write_reg8(areg_aon_0x4f, tlk_analog_read_reg8(areg_aon_0x4f) & (~FLD_RC_32K_CAP_SEL)); //manual on

    return status;
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_status tlk_clock_24m_rc_calibrate(void)
{
    enum tlk_clock_status status = TLK_CLOCK_OK;

    tlk_clock_24m_rc_acquire();

    tlk_analog_write_reg8(areg_0xc8, 0x80); //wait 24m rc stable cycles

    tlk_analog_write_reg8(areg_aon_0x4f, tlk_analog_read_reg8(areg_aon_0x4f) | FLD_RC_24M_CAP_SEL);

    tlk_analog_write_reg8(areg_0xc7, FLD_CAL_24M_RC_DISABLE);
    tlk_analog_write_reg8(areg_0xc7, FLD_CAL_24M_RC_ENABLE);

    TLK_WAIT_FOR_TRUE_OR_TIMEOUT(
        (tlk_analog_read_reg8(areg_0xcf) & FLD_CAL_24M_DONE), 
        (TLK_CLOCK_32K_RC_STABLE_TIME_US), 
        (status = TLK_CLOCK_ERROR;)
    )

    tlk_analog_write_reg8(areg_aon_0x52, tlk_analog_read_reg8(areg_0xcb)); //write 24m cap into manual register

    tlk_analog_write_reg8(areg_aon_0x4f, tlk_analog_read_reg8(areg_aon_0x4f) & (~FLD_RC_24M_CAP_SEL));

    tlk_analog_write_reg8(areg_0xc7, FLD_CAL_24M_RC_DISABLE);

    tlk_clock_24m_rc_release();

    return status;
}

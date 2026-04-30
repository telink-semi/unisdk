#include "core/include/tlk_clock.h"
#include "core/include/tlk_analog.h"
#include "core/include/tlk_stimer.h"
#include "tlk_core.h"
#include "tlk_mspi.h"
#include "common/include/tlk_init.h"
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
    // something requires rc24m to be enabled for the correct pm, so for now, it is always enabled
    .clk_24m_rc_use_count = 1, 
};

_tlk_attribute_ram_code_sec_ void tlk_clock_32k_configure(enum tlk_clock_32k_source source)
{
    uint8_t sel_32k   = tlk_analog_read_reg8(0x4e) & 0x7f;
    uint8_t power_32k = tlk_analog_read_reg8(0x05) & 0xfc;
    tlk_analog_write_reg8(0x4e, sel_32k | (source << 7));
    if (source) {
        tlk_analog_write_reg8(0x05, power_32k | 0x1); //32k xtal
    } else {
        tlk_analog_write_reg8(0x05, power_32k | 0x2); //32k rc
    }

    tlk_clock.clk_32k_source = source;
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_32k_source tlk_clock_32k_get_source(void)
{
    return tlk_clock.clk_32k_source;
}

_tlk_attribute_ram_code_sec_ uint8_t tlk_clock_get_pll_reg_value(uint16_t pll_clk)
{
    switch (pll_clk) {
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

_tlk_attribute_ram_code_sec_ enum tlk_clock_status tlk_clock_sys_clk_configure(const struct tlk_clock_sys_clk_config *sys_clk_config)
{
    tlk_clock_24m_rc_acquire();

    if (tlk_sys_clk.source == TLK_CLOCK_24M_RC) {
        tlk_clock_24m_rc_release();
    }

    tlk_sys_clk = *sys_clk_config;

    // ensure mspi is not in busy status before change mspi clock
    tlk_mspi_stop_xip();

    // first cclk/mspi_clk switch to 24rc to avoid the risk of hclk/pclk/mspi_clk 
    // exceeding its maximum configurable frequency for a short period of time
    // when switching different clock frequencies using this interface.
    tlk_clock_reg.clock_sel0 &= 0x8f;                              // cclk to 24M rc clock
    write_reg8(0x1401c0, read_reg8(0x1401c0) & (~TLK_BIT(6))); // mspi_clk to 24M rc clock

    if (sys_clk_config->source == TLK_CLOCK_PLL) {
        uint8_t pll_reg_value = tlk_clock_get_pll_reg_value(sys_clk_config->source_clk);
        tlk_analog_write_reg8(0x80, (tlk_analog_read_reg8(0x80) & 0xe0) | ((pll_reg_value >> 2) & 0x1f));
        tlk_analog_write_reg8(0x09, (tlk_analog_read_reg8(0x09) & 0xf3) | ((pll_reg_value & 0x03) << 2));

        // wait for PLL stable
        tlk_analog_write_reg8(0x81, (tlk_analog_read_reg8(0x81) | TLK_BIT(6)));
        while (!(tlk_analog_read_reg8(0x88) & TLK_BIT(5))) { }
        tlk_analog_write_reg8(0x81, (tlk_analog_read_reg8(0x81) & ~TLK_BIT(6)));
    }

    // usb clock (192M/4 =48M) pll clock should be the multiple of 48, because USB clock is 48M.
    write_reg8(0x1401fb, sys_clk_config->source_clk / 48);

    uint8_t mspi_clk_div = sys_clk_config->source_clk / sys_clk_config->mspi_clk;
    if (mspi_clk_div == 1) {
        write_reg8(0x1401c0, read_reg8(0x1401c0) & 0xbf); //bit6 0
    } else {
        write_reg8(0x1401c0, (read_reg8(0x1401c0) & 0xc0) | mspi_clk_div); 
        //if the div is odd, should set two times to ensure the correct sequence.
        write_reg8(0x1401c0, read_reg8(0x1401c0) | TLK_BIT(6));
        write_reg8(0x1401c0, read_reg8(0x1401c0) | TLK_BIT(6));
    }
    tlk_mspi_set_xip_en();

    // hclk and pclk should be set ahead of cclk, ensure the hclk and pclk not exceed the max clk(cclk max 96M, hclk max 48M, pclk max 48M)
    uint8_t hclk_div = sys_clk_config->cclk / sys_clk_config->hclk;
    if (hclk_div == 1) {
        write_reg8(0x1401d8, read_reg8(0x1401d8) & ~TLK_BIT(2));
    } else {
        write_reg8(0x1401d8, read_reg8(0x1401d8) | TLK_BIT(2));
    }

    // pclk can div1/div2/div4 from hclk.
    uint8_t pclk_div = sys_clk_config->hclk / sys_clk_config->pclk;
    if (pclk_div == 1) {
        write_reg8(0x1401d8, read_reg8(0x1401d8) & 0xfc);
    } else {
        write_reg8(0x1401d8, (read_reg8(0x1401d8) & 0xfc) | (pclk_div / 2));
    }

    // select cclk source(TLK_RC24M/TLK_PAD24M/TLK_PAD_PLL_DIV/TLK_PAD_PLL)
    uint8_t cclk_div = sys_clk_config->source_clk / sys_clk_config->cclk;
    uint8_t source_reg_value;
    if (sys_clk_config->source == TLK_CLOCK_PLL) {
        if (cclk_div != 1) {
            tlk_clock_reg.clock_sel0 = (tlk_clock_reg.clock_sel0 & 0xf0) | cclk_div;
            source_reg_value = 2;
        } else {
            source_reg_value = 3;
        }
    } else if (sys_clk_config->source == TLK_CLOCK_24M_XTAL) {
        source_reg_value = 1;
    } else {
        source_reg_value = 0;
    }

    tlk_clock_reg.clock_sel0_bit.sclk_sel = source_reg_value;

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
    uint8_t ana_81 = tlk_analog_read_reg8(0x81);
    tlk_analog_write_reg8(0x81, ana_81 | TLK_BIT(6));
    
    for (volatile uint8_t i = 0; i < 3; i++) {
        uint8_t pll_done_cnt = 0;
        // This delay time is about 20.38us under the calibrated 24M RC clock.
        tlk_clock_cclk_delay(tlk_sys_clk.cclk * 20); 
        
        for (volatile uint8_t j = 0; j < 3; j++) {
            if (tlk_analog_read_reg8(0x88) & TLK_BIT(5)) {
                pll_done_cnt++;
            }
        }

        if (pll_done_cnt == 3) {
            tlk_analog_write_reg8(0x81, ana_81 & 0xbf);

            return TLK_CLOCK_OK;
        }
        else {
            if (i == 0) {
                // The high temperature test found that setting the bbpll ldo in the default voltage gear will cause the chip to crash.
                // After debug, it was found that when the bbpll ldo is switched, the clock will not be locked under certain voltages.
                // Therefore, what is done here Detect whether the bbpll clock is locked during processing, and if not, switch to a voltage gear.
                // (add by weihua.zhang, confirmed by wenfeng.lou 20220720)
                tlk_analog_write_reg8(0x01, 0x46);
            }
            else if (i == 1) {
                tlk_analog_write_reg8(0x01, 0x43);
            }
            else {
                // The reason why the PLL is stable does not require a reboot:
                // If there is a problem with the PLL, even if it is restarted, the PLL will not be good.
                // This is different from the crystal oscillator. After rebooting, the crystal oscillator still has a chance to oscillate.
                // (add by weihua.zhang, confirmed by wenfeng.lou 20220720)
                tlk_analog_write_reg8(0x81, ana_81 & 0xbf);
            }
        }
    }

    return TLK_CLOCK_ERROR;
}

_tlk_attribute_ram_code_sec_ bool tlk_clock_24m_rc_acquire(void)
{
    if (tlk_clock.clk_24m_rc_use_count++ == 0) {
        tlk_analog_write_reg8(0x05, tlk_analog_read_reg8(0x05) & (~TLK_BIT(2)));
        //<2>:24M_rc_pd,        default:0,->0: Power up 24MHz RC oscillator.

        return false;
    }

    return true;
}

_tlk_attribute_ram_code_sec_ bool tlk_clock_24m_rc_release(void)
{
    if (--tlk_clock.clk_24m_rc_use_count == 0) {
        tlk_analog_write_reg8(0x05, tlk_analog_read_reg8(0x05) | (TLK_BIT(2)));
        //<2>:24M_rc_pd,        default:0,->1: Power down 24MHz RC oscillator.

        return false;
    }

    return true;
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_status tlk_clock_32k_rc_calibrate(void)
{
    enum tlk_clock_status status = TLK_CLOCK_OK;

    tlk_analog_write_reg8(0x4f, ((tlk_analog_read_reg8(0x4f) & 0x3f) | 0x40));
    tlk_analog_write_reg8(0xc6, 0xf6);
    tlk_analog_write_reg8(0xc6, 0xf7);

    TLK_WAIT_FOR_TRUE_OR_TIMEOUT(
        (tlk_analog_read_reg8(0xcf) & TLK_BIT(6)), 
        (TLK_CLOCK_32K_RC_STABLE_TIME_US), 
        (status = TLK_CLOCK_ERROR;)
    )

    uint8_t res1 = tlk_analog_read_reg8(0xc9); //read 32k res[13:6]
    tlk_analog_write_reg8(0x51, res1); //write 32k res[13:6] into manual register
    uint8_t res2 = tlk_analog_read_reg8(0xca); //read 32k res[5:0]
    tlk_analog_write_reg8(0x4f, (res2 | (tlk_analog_read_reg8(0x4f) & 0xc0))); //write 32k res[5:0] into manual register
    
    tlk_analog_write_reg8(0xc6, 0xf6);
    tlk_analog_write_reg8(0x4f, ((tlk_analog_read_reg8(0x4f) & 0x3f) | 0x00)); //manual on

    return status;
}

_tlk_attribute_ram_code_sec_ enum tlk_clock_status tlk_clock_24m_rc_calibrate(void)
{
    enum tlk_clock_status status = TLK_CLOCK_OK;

    tlk_clock_24m_rc_acquire();

    tlk_analog_write_reg8(0xc8, 0x80);
    tlk_analog_write_reg8(0x4f, tlk_analog_read_reg8(0x4f) | TLK_BIT(7));

    tlk_analog_write_reg8(0xc7, 0x0e);
    tlk_analog_write_reg8(0xc7, 0x0f);
    
    TLK_WAIT_FOR_TRUE_OR_TIMEOUT(
        (tlk_analog_read_reg8(0xcf) & TLK_BIT(7)), 
        (TLK_CLOCK_24M_RC_STABLE_TIME_US), 
        (status = TLK_CLOCK_ERROR;)
    )

    uint8_t cap = tlk_analog_read_reg8(0xcb);
    tlk_analog_write_reg8(0x52, cap); //write 24m cap into manual register

    tlk_analog_write_reg8(0x4f, tlk_analog_read_reg8(0x4f) & (~TLK_BIT(7)));
    tlk_analog_write_reg8(0xc7, 0x0e);

    tlk_clock_24m_rc_release();

    return status;
}
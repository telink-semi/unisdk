#include "core/include/tlk_core.h"
#include "properties/tlk_plic.h"
#include "registers/tlk_plic.h"
#include <string.h>

/* ==== MACROS ==== */
/**
 * @brief   Set "DP_THROUGH_SWIRE_DIS" as "1" to disable the "dp_through_swire" function. Set
 * "DP_THROUGH_SWIRE_DIS" as "0" to enable the "dp_through_swire" function. Risk description: The
 * "dp_through_swire" function is enabled by default.When DP and DM pin are used as GPIO function
 * and trigger "dp_through_swire" timing, there may be a problem of miswriting the chip through
 * swire to cause the chip to crash. Therefore, disable the "dp_through_swire" function in this
 * startup file.(set "DP_THROUGH_SWIRE_DIS" as "1"). If the chip has only dp pin but no sws pin,
 * using the above scheme (set "DP_THROUGH_SWIRE_DIS" as "1") will cause the following problems.
 *          1. During fixture production, burning a firmware program that disables the
 * "dp_through_swire"function into unprogrammed chip will cause the problem that chip can not be
 * burned again due to swire communication failure.
 *          2. Burning a firmware program that disables the "dp_through_swire" function with BDT
 * tool may result in failure of debugging due to swire communication failure. If developers
 * evaluate that the above scheme (set "DP_THROUGH_SWIRE_DIS" as "1") cannot be used, they can only
 * set "DP_THROUGH_SWIRE_DIS" to "0" to enable the "dp_through_swire" function. However, be careful
 * not to make DP and DM pin trigger "dp_through_swire" timing. In USB applications, 1. calling the
 * usb_set_pin(1) interface (equivalent to usb_pin_set_en) will re-enable the dp_through_swire
 * function 2. calling the usb_set_pin(0) interface will disable the dp_through_swire function.
 */
#ifndef DP_THROUGH_SWIRE_DIS
#define DP_THROUGH_SWIRE_DIS 1
#endif

// According to the current internal seal flash model of Telink RISC-V MCU, the maximum time between
// the flash awakening and the operating flash is 20us, which leaves some margin and is set to 25us.
// When adding flash later, if tRES1 is greater than 25us, this waiting time needs to be updated.
// (flash tRES1, which can be viewed in the comments of the variable flash_support_mid.) (add by
// weihua.zhang 20230811)
#define EFUSE_LOAD_AND_FLASH_WAKEUP_LOOP_NUM 320 // 25us
/* ==== MACROS ==== */

/* ==== REGISTERS ==== */
/* If using Andes toolchain, these two Macro are defined in it's toolchain */
/* If using std gcc toolchain, these two core registers(not standard risc-v core registers)
    should be defined here. */
#ifndef mmisc_ctl
#define mmisc_ctl 0x7D0
#endif

#ifndef mcache_ctl
#define mcache_ctl 0x7CA
#endif

#define REG_PG_IE       0x80140331
#define REG_MSPI_CMD    0xA3FFFF04
#define REG_MSPI_CTRL1  0xA3FFFF08
#define REG_MSPI_CTRL4  0xA3FFFF1c
#define REG_MSPI_CFG    0xA3FFFF20
#define REG_MSPI_STATUS 0xA3FFFF28
/* ==== REGISTERS ==== */

/* ==== VARIABLES ==== */
extern uint32_t tlk_g_pm_mspi_cfg;
/* ==== VARIABLES ==== */

/* ==== EXTERN HELPERS ==== */
typedef void (*tlk_irq_handler)(void);
extern tlk_irq_handler __tlk_vectors[UNISDK_PLIC_IRQ_COUNT];
extern void            tlk_trap_entry(void);
/* ==== EXTERN HELPERS ==== */

/* ==== LINKER SYMBOLS ==== */
extern uint32_t _RETENTION_RESET_LMA_START[];
extern uint32_t _RETENTION_RESET_VMA_START[];
extern uint32_t _RETENTION_RESET_VMA_END[];

extern uint32_t _RAMCODE_LMA_START[];
extern uint32_t _RAMCODE_VMA_START[];
extern uint32_t _RAMCODE_VMA_END[];

extern uint32_t _DATA_LMA_START[];
extern uint32_t _DATA_VMA_START[];
extern uint32_t _DATA_VMA_END[];

extern uint32_t _BSS_VMA_START[];
extern uint32_t _BSS_VMA_END[];

extern uint32_t _AES_VMA_START[];
extern uint32_t _AES_VMA_END[];
/* ==== LINKER SYMBOLS ==== */

/**
 * @brief Core initialization entry point (called from assembly startup code).
 *
 * @param init_type Boot type selector passed from assembly:
 *                  - 0 — COLD BOOT (_START)
 *                  - 1 — WARM BOOT AFTER RETENTION SLEEP (_ISTART)
 */
_tlk_attribute_ram_code_sec_noinline_ void tlk_core_init(uint8_t init_type)
{
#if defined(CHIP_A3_VERSION_DCDC_1P4_LDO_2P0_EN)
    if (!init_type)
    {
        uint32_t wait_count;

        /* Set the power supply mode to DCDC_LDO. */
        *(volatile uint8_t*) 0x80140183 = 0x01; // reg_ana_len = 1;
        *(volatile uint8_t*) 0x80140180 = 0x0a; // reg_ana_addr = 0x0a;
        *(volatile uint8_t*) 0x80140184 = 0x91; // reg_ana_data(0) = 0x91;

        /* _ANA_CNT_WAIT */
        wait_count = 0;
        while (wait_count < 10000 && (*(volatile uint8_t*) 0x80140188 & 0xf0) != 0x10)
        {
            wait_count++;
        }

        if (wait_count >= 10000)
        {
            /* _ANA_TIMEROUT */
            *(volatile uint8_t*) 0x801401ef = 0x20; // reboot
        }
        else
        {
            /* _ANA_CYC_W */
            *(volatile uint8_t*) 0x80140182 = 0x60; // reg_ana_ctrl = (FLD_ANA_CYC | FLD_ANA_RW);

            /* _ANA_W_WAIT */
            wait_count = 0;
            while (wait_count < 10000 && (*(volatile uint8_t*) 0x80140182 & 0x80))
            {
                wait_count++;
            }

            if (wait_count >= 10000)
            {
                /* _ANA_TIMEROUT */
                *(volatile uint8_t*) 0x801401ef = 0x20; // reboot
            }
            else
            {
                /* _ANA_W_END */
                *(volatile uint8_t*) 0x80140182 = 0x00; // reg_ana_ctrl = 0x00;
            }
        }
    }
#endif

#if DP_THROUGH_SWIRE_DIS
    *(volatile uint8_t*) 0x80100C01 = 0x00; // 0x80100c01 -> 0x00 <7>: 0 swire_usb_dis
#endif

#ifdef __riscv_flen
    /* Enable FPU */
    tlk_set_csr(NDS_MSTATUS, 0x00006000);
    /* Initialize FCSR */
    __asm__ volatile("fscsr zero");
#endif

#if defined(CONFIG_TLK_PLIC_VECTOR_MODE)
    tlk_write_csr(NDS_MTVEC, __tlk_vectors); /* Initial machine trap-vector Base */
    tlk_set_csr(mmisc_ctl, 0x02);            /* Enable vectored external plic interrupt */
    tlk_plic_reg.feat_en =
        0x02; /* Vector mode enable bit (VECTORED) of the Feature Enable Register */
#else
    tlk_write_csr(NDS_MTVEC, tlk_trap_entry);
    tlk_clear_csr(mmisc_ctl, 0x02); /* Disable vectored external plic interrupt */
    tlk_plic_reg.feat_en =
        0x00; /* Vector mode disable bit (VECTORED) of the Feature Enable Register */
#endif

    /* Enable I/D-Cache */
    tlk_set_csr(mcache_ctl, 0x03);
    __asm__ volatile("fence.i" ::: "memory");

#if TLK_IS_ENABLED(CONFIG_TLK_PM_RAM_RETENTION_ENABLE)
    if (init_type)
    {
        /* flash wakeup */
        write_sram8(REG_MSPI_CTRL4, 0x0e); // xip_stop
        while (read_sram8(REG_MSPI_STATUS) == 0x80)
            ;                              // read reg_mspi_status FLD_MSPI_BUSY(0xA3FFFF28[bit7])
        write_sram8(REG_MSPI_CTRL4, 0x06); // xip_disable

        write_sram8(REG_PG_IE, 0x3F);      // mspi ie enable
        write_sram8(REG_MSPI_CTRL1, 0x80); // mspi_ctrl1
        write_sram8(REG_MSPI_CMD, 0xab);   // wakeup_cmd

        while (read_sram8(REG_MSPI_STATUS) == 0x80)
            ;                              // read reg_mspi_status FLD_MSPI_BUSY(0xA3FFFF28[bit7])
        write_sram8(REG_MSPI_CTRL4, 0x0a); // xip_enable

        /*flash wakeup need delay about 20us */
        /*efuse load need delay about 6us */
        for (volatile uint32_t i = 0; i <= EFUSE_LOAD_AND_FLASH_WAKEUP_LOOP_NUM; i++)
            ;

        write_sram32(REG_MSPI_CFG, tlk_g_pm_mspi_cfg); // tlk_g_pm_mspi_cfg->0xA3FFFF20
    }
#endif

    if (!init_type)
    {
        /* Move retention reset from flash to sram */
        memcpy(_RETENTION_RESET_VMA_START,
               _RETENTION_RESET_LMA_START,
               (uint8_t*) _RETENTION_RESET_VMA_END - (uint8_t*) _RETENTION_RESET_VMA_START);

        /* Zero .aes section in sram */
        memset(_AES_VMA_START, 0, (uint8_t*) _AES_VMA_END - (uint8_t*) _AES_VMA_START);

        /* Move Data from flash to sram */
        memcpy(_DATA_VMA_START,
               _DATA_LMA_START,
               (uint8_t*) _DATA_VMA_END - (uint8_t*) _DATA_VMA_START);

        /* Zero .bss section in sram */
        memset(_BSS_VMA_START, 0, (uint8_t*) _BSS_VMA_END - (uint8_t*) _BSS_VMA_START);
    }
}

_tlk_attribute_text_sec_noinline_ void tlk_core_init_flash(void)
{
    /* Move ramcode from flash to sram */
    memcpy(_RAMCODE_VMA_START,
           _RAMCODE_LMA_START,
           (uint8_t*) _RAMCODE_VMA_END - (uint8_t*) _RAMCODE_VMA_START);

    tlk_core_init(0);
}

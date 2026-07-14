/********************************************************************************************************
 * @file    cstartup_flash.S
 *
 * @brief   This is the boot file for UniSDK
 *
 * @author  Telink UTC
 * @date    2025
 *
 * @par     Copyright (c) 2024, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/

#include "properties/tlk_chip.h"
#include "properties/tlk_plic.h"

.extern tlk_core_init
.extern tlk_core_init_flash
.extern tlk_pre_init
.extern main

    .section .vectors, "ax"
    .org 0x0

    .global _RESET_ENTRY
    .type _RESET_ENTRY,@function

    .align 2
_RESET_ENTRY:
    j        _START
    //free the 6th ~ 7th byte to store the crc type of the bin file
    /* This configuration is specifically for RF_Certification and does not need to be enabled for other scenarios */
#ifdef RF_CERTIFICATION_CGF_EN
    .org 0x10
    .word(_RF_CERTIFICATION_CFG_ADDR_OFFSET)
#endif
    .org 0x18
    .word (BIN_SIZE)

    .org 0x20
    .word ('T'<<24 | 'L'<<16 | 'N'<<8 | 'K')

    .org 0x24
/* in the original configuration, set bit[22] to 1, enable timeout:
 * when xip reaches a certain time without fetching instructions, cs will not be pulled down, saving power consumption.
 */
    .word (0x3b4097a9)          //DREAD:   cmd:1x, addr:1x, data:2x, dummy cycle:8

    .align 2

/* ==== BOOT (_START) ==== */
_START:
    /* timer rst enable. */
    li      t0, UNISDK_CHIP_REG_RST2  /* reg_rst2 register address. */
    li      t1, 0x39                  /* bit0=1 timer enable(reset velue 0x38). */
    sb      t1, 0x0(t0)               /* (*(volatile unsigned char*)(UNISDK_CHIP_REG_RST2))= 0x39. */

    /* timer clk enable. */
    li      t0, UNISDK_CHIP_REG_TIM_CTRL2   /* tim_ctrl2 register address. */
    li      t1, 0x31                        /* bit0=1 timer enable(reset velue 0x30). */
    sb      t1, 0x0(t0)                     /* (*(volatile unsigned char*)(UNISDK_CHIP_REG_TIM_CTRL2))= 0x31. */


    /* enable analog */
    li      t0, UNISDK_CHIP_REG_RST
    lw      t2, 0x0(t0)
    li      t1, 0x800
    or      t1, t2, t1
    sw      t1, 0x0(t0)         /* reg_rst */

    li      t0, UNISDK_CHIP_REG_CLK_EN
    lw      t2, 0x0(t0)
    li      t1, 0x800
    or      t1, t2, t1
    sw      t1, 0x0(t0)         /* reg_clk_en */

    call _WDT_ERRATA

    /* Initialize stack pointer */
    la     t0, _STACK_TOP
    mv     sp, t0


#if TLK_IS_ENABLED(CONFIG_TLK_STACK_STATISTIC)
    la      t1, _Stack_Size          /* Load stack size from linker */
    sub     t2, t0, t1               /* t2 = stack bottom (TOP - SIZE) */
    li      t3, 0x55555555           /* Mask to fill */

_STACK_FILL_LOOP:
    sw      t3, 0(t2)                /* Store mask to current address */
    addi    t2, t2, 4                /* Move to next word */
    bltu    t2, t0, _STACK_FILL_LOOP /* Repeat until TOP reached */
#endif


    /* This code should be placed before the following tlk_gpio_shutdown_for_asm interface because
    when using the Os compilation option, the tlk_gpio_shutdown_for_asm will use the EXEC.IT table. */
#ifdef __nds_execit
    /* Initialize EXEC.IT table */
    la t0, _ITB_BASE_
    csrw uitb, t0

    /* Move exec.it from flash to sram */
_EXECIT_INIT:
    la     t1, _EXECIT_LMA_START
    la     t2, _EXECIT_VMA_START
    la     t3, _EXECIT_VMA_END
_EXECIT_INIT_BEGIN:
    bleu   t3, t2, _EXECIT_INIT_END
    lw     t0, 0(t1)
    sw     t0, 0(t2)
    addi   t1, t1, 4
    addi   t2, t2, 4
    j      _EXECIT_INIT_BEGIN
_EXECIT_INIT_END:
#endif

    /* Run the core_init, pre-init */
    call tlk_core_init_flash
    call tlk_pre_init

    /* Jump to the main function */
_MAIN_FUNC:
#if TLK_IS_ENABLED (CONFIG_TLK_TASK_PLANNER)
    call user_setup
#else
    call main
#endif
#if TLK_IS_ENABLED (CONFIG_TLK_TASK_PLANNER_MODE_LOOP)
    call tlk_platform_loop
#elif TLK_IS_ENABLED (CONFIG_TLK_TASK_PLANNER_MODE_SCHEDULER)
    call task_planner_run
#endif
_END:
    j    _END
/* ==== BOOT (_START) ==== */


/* ==== BOOT AFTER RETENTION SLEEP (_ISTART) ==== */
#if TLK_IS_ENABLED(CONFIG_TLK_PM_RAM_RETENTION_ENABLE)

    .section .retention_reset, "ax"
    .global _IRESET_ENTRY
    .type _IRESET_ENTRY,@function

    .align 2
_IRESET_ENTRY:
    /* Decide whether this is an NMI or cold reset */
    j       _ISTART

    .org 0x22
_ISTART:
    /* timer rst enable. */
    li      t0, UNISDK_CHIP_REG_RST2  /* reg_rst2 register address. */
    li      t1, 0x39                  /* bit0=1 timer enable(reset velue 0x38). */
    sb      t1, 0x0(t0)               /* (*(volatile unsigned char*)(UNISDK_CHIP_REG_RST2))= 0x39. */

    /* timer clk enable. */
    li      t0, UNISDK_CHIP_REG_TIM_CTRL2  /* tim_ctrl2 register address. */
    li      t1, 0x31                       /* bit0=1 timer enable(reset velue 0x30). */
    sb      t1, 0x0(t0)                    /* (*(volatile unsigned char*)(UNISDK_CHIP_REG_TIM_CTRL2))= 0x31. */


    /* Restore stack pointer */
    lui    x31, %hi(tlk_context_backup_registers)
    addi   x31, x31, %lo(tlk_context_backup_registers)

    lw     x2, (1 * 4)(x31)


    /* This code should be placed before the following tlk_gpio_shutdown_ramcode_for_asm interface because
    when using the Os compilation option, the tlk_gpio_shutdown_ramcode_for_asm will use the EXEC.IT table. */
#ifdef __nds_execit
    /* Initialize EXEC.IT table */
    la t0, _ITB_BASE_
    csrw uitb, t0
#endif

    li  a0, 1
    call tlk_core_init

    /* Continue execution from the tlk_context_save call */
    call tlk_context_restore

_IEND:
    j    _IEND

#endif
/* ==== BOOT AFTER RETENTION SLEEP (_ISTART) ==== */


.text
    .global default_irq_entry
    .align 2

default_irq_entry:
1:  j 1b

       .weak trap_handler

trap_handler:
1:  j 1b

    .macro INTERRUPT num
    .weak entry_irq\num
    .set entry_irq\num, default_irq_entry
    .long entry_irq\num
    .endm

    .section .ram_code, "ax"

    .global __tlk_vectors
    .balign 256


__tlk_vectors:
    /* Trap vector */
    .long tlk_trap_entry

    /* PLIC interrupt vector */
    .altmacro
    .set irqno, 1
    .rept UNISDK_PLIC_IRQ_COUNT /* .rept .endr */
    INTERRUPT %irqno
    .set irqno, irqno+1
    .endr

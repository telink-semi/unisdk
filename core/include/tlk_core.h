/********************************************************************************************************
 * @file    tlk_core.h
 *
 * @brief   This is the header file for TL721X
 *
 * @author  Driver Group
 * @date    2024
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
#ifndef TLK_CORE_H
#define TLK_CORE_H

#include "registers/tlk_core_reg.h"

#define TLK_DISABLE_BTB       __asm__("csrci %0,8" ::"i"(mmisc_ctl))
#define TLK_ENABLE_BTB        __asm__("csrsi %0,8" ::"i"(mmisc_ctl))
#define TLK_DISABLE_VECTORING __asm__("csrci %0,2" ::"i"(mmisc_ctl))
#define TLK_ENABLE_VECTORING  __asm__("csrsi %0,2" ::"i"(mmisc_ctl))

/**
 * @brief Machine mode MHSP_CTL
 */
typedef enum
{
    TLK_MHSP_CTL_OVF_EN =
        TLK_BIT(0), /**< Enable bit for the stack overflow protection and recording mechanism. */
    TLK_MHSP_CTL_UDF_EN =
        TLK_BIT(1), /**< Enable bit for the stack underflow protection mechanism. */
    TLK_MHSP_CTL_SCHM_SEL = TLK_BIT(2), /**< 0-Stack overflow/underflow,1-Top-of-stack recording. */
    TLK_MHSP_CTL_U_EN =
        TLK_BIT(3), /**< Enables the SP protection and recording mechanism in User mode. */
    TLK_MHSP_CTL_S_EN =
        TLK_BIT(4), /**< Enables the SP protection and recording mechanism in Supervisor mode. */
    TLK_MHSP_CTL_M_EN =
        TLK_BIT(5), /**< Enables the SP protection and recording mechanism in Machine mode. */
} tlk_mhsp_ctl_e;

typedef enum
{
    TLK_FLD_MSTATUS_MIE  = TLK_BIT(3),               // M-mode interrupt enable bit
    TLK_FLD_MSTATUS_MPIE = TLK_BIT(7),               // M-mode previous interrupt enable bit
    TLK_FLD_MSTATUS_MPP  = TLK_BIT(11) | TLK_BIT(12) // M-mode previous privilege bit
} tlk_mstatus_e;

typedef enum
{
    TLK_FLD_MIE_MSIE = TLK_BIT(3),  // M-mode software interrupt enable bit.
    TLK_FLD_MIE_MTIE = TLK_BIT(7),  // M-mode timer interrupt enable bit
    TLK_FLD_MIE_MEIE = TLK_BIT(11), // M-mode external interrupt enable bit
} tlk_mie_e;

#define tlk_read_csr(reg)                                                                          \
    ({                                                                                             \
        unsigned long __tmp;                                                                       \
        __asm__ volatile("csrr %0, %1" : "=r"(__tmp) : "i"(reg));                                  \
        __tmp;                                                                                     \
    })

#define tlk_write_csr(reg, val) ({ __asm__ volatile("csrw %0, %1" ::"i"(reg), "rK"(val)); })

#define tlk_swap_csr(reg, val)                                                                     \
    ({                                                                                             \
        unsigned long __tmp;                                                                       \
        __asm__ volatile("csrrw %0, %1, %2" : "=r"(__tmp) : "i"(reg), "rK"(val));                  \
        __tmp;                                                                                     \
    })

#define tlk_set_csr(reg, bit)                                                                      \
    ({                                                                                             \
        unsigned long __tmp;                                                                       \
        __asm__ volatile("csrrs %0, %1, %2" : "=r"(__tmp) : "i"(reg), "rK"(bit));                  \
        __tmp;                                                                                     \
    })

#define tlk_clear_csr(reg, bit)                                                                    \
    ({                                                                                             \
        unsigned long __tmp;                                                                       \
        __asm__ volatile("csrrc %0, %1, %2" : "=r"(__tmp) : "i"(reg), "rK"(bit));                  \
        __tmp;                                                                                     \
    })

#define tlk_fence_iorw __asm__ volatile("fence" : : : "memory")

#define tlk_core_get_current_sp()                                                                  \
    ({                                                                                             \
        unsigned long __tmp;                                                                       \
        __asm__ volatile("mv %0, sp" : "=r"(__tmp));                                               \
        __tmp;                                                                                     \
    })

/*
 * Inline nested interrupt entry/exit macros
 */
/* Save/Restore macro */
#define tlk_save_csr(r)    long __##r = tlk_read_csr(r);
#define tlk_restore_csr(r) tlk_write_csr(r, __##r);
/* Support PowerBrake (Performance Throttling) feature */

/* Nested IRQ entry macro : Save CSRs and enable global interrupt. */
#define tlk_core_save_nested_context()                                                             \
    tlk_save_csr(NDS_MEPC) tlk_save_csr(NDS_MIE) tlk_save_csr(NDS_MSTATUS)                         \
        tlk_save_csr(NDS_MXSTATUS)

/* Nested IRQ exit macro : Restore CSRs */
#define tlk_core_restore_nested_context()                                                          \
    tlk_restore_csr(NDS_MIE) tlk_restore_csr(NDS_MSTATUS) tlk_restore_csr(NDS_MEPC)                \
        tlk_restore_csr(NDS_MXSTATUS)

typedef enum
{
    TLK_FLD_FEATURE_PREEMPT_PRIORITY_INT_EN = TLK_BIT(0),
    TLK_FLD_FEATURE_VECTOR_MODE_EN          = TLK_BIT(1),
} tlk_feature_e;

/**
 * @brief Disable interrupts globally in the system.
 * @return  r - the MIE bit value of machine status register(MSTATUS).
 * @note  this function must be used when the system wants to disable all the interrupt.
 * @return     none
 */
_tlk_always_inline_ static unsigned int tlk_core_interrupt_disable(void)
{
    unsigned int r = tlk_read_csr(NDS_MSTATUS) & TLK_FLD_MSTATUS_MIE;
    if (r)
    {
        tlk_clear_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE); // global interrupts disable
        tlk_fence_iorw; /* Hardware may change this value, fence IO ensures that software changes
                           are valid. */
    }
    return r;
}

/**
 * @brief restore interrupts globally in the system.
 * @param[in]  en - the MIE bit value of machine status register(MSTATUS).
 * this  parameter must be the return value of the tlk_core_interrupt_disable function.
 * @return     0
 * @note this function must be used when the system wants to restore all the interrupt.
 */
_tlk_always_inline_ static unsigned int tlk_core_interrupt_restore(unsigned int en)
{
    if (en)
    {
        tlk_set_csr(NDS_MSTATUS, en); // global interrupts enable
        tlk_fence_iorw; /* Hardware may change this value, fence IO ensures that software changes
                           are valid. */
    }
    return 0;
}

/**
 * @brief This function serves to enable MEI(Machine External Interrupt),MTI(Machine timer
 * Interrupt),or MSI(Machine Software Interrupt).
 * @param[in] mie_mask - MIE(Machine Interrupt Enable) register mask.
 * @return  none
 */
_tlk_always_inline_ static void tlk_core_mie_enable(tlk_mie_e mie_mask)
{
    tlk_set_csr(NDS_MIE, mie_mask);
}

/**
 * @brief      This function serves to disable MEI(Machine External Interrupt),MTI(Machine timer
 * Interrupt),or MSI(Machine Software Interrupt).
 * @param[in]  mie_mask - MIE(Machine Interrupt Enable) register mask.
 * @return     mie register value before disable.
 * @note       tlk_core_mie_disable and tlk_core_mie_restore must be used in pairs.
 */
_tlk_always_inline_ static unsigned int tlk_core_mie_disable(tlk_mie_e mie_mask)
{
    unsigned int r = tlk_read_csr(NDS_MIE);
    tlk_clear_csr(NDS_MIE, mie_mask);
    return r;
}

/**
 * @brief      This function serves to restore MIE register value.
 * @param[in]  mie_value - mie register value returned by tlk_core_mie_disable().
 * @return     none.
 * @note       tlk_core_mie_disable and tlk_core_mie_restore must be used in pairs.
 */
_tlk_always_inline_ static void tlk_core_mie_restore(unsigned int mie_value)
{
    tlk_write_csr(NDS_MIE, mie_value);
}

/**
 * @brief This function serves to enable interrupts globally in the system, MEI(machine external
 * interrupt) will also enable.
 * @return  none
 */
_tlk_always_inline_ static void tlk_core_interrupt_enable(void)
{
    tlk_set_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE); // global interrupts enable
    tlk_fence_iorw; /* Hardware may change this value, fence IO ensures that software changes are
                       valid. */
    tlk_core_mie_enable(TLK_FLD_MIE_MEIE); // external interrupt enable selectively
}

/**
 * @brief This function serves to set mhsp control registers.
 * @param[in]  ctl - the value of mhsp_ctl, refer to tlk_mhsp_ctl_e for the definition of MHSP_CTL
 * @return     none
 */
_tlk_always_inline_ static void tlk_core_mhsp_set_ctr(tlk_mhsp_ctl_e ctl)
{
    tlk_write_csr(NDS_MHSP_CTL, (unsigned int) ctl);
}

/**
 * @brief This function serves to set hsp bound registers.
 *  ---Stack overflow---
 When the SP overflow detection mechanism is properly selected and enabled, any updated value to
 the SP register (via any instruction) is compared with the msp_bound register If the updated value
 to the SP register is smaller than the msp_bound register, a stack overflow exception is generated
 a stack overflow exception is generated.
 The stack overflow exception has an exception code of 32 in the mcause register
 ---Top-of-stack recording---
 When the top of stack recording mechanism is properly selected and enabled, any updated value to
 the SP register on any instruction is compared with the msp_bound register. If the updated value
 to the SP register is smaller than the msp_bound register, the msp_bound register is updated with
 this updated value
 * @param[in]  bound - the value of  hsp_bound .
 * @return     none
 */
_tlk_always_inline_ static void tlk_core_msp_set_bound(unsigned int bound)
{
    tlk_write_csr(NDS_MSP_BOUND, bound);
}

/**
 * @brief This function serves to set hsp base registers.
 When the SP underflow detection mechanism is properly selected and enabled, any updated value to
 the SP register (via any instruction) is compared with the msp_base register. If the updated value
 to the SP register is greater than the msp_base register, a stack underflow exception is generated.
 The stack underflow exception has an exception code of 33 in the mcause register.
 * @param[in]  base - the value of hsp_base .
 * @return     none
 */
_tlk_always_inline_ static void tlk_core_msp_set_base(unsigned int base)
{
    tlk_write_csr(NDS_MSP_BASE, base);
}

/**
 * @brief This function serves to get hsp bound registers.
 * ---Top-of-stack recording---
 When the top of stack recording mechanism is properly selected and enabled, any updated value to
 the SP register on any instruction is compared with the msp_bound register. If the updated value
 to the SP register is smaller than the msp_bound register, the msp_bound register is updated with
 this updated value,you can read this value.
 * @return     none
 */
_tlk_always_inline_ static unsigned int tlk_core_msp_get_bound(void)
{
    return tlk_read_csr(NDS_MSP_BOUND);
}

/**
 * @brief This function serves to get hsp base registers.there is no recording.
 * @return     none
 */
_tlk_always_inline_ static unsigned int tlk_core_msp_get_base(void)
{
    return tlk_read_csr(NDS_MSP_BASE);
}

/**
 * @brief This function serves to get mcause(Machine Cause) value.
 * This register indicates the cause of trap, reset, NMI or the interrupt source ID of a vector
 interrupt. This register is updated when a trap, reset, NMI or vector interrupt occurs
 * @return     none
 */
_tlk_always_inline_ static unsigned int tlk_core_mcause_get(void)
{
    return tlk_read_csr(NDS_MCAUSE);
}

/**
 * @brief This function serves to get mepc(Machine Exception Program Counter) value.
 * When entering an exception, the hardware will automatically update the value of the mepc register
 * to the value of the instruction pc currently encountered with the exception
 * @return     none
 */
_tlk_always_inline_ static unsigned int tlk_core_mepc_get(void)
{
    return tlk_read_csr(NDS_MEPC);
}

/**
 * @brief    This function serves to enable mcu entry WFI(Wait-For-Interrupt) mode similar to stall
 * mcu.
 * @return   none
 * @note:    there are two awoke modes by interrupt:
 *             - When global interrupts are enabled using the interface tlk_core_interrupt_enable()
 * (mstatus.MIE is enabled)
 *               - Before entering WFI, make sure the following conditions are met:
 *                 -# Enable MEI, MSI or MTI using interface tlk_core_mie_enable(), so that can wake
 * up mcu after core enters WFI.
 *                 -# If there is an interrupt that has already been triggered, the corresponding
 * interrupt flag bit has been cleared.
 *                 -# Interrupt enable for wakeup source using interface
 * tlk_plic_interrupt_enable(), interrupt disable for non-wakeup sources using interface
 * tlk_plic_interrupt_disable().
 *               - After exiting WFI, the processor is awoken by a taken interrupt, it will resume
 * and start to execute from the corresponding interrupt service routine, the processing steps in
 * ISR as follows:
 *                  -# Clear the corresponding interrupt flag bit in the corresponding interrupt
 * service routine.
 *                 -# Your application code.
 *             - When global interrupts are disabled using the interface
 * tlk_core_interrupt_disable() (mstatus.MIE is disabled)
 *               - Before entering WFI, make sure the following conditions are met:
 *                 -# Enable MEI, MSI or MTI using interface tlk_core_mie_enable(), so that can wake
 * up mcu after core enters WFI.
 *                 -# If there is an interrupt that has already been triggered, the corresponding
 * interrupt flag bit has been cleared.
 *                 -# Interrupt enable for wakeup source using interface
 * tlk_plic_interrupt_enable(), interrupt disable for non-wakeup sources using interface
 * tlk_plic_interrupt_disable().
 *                 -# Clear all current requests from the PLIC using the tlk_plic_clr_all_request()
 * interface.
 *               - After exiting WFI, the processor is awoken by a pending interrupt, it will resume
 * and start to execute from the instruction after the WFI instruction, the processing steps after
 * WFI instruction as follows:
 *                 -# Getting the wakeup source using interface tlk_plic_interrupt_claim().
 *                 -# Take stimer for example, using the interfaces tlk_stimer_irq_get_status() and
 * tlk_stimer_irq_clr_status() to get and clear the corresponding interrupt flag bit according to
 * the interrupt source.
 *                 -# Your application code.
 *                 -# Using interface tlk_plic_interrupt_complete() to notify PLIC that the
 * corresponding interrupt processing is complete.
 */
_tlk_always_inline_ static void tlk_core_enter_wfi(void)
{
    /* Interrupts disabled by the mie CSR will not be able to wake up the processor.
      However,the processor can be awoken by these interrupts regardless the value of the global
      interrupt enable bit (mstatus.MIE)*/
    __asm__ __volatile__("wfi");
}

/**
 * @brief     This function serves to get current pc.
 * @return    current pc
 */
_tlk_always_inline_ static unsigned int tlk_core_get_current_pc(void)
{
    unsigned int current_pc = 0;
    __asm__("auipc %0, 0" : "=r"(current_pc)::"a0");
    return current_pc;
}

/**
 * @brief Switches the core to User mode (U-mode).
 *
 * Clears MPP to 0 (U-mode) so that mret transitions privilege level.
 * MPIE is set to match the previous MIE state so that interrupts are
 * correctly restored after mret.
 *
 * @param entry_fn  Function to execute in U-mode.
 *                  If NULL, mepc is set to the next instruction and mret
 *                  switches privilege inline — execution continues in U-mode
 *                  at the call site.
 *                  If not NULL, mepc is set to entry_fn and mret jumps
 *                  there; execution resumes in U-mode after entry_fn returns.
 *
 * @param return_fn Optional function called in U-mode after entry_fn returns.
 *                  Ignored if entry_fn is NULL.
 */
_tlk_always_inline_ static void tlk_core_enable_umode(void (*entry_fn)(void),
                                                      void (*return_fn)(void))
{
    unsigned int prev_mie = tlk_core_interrupt_disable();
    tlk_clear_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MPP | TLK_FLD_MSTATUS_MPIE);

    if (prev_mie)
    {
        tlk_set_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MPIE);
    }

    if (!entry_fn)
    {
        __asm__ volatile("la   t0, 1f  \n\t"
                         "csrw mepc, t0\n\t"
                         "mret         \n"
                         "1:" ::
                             : "t0");

        return;
    }

    tlk_write_csr(NDS_MEPC, (unsigned int) entry_fn);

    __asm__ volatile("la  ra, 1f \n\t"
                     "mret       \n"
                     "1:" ::
                         : "ra");

    if (return_fn)
    {
        return_fn();
    }
}

/**
 * @brief Configures mret to return to Machine mode.
 *
 * Sets MPP = M-mode (0b11) in mstatus so that the next mret
 * transitions to M-mode instead of returning to the previous
 * privilege level.
 *
 * Must be called from M-mode before mret fires — typically
 * inside an ecall handler when the caller wants to remain in
 * M-mode after the trap returns.
 */
_tlk_always_inline_ static void tlk_core_enable_mmode(void)
{
    tlk_set_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MPP);
}
#endif

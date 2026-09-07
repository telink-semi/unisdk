#include "core/include/tlk_trap.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_sys.h"
#include "properties/tlk_trap.h"
#include "system/debug/log/tlk_log.h"

#if TLK_IS_ENABLED(CONFIG_TLK_PLIC)
#include "core/include/tlk_plic.h"
#endif

TLK_LOG_CREATE(trap_log, "TRAP");

/**
 * @brief The value of the mtval register when the exception is entered.
 */
static volatile long tlk_exception_mtval;

/**
 * @brief The value of the mepc register when the exception is entered.
 */
static volatile long tlk_exception_mepc;

/**
 * @brief The value of the mstatus register when the exception is entered.
 */
static volatile long tlk_exception_mstatus;

/**
 * @brief The value of the mcause register when the exception is entered.
 */
static volatile long tlk_exception_mcause;

/**
 * @brief The value of the mdcause register when the exception is entered.
 */
static volatile long tlk_exception_mdcause;

/**
 * @brief Pointer to the callback function in the case of ecall
 */
void (*tlk_trap_ecall_callback)(void) = NULL;

/**
 * @brief      Register a callback to be invoked when an ecall exception occurs.
 * @param[in]  cb - pointer to the callback function. Pass NULL to clear.
 * @return     None.
 */
void tlk_trap_register_ecall_callback(void (*cb)(void))
{
    tlk_trap_ecall_callback = cb;
}

/**
 * @brief     Default exception irq handler.
 * @return    None.
 */
__attribute__((weak)) _tlk_attribute_ram_code_sec_ void tlk_exception_handle(void)
{
    /* unhandled trap */
    tlk_exception_mtval   = tlk_read_csr(NDS_MTVAL);
    tlk_exception_mepc    = tlk_read_csr(NDS_MEPC);
    tlk_exception_mstatus = tlk_read_csr(NDS_MSTATUS);
    tlk_exception_mcause  = tlk_read_csr(NDS_MCAUSE);
    tlk_exception_mdcause = tlk_read_csr(NDS_MDCAUSE);

    TLK_LOG_ERROR(trap_log, "\n\nUnhandled exception:\n\n");
    TLK_LOG_ERROR(trap_log, "mtval  : %8x\n", tlk_exception_mtval);
    TLK_LOG_ERROR(trap_log, "mepc   : %8x\n", tlk_exception_mepc);
    TLK_LOG_ERROR(trap_log, "mstatus: %8x\n", tlk_exception_mstatus);
    TLK_LOG_ERROR(trap_log, "mcause : %8x\n", tlk_exception_mcause);
    TLK_LOG_ERROR(trap_log, "mdcause: %8x\n", tlk_exception_mdcause);

    while (1)
    {
        _TLK_ASM_NOP_;
    }
}

/**
 * @brief       This function service to handle all the platform pre-defined interrupt or exception.
 * @return      None.
 */
__attribute_trap_entry_ void tlk_trap_entry(void)
{
    uint32_t mcause = tlk_read_csr(NDS_MCAUSE);

    if (TLK_BM_IS_SET(mcause, UNISDK_TRAP_MCAUSE_INTERRUPT_MASK))
    {
        switch (mcause & UNISDK_TRAP_MCAUSE_EXCEPTION_CODE_MASK)
        {
#if TLK_IS_ENABLED(CONFIG_TLK_MTIMER_IRQ_ENABLE)
        case UNISDK_TRAP_MCAUSE_EXCEPTION_CODE_MTI:
        {
            extern void tlk_mtimer_irq_handle(void);

            tlk_mtimer_irq_handle();
            break;
        }
#endif
#if TLK_IS_ENABLED(CONFIG_TLK_PLIC_SW)
        case UNISDK_TRAP_MCAUSE_EXCEPTION_CODE_MSI:
        {
            extern void tlk_plic_sw_handle(void);

            tlk_plic_sw_handle();
            break;
        }
#endif
#if TLK_IS_ENABLED(CONFIG_TLK_PLIC) && TLK_IS_DISABLED(CONFIG_TLK_PLIC_VECTOR_MODE)
        case UNISDK_TRAP_MCAUSE_EXCEPTION_CODE_MEI:
        {
            extern void tlk_plic_handle(void);

            tlk_plic_handle();
            break;
        }
#endif
        default:
        {
            tlk_exception_handle();
            break;
        }
        }
    }
    else
    {
        switch (mcause & UNISDK_TRAP_MCAUSE_EXCEPTION_CODE_MASK)
        {
        case UNISDK_TRAP_MCAUSE_EXCEPTION_CODE_ECALL:
        {
            if (tlk_trap_ecall_callback)
            {
#ifdef CONFIG_TLK_TRAP_ECALL_NESTING
                /* save CSRs to stack */
                tlk_core_save_nested_context();
                /* enable global interupt */
                tlk_set_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE);
#endif
                /* call the handler */
                tlk_trap_ecall_callback();
#ifdef CONFIG_TLK_TRAP_ECALL_NESTING
                /* save MPP bit (u_mode / m_mode) */
                uint32_t mpp = tlk_read_csr(NDS_MSTATUS) & TLK_FLD_MSTATUS_MPP;
                /* disable global interrupts before critical section */
                tlk_clear_csr(NDS_MSTATUS, TLK_FLD_MSTATUS_MIE);
                /* restore CSRs */
                tlk_core_restore_nested_context();
                /* set saved MPP bit (u_mode / m_mode) */
                tlk_set_csr(NDS_MSTATUS, mpp);
#endif
            }
            else
            {
                tlk_exception_handle();
            }

            tlk_write_csr(NDS_MEPC, tlk_read_csr(NDS_MEPC) + 4);
            break;
        }
        default:
        {
            // In case of exception
            tlk_exception_handle();
            break;
        }
        }
    }
}

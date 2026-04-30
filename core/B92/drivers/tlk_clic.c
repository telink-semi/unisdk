#include "core/include/tlk_clic.h"
#include "core/include/tlk_sys.h"
#include "system/debug/log/tlk_log.h"
#include "properties/tlk_clic.h"

#if IS_ENABLED(CONFIG_TLK_PLIC)
    #include "core/include/tlk_plic.h"
#endif

TLK_LOG_CREATE(clic_log, "CLIC");

void tlk_clic_enable_interrupt_source(tlk_mie_e source)
{
    tlk_set_csr(NDS_MIE, source); 
}

void tlk_clic_disable_interrupt_source(tlk_mie_e source)
{
    tlk_clear_csr(NDS_MIE, source); 
}

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
 * @brief     Default exception irq handler.
 * @return    None.
 */
__attribute__((weak)) _tlk_attribute_ram_code_sec_ void tlk_exception_handler(void)
{
    /* unhandled trap */
    tlk_exception_mtval   = tlk_read_csr(NDS_MTVAL);
    tlk_exception_mepc    = tlk_read_csr(NDS_MEPC);
    tlk_exception_mstatus = tlk_read_csr(NDS_MSTATUS);
    tlk_exception_mcause  = tlk_read_csr(NDS_MCAUSE);
    tlk_exception_mdcause = tlk_read_csr(NDS_MDCAUSE);

    TLK_LOG_ERROR(clic_log, "\n\nUnhandled exception:\n\n");
    TLK_LOG_ERROR(clic_log, "mtval  : %8x\n", tlk_exception_mtval);
    TLK_LOG_ERROR(clic_log, "mepc   : %8x\n", tlk_exception_mepc);
    TLK_LOG_ERROR(clic_log, "mstatus: %8x\n", tlk_exception_mstatus);
    TLK_LOG_ERROR(clic_log, "mcause : %8x\n", tlk_exception_mcause);
    TLK_LOG_ERROR(clic_log, "mdcause: %8x\n", tlk_exception_mdcause);

    while (1) {
        _TLK_ASM_NOP_;
    }
}

/**
 * @brief       This function service to handle all the platform pre-defined interrupt or exception.
 * @return      None.
 */
__attribute_trap_entry_ void tlk_trap_entry(void)
{
    long mcause   = tlk_read_csr(NDS_MCAUSE);

    if (TLK_BM_IS_SET(mcause, UNISDK_CLIC_MCAUSE_INTERRUPT_MASK)) {
        switch (TLK_BM_IS_SET(mcause, UNISDK_CLIC_MCAUSE_EXCEPTION_CODE_MASK)) {
#if IS_ENABLED(CONFIG_TLK_MTIMER_ENABLE_IRQ)
            case UNISDK_CLIC_MCAUSE_EXCEPTION_CODE_MTI:
            {
                extern void tlk_mtimer_handler(void);

                tlk_mtimer_handler();
                break;
            }
#endif
#if IS_ENABLED(CONFIG_TLK_PLIC_SW)
            case UNISDK_CLIC_MCAUSE_EXCEPTION_CODE_MSI: 
            {
                extern void tlk_plic_sw_handler(void);

                tlk_plic_sw_handler();
                break;
            }
#endif
#if IS_ENABLED(CONFIG_TLK_PLIC) && IS_DISABLED(CONFIG_TLK_PLIC_VECTOR_MODE)
            case UNISDK_CLIC_MCAUSE_EXCEPTION_CODE_MEI: 
            {
                extern void tlk_plic_handler(void);
                tlk_plic_handler();
                break;
            }
#endif
            default:
            {
                tlk_exception_handler();
                break;
            }
        }

    } else {  /* In case of exception */
        tlk_exception_handler();
    }
}

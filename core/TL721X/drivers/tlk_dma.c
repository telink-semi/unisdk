#include "core/include/tlk_dma.h"
#include "common/include/tlk_init.h"
#include "core/include/tlk_plic.h"
#include "properties/tlk_plic.h"
#include "registers/tlk_dma.h"
#include "system/debug/assert/tlk_assert.h"

/* ==== VARIABLES ==== */
#ifdef CONFIG_TLK_DMA_IRQ_HANDLER
tlk_dma_chn_handler_t tlk_dma_chn_handlers[UNISDK_DMA_CHN_COUNT];
#endif

#ifdef CONFIG_TLK_DMA_PM_DEVICE
struct tlk_dma_config tlk_dma_retention_configs[UNISDK_DMA_CHN_COUNT];

struct tlk_dma_ll_node* tlk_dma_retention_lls[UNISDK_DMA_CHN_COUNT];
#endif
/* ==== VARIABLES ==== */

/* ==== HELPERS ==== */
static uint32_t tlk_dma_cal_transfer_size(uint32_t                    size_byte,
                                          enum tlk_dma_transfer_width byte_width)
{
    byte_width = 1 << byte_width;
    return (((size_byte + byte_width - 1) / byte_width) | ((size_byte % byte_width) << 22));
}

#ifdef CONFIG_TLK_DMA_IRQ_HANDLER
static void tlk_dma_irq_enable_all(enum tlk_dma_chn chn)
{
    tlk_dma_reg.chn[chn].control_bit.tc_irq_en = 1;     // Enable TC interrupt
    TLK_BM_SET(tlk_dma_reg.err_irq_en, TLK_BIT(chn));   // Enable ERROR interrupt
    TLK_BM_SET(tlk_dma_reg.abort_irq_en, TLK_BIT(chn)); // Enable ABORT interrupt
}

static void tlk_dma_irq_disable_all(enum tlk_dma_chn chn)
{
    tlk_dma_reg.chn[chn].control_bit.tc_irq_en = 0;     // Disable TC interrupt
    TLK_BM_CLR(tlk_dma_reg.err_irq_en, TLK_BIT(chn));   // Disable ERROR interrupt
    TLK_BM_CLR(tlk_dma_reg.abort_irq_en, TLK_BIT(chn)); // Disable ABORT interrupt
}

static uint8_t tlk_dma_irq_get_active(enum tlk_dma_chn chn)
{
    uint8_t active_irqs = 0;

    if (TLK_BM_IS_SET(tlk_dma_reg.irq_status_bit.tc_irq, TLK_BIT(chn)))
    {
        active_irqs |= TLK_BIT(TLK_DMA_IRQ_TC);
    }

    if (TLK_BM_IS_SET(tlk_dma_reg.irq_status_bit.error_irq, TLK_BIT(chn)))
    {
        active_irqs |= TLK_BIT(TLK_DMA_IRQ_ERR);
    }

    if (TLK_BM_IS_SET(tlk_dma_reg.irq_status_bit.abort_irq, TLK_BIT(chn)))
    {
        active_irqs |= TLK_BIT(TLK_DMA_IRQ_ABT);
    }

    return active_irqs;
}

static void tlk_dma_irq_clear(enum tlk_dma_chn chn, uint8_t mask)
{
    if (TLK_BM_IS_SET(mask, TLK_BIT(TLK_DMA_IRQ_TC)))
    {
        tlk_dma_reg.irq_status_bit.tc_irq = TLK_BIT(chn);
    }

    if (TLK_BM_IS_SET(mask, TLK_BIT(TLK_DMA_IRQ_ERR)))
    {
        tlk_dma_reg.irq_status_bit.error_irq = TLK_BIT(chn);
    }

    if (TLK_BM_IS_SET(mask, TLK_BIT(TLK_DMA_IRQ_ABT)))
    {
        tlk_dma_reg.irq_status_bit.abort_irq = TLK_BIT(chn);
    }
}
#endif
/* ==== HELPERS ==== */

/* ==== DRIVER APIs ==== */
void tlk_dma_chn_configure(enum tlk_dma_chn chn, struct tlk_dma_config* config)
{
    tlk_dma_reg.chn[chn].control =
        (tlk_dma_reg.chn[chn].control & (~TLK_BIT_RNG(2, 31))) | ((*(uint32_t*) config) << 2);

#ifdef CONFIG_TLK_DMA_PM_DEVICE
    tlk_dma_retention_configs[chn] = *config;
#endif
}

void tlk_dma_chn_transfer_configure(enum tlk_dma_chn chn, uint32_t src_addr, uint32_t dst_addr,
                                    uint32_t size)
{
    tlk_dma_reg.chn[chn].control_bit.chn_en = 0;
    tlk_dma_reg.chn[chn].src_addr           = (uint32_t) (src_addr);
    tlk_dma_reg.chn[chn].dst_addr           = (uint32_t) (dst_addr);
    tlk_dma_reg.chn[chn].size =
        tlk_dma_cal_transfer_size(size, tlk_dma_reg.chn[chn].control_bit.src_width);
}

void tlk_dma_chn_transfer_configure_ll(enum tlk_dma_chn chn, struct tlk_dma_ll_node* head)
{
    tlk_dma_reg.chn[chn].control_bit.chn_en = 0;
    tlk_dma_reg.chn[chn].ll_pointer         = (uint32_t) (head == NULL ? NULL : head->next);

    TLK_IF_ENABLED(CONFIG_TLK_DMA_PM_DEVICE, (tlk_dma_retention_lls[chn] = head;))

    if (head != NULL)
    {
        tlk_dma_reg.chn[chn].src_addr = (uint32_t) head->src_addr;
        tlk_dma_reg.chn[chn].dst_addr = (uint32_t) head->dst_addr;
        tlk_dma_reg.chn[chn].size =
            tlk_dma_cal_transfer_size(head->length, tlk_dma_reg.chn[chn].control_bit.src_width);
    }

    for (struct tlk_dma_ll_node* current = head; current != NULL; current = current->next)
    {
        current->control = tlk_dma_reg.chn[chn].control | TLK_BIT(0);
        current->length =
            tlk_dma_cal_transfer_size(current->length, tlk_dma_reg.chn[chn].control_bit.src_width);

        if (current->next == head)
        {
            break;
        }
    }
}

void tlk_dma_chn_transfer_start(enum tlk_dma_chn chn)
{
    tlk_dma_reg.chn[chn].control_bit.chn_en = 1;
}

_tlk_attribute_ram_code_sec_ void tlk_dma_chn_transfer_abort(enum tlk_dma_chn chn)
{
    tlk_dma_reg.abort |= TLK_BIT(chn);
    while (!(tlk_dma_reg.irq_status_bit.abort_irq & TLK_BIT(chn)) &&
           tlk_dma_reg.chn[chn].control_bit.chn_en)
    {
    }

#ifdef CONFIG_TLK_DMA_IRQ_HANDLER
    if (tlk_dma_chn_handlers[chn] == NULL)
#endif
    {
        tlk_dma_reg.irq_status_bit.abort_irq = TLK_BIT(chn);
    }
}

#ifdef CONFIG_TLK_DMA_IRQ_HANDLER
void tlk_dma_chn_add_callback(enum tlk_dma_chn chn, tlk_dma_chn_handler_t handler)
{
    tlk_dma_chn_handlers[chn] = handler;

    if (handler != NULL)
    {
        tlk_dma_irq_enable_all(chn);
    }
    else
    {
        tlk_dma_irq_disable_all(chn);
    }
}

void tlk_dma_chn_ll_set_irq_mode(enum tlk_dma_chn chn, enum tlk_dma_ll_irq_mode mode)
{
    uint8_t byte_offset = chn > TLK_DMA3;
    uint8_t bit_offset  = (chn % 4) * 2;
    tlk_dma_reg.ll_int_mode[byte_offset] =
        (tlk_dma_reg.ll_int_mode[byte_offset] & (~TLK_BIT_RNG(bit_offset, bit_offset + 1))) |
        (mode << bit_offset);
}
#endif
/* ==== DRIVER APIs ==== */

/* ==== IRQ HELPERS ==== */
#ifdef CONFIG_TLK_DMA_IRQ_HANDLER
_tlk_attribute_ram_code_sec_ static void tlk_dma_irq_init(void)
{
    for (uint8_t i = 0; i < UNISDK_DMA_CHN_COUNT; i++)
    {
        tlk_dma_irq_disable_all(i);
    }

    tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_DMA);
}
TLK_REGISTER_PRE_INIT(tlk_dma_irq_init, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)

_tlk_attribute_ram_code_sec_ static void tlk_dma_irq_handle(void)
{
    for (uint8_t i = 0; i < UNISDK_DMA_CHN_COUNT; i++)
    {
        uint8_t active_irqs = tlk_dma_irq_get_active(i);
        tlk_dma_irq_clear(i, active_irqs);

        if (!active_irqs)
        {
            continue;
        }

        if (tlk_dma_chn_handlers[i] != NULL)
        {
            if (active_irqs & TLK_BIT(TLK_DMA_IRQ_TC))
            {
                tlk_dma_chn_handlers[i](TLK_DMA_IRQ_TC);
            }

            if (active_irqs & TLK_BIT(TLK_DMA_IRQ_ERR))
            {
                tlk_dma_chn_handlers[i](TLK_DMA_IRQ_ERR);
            }

            if (active_irqs & TLK_BIT(TLK_DMA_IRQ_ABT))
            {
                tlk_dma_chn_handlers[i](TLK_DMA_IRQ_ABT);
            }
        }
    }
}
TLK_PLIC_ISR_REGISTER(tlk_dma_irq_handle, UNISDK_PLIC_IRQ_NUM_DMA)
#endif
/* ==== IRQ HELPERS ==== */

/* ==== PM HELPERS ==== */
#ifdef CONFIG_TLK_DMA_PM_DEVICE
static void tlk_dma_restore_context(void)
{
    for (uint8_t i = 0; i < UNISDK_DMA_CHN_COUNT; i++)
    {
        tlk_dma_chn_configure(i, &tlk_dma_retention_configs[i]);

        if (tlk_dma_retention_lls[i] != NULL)
        {
            tlk_dma_chn_transfer_configure_ll(i, tlk_dma_retention_lls[i]);
        }

#ifdef CONFIG_TLK_DMA_IRQ_HANDLER
        tlk_dma_chn_add_callback(i, tlk_dma_chn_handlers[i]);
    }

    tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_DMA);
#else
    }
#endif
}
TLK_REGISTER_AFTER_SUSPEND(tlk_dma_restore_context, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)
TLK_REGISTER_AFTER_SLEEP(tlk_dma_restore_context, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)
#endif
/* ==== PM HELPERS ==== */

/* ==== PREVENT SLEEP ==== */
#ifdef CONFIG_TLK_DMA_PREVENT_SLEEP
static bool tlk_dma_prevent_sleep(void)
{
    for (uint8_t i = 0; i < UNISDK_DMA_CHN_COUNT; i++)
    {
        if (tlk_dma_reg.chn[i].control_bit.chn_en)
        {
            return 1;
        }
    }
    return 0;
}

TLK_REGISTER_PREVENT_SUSPEND(tlk_dma_prevent_sleep)
TLK_REGISTER_PREVENT_SLEEP(tlk_dma_prevent_sleep)
#endif
/* ==== PREVENT SLEEP ==== */

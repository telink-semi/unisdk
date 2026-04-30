#include "core/include/tlk_dma.h"
#include "core/include/tlk_plic.h"
#include "common/include/tlk_init.h"
#include "registers/tlk_dma.h"
#include "properties/tlk_plic.h"


/* ==== VARIABLES ==== */
#ifdef CONFIG_TLK_DMA_IRQ_HANDLER
tlk_dma_chn_handler_t tlk_dma_chn_handlers[TLK_DMA_CHN_MAX];
#endif

#ifdef CONFIG_TLK_DMA_PM_DEVICE
struct tlk_dma_config tlk_dma_retention_configs[TLK_DMA_CHN_MAX];
#endif
/* ==== VARIABLES ==== */


/* ==== CHANNELS MANAGEMENT ==== */
#define _TLK_CONFIG_WITH_DMA(name) CONFIG_TLK_##name##_DMA
#define TLK_CONFIG_WITH_DMA(name) _TLK_CONFIG_WITH_DMA(name)

#define static_assert(cond, msg) typedef char static_assertion_##__LINE__[(cond) ? -1 : 1]


#define TLK_DMA_FUNCTION(num) UNISDK_DMA_FUNCTION_##num

#define TLK_DECLARE_DMA_FUNCTION(num) \
    IF_ENABLED( \
        TLK_CONFIG_WITH_DMA(TLK_DMA_FUNCTION(num)), \
        (TLK_DMA_FUNCTION_##num,) \
    )

enum tlk_dma_function_channels {
    FOR_CALL(UNISDK_DMA_FUNCTION_COUNT, TLK_DECLARE_DMA_FUNCTION)
    TLK_DMA_RESERVED_COUNT
};

static_assert(TLK_DMA_RESERVED_COUNT > UNISDK_DMA_CHN_COUNT, "Too many requests for DMA channels!");


#define TLK_DECLARE_DMA_CHANNEL_FOR_RESERVATION(num) uint8_t tlk_dma##num : 1;
#define TLK_DMA_RESERVATION(num) UNISDK_DMA_RESERVATIONS_##num

#define TLK_DECLARE_DMA_CHANNEL_WITH_RESERVATION(num) \
    .tlk_dma##num = IF_ENABLED_ELSE( \
        TLK_CONFIG_WITH_DMA(TLK_DMA_RESERVATION(num)), \
        (1), \
        (0) \
    ),

static struct
{
    FOR_CALL(UNISDK_DMA_CHN_COUNT, TLK_DECLARE_DMA_CHANNEL_FOR_RESERVATION)
} tlk_dma_channels = {
    FOR_CALL(UNISDK_DMA_CHN_COUNT, TLK_DECLARE_DMA_CHANNEL_WITH_RESERVATION)
};
/* ==== CHANNELS MANAGEMENT ==== */


/* ==== HELPERS ==== */
#ifdef CONFIG_TLK_DMA_IRQ_HANDLER
static inline void tlk_dma_irq_enable_all(enum tlk_dma_chn chn)
{
    tlk_dma_reg.chn[chn].control_bit.tc_irq_en = 1; // Enable TC interrupt
    TLK_BM_SET(tlk_dma_reg.err_irq_en, TLK_BIT(chn)); // Enable ERROR interrupt
    TLK_BM_SET(tlk_dma_reg.abort_irq_en, TLK_BIT(chn)); // Enable ABORT interrupt
}

static inline void tlk_dma_irq_disable_all(enum tlk_dma_chn chn)
{
    tlk_dma_reg.chn[chn].control_bit.tc_irq_en = 0; // Disable TC interrupt
    TLK_BM_CLR(tlk_dma_reg.err_irq_en, TLK_BIT(chn)); // Disable ERROR interrupt
    TLK_BM_CLR(tlk_dma_reg.abort_irq_en, TLK_BIT(chn)); // Disable ABORT interrupt
}

static inline uint8_t tlk_dma_get_active_irqs(enum tlk_dma_chn chn)
{
    uint8_t active_irqs = 0;

    if (TLK_BM_IS_SET(tlk_dma_reg.irq_status_bit.tc_irq, TLK_BIT(chn)) ) {
        active_irqs |= TLK_BIT(TLK_DMA_IRQ_TC);
    }
    
    if (TLK_BM_IS_SET(tlk_dma_reg.irq_status_bit.error_irq, TLK_BIT(chn)) ) {
        active_irqs |= TLK_BIT(TLK_DMA_IRQ_ERR);
    }

    if (TLK_BM_IS_SET(tlk_dma_reg.irq_status_bit.abort_irq, TLK_BIT(chn)) ) {
        active_irqs |= TLK_BIT(TLK_DMA_IRQ_ABT);
    }

    return active_irqs;
}

static inline void tlk_dma_clear_irqs(enum tlk_dma_chn chn, uint8_t mask)
{
    if (TLK_BM_IS_SET(mask, TLK_BIT(TLK_DMA_IRQ_TC)) ) {
        tlk_dma_reg.irq_status_bit.tc_irq = TLK_BIT(chn);
    }

    if (TLK_BM_IS_SET(mask, TLK_BIT(TLK_DMA_IRQ_ERR)) ) {
        tlk_dma_reg.irq_status_bit.error_irq = TLK_BIT(chn);
    }

    if (TLK_BM_IS_SET(mask, TLK_BIT(TLK_DMA_IRQ_ABT)) ) {
        tlk_dma_reg.irq_status_bit.abort_irq = TLK_BIT(chn);
    }
}
#endif
/* ==== HELPERS ==== */


/* ==== DRIVER APIs ==== */
enum tlk_dma_chn tlk_dma_request_chn(void)
{
    for (uint8_t i = 0;i < TLK_DMA_CHN_MAX;i++) {
        if (!TLK_BM_IS_SET(*(uint8_t *)&tlk_dma_channels, TLK_BIT(i)) ) {
            TLK_BM_SET(*(uint8_t *)&tlk_dma_channels, TLK_BIT(i));
            return i;
        }
    }

    return TLK_DMA_NONE;
}

void tlk_dma_release_chn(enum tlk_dma_chn chn)
{
    TLK_BM_CLR(*(uint8_t *)&tlk_dma_channels, TLK_BIT(chn));
}

void tlk_dma_configure(enum tlk_dma_chn chn, struct tlk_dma_config* config)
{
    tlk_dma_reg.chn[chn].control = (tlk_dma_reg.chn[chn].control & (~TLK_BIT_RNG(2, 31))) | ((*(uint32_t *)config) << 2); // dma config

#ifdef CONFIG_TLK_DMA_PM_DEVICE
    tlk_dma_retention_configs[chn] = *config;
#endif
}

void tlk_dma_start_transfer(enum tlk_dma_chn chn, uint32_t src_addr, uint32_t dst_addr, uint32_t size, enum tlk_dma_transfer_width width)
{
    uint32_t bytes_per_transfer = 1u << width;

    tlk_dma_reg.chn[chn].control_bit.chn_en = 0; // dma_chn_dis
    tlk_dma_reg.chn[chn].src_addr = (uint32_t)(src_addr); // dma_set_address
    tlk_dma_reg.chn[chn].dst_addr = (uint32_t)(dst_addr); // dma_set_address
    tlk_dma_reg.chn[chn].size_bit.tran_size = (size + bytes_per_transfer - 1) / bytes_per_transfer; // dma_set_size
    tlk_dma_reg.chn[chn].size_bit.tran_size_idx = size % bytes_per_transfer; // dma_set_size
    tlk_dma_reg.chn[chn].control_bit.chn_en = 1; // dma_chn_en
}

_tlk_always_inline _tlk_attribute_ram_code_sec_ void tlk_dma_abort(enum tlk_dma_chn chn)
{
    tlk_dma_reg.abort |= TLK_BIT(chn);
    while( !(tlk_dma_reg.irq_status_bit.abort_irq & TLK_BIT(chn)) && tlk_dma_reg.chn[chn].control_bit.chn_en) {}

#ifdef CONFIG_TLK_DMA_IRQ_HANDLER
    if (tlk_dma_chn_handlers[chn] == NULL)
#endif
    {
        tlk_dma_reg.irq_status_bit.abort_irq = TLK_BIT(chn);
    }
}

#ifdef CONFIG_TLK_DMA_IRQ_HANDLER
void tlk_dma_add_callback(enum tlk_dma_chn chn, tlk_dma_chn_handler_t handler)
{
    tlk_dma_chn_handlers[chn] = handler;

    if (handler != NULL) {
        tlk_dma_irq_enable_all(chn);
    } else {
        tlk_dma_irq_disable_all(chn);
    }
}
#endif
/* ==== DRIVER APIs ==== */


/* ==== REGISTER IRQ HELPERS ==== */
#ifdef CONFIG_TLK_DMA_IRQ_HANDLER
_tlk_attribute_ram_code_sec_ static void tlk_dma_irq_init(void) {
    for (uint8_t i = 0;i < TLK_DMA_CHN_MAX;i++) {
        tlk_dma_irq_disable_all(i);
    }

    tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_DMA);
}
TLK_REGISTER_PRE_INIT(tlk_dma_irq_init, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)

_tlk_attribute_ram_code_sec_ static void tlk_dma_irq_handler(void) {
    for (uint8_t i = 0;i < TLK_DMA_CHN_MAX;i++) {
        uint8_t active_irqs = tlk_dma_get_active_irqs(i);
        tlk_dma_clear_irqs(i, active_irqs);

        if (!active_irqs) {
            continue; 
        }

        if (tlk_dma_chn_handlers[i] != NULL) {
            if (active_irqs & TLK_BIT(TLK_DMA_IRQ_TC)) {
                tlk_dma_chn_handlers[i](TLK_DMA_IRQ_TC);
            }

            if (active_irqs & TLK_BIT(TLK_DMA_IRQ_ERR)) {
                tlk_dma_chn_handlers[i](TLK_DMA_IRQ_ERR);
            }

            if (active_irqs & TLK_BIT(TLK_DMA_IRQ_ABT)) {
                tlk_dma_chn_handlers[i](TLK_DMA_IRQ_ABT);
            }
        }
    }
}
TLK_PLIC_ISR_REGISTER(tlk_dma_irq_handler, UNISDK_PLIC_IRQ_NUM_DMA)
#endif
/* ==== REGISTER IRQ HELPERS ==== */


/* ==== REGISTER PM HELPERS ==== */
#ifdef CONFIG_TLK_DMA_PM_DEVICE
static void tlk_dma_restore_context(void) {
    for (uint8_t i = 0;i < TLK_DMA_CHN_MAX;i++) {
        tlk_dma_configure(i, &tlk_dma_retention_configs[i]);
#ifdef CONFIG_TLK_DMA_IRQ_HANDLER
        tlk_dma_add_callback(i, tlk_dma_chn_handlers[i]);
    }

    tlk_plic_interrupt_enable(UNISDK_PLIC_IRQ_NUM_DMA);
#else
    }
#endif
}
TLK_REGISTER_AFTER_SUSPEND(tlk_dma_restore_context, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)
TLK_REGISTER_AFTER_SLEEP(tlk_dma_restore_context, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)
#endif
/* ==== REGISTER PM HELPERS ==== */


/* ==== PREVENT SLEEP ==== */
#ifdef CONFIG_TLK_DMA_PREVENT_SLEEP
static bool tlk_dma_prevent_sleep(void)
{
    for (uint8_t i = 0;i < TLK_DMA_CHN_MAX;i++) {
        if (tlk_dma_reg.chn[i].control_bit.chn_en) {
            return 1;
        }
    }
    return 0;
}

TLK_REGISTER_PREVENT_SUSPEND(tlk_dma_prevent_sleep)
TLK_REGISTER_PREVENT_SLEEP(tlk_dma_prevent_sleep)
#endif
/* ==== PREVENT SLEEP ==== */
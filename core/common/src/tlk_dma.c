#include "core/include/tlk_dma.h"
#include "system/debug/assert/tlk_assert.h"

/* ==== CHANNELS MANAGEMENT ==== */
#define _TLK_GET_CONFIG_WITH_DMA(name) CONFIG_TLK_##name##_DMA
#define TLK_GET_CONFIG_WITH_DMA(name)  _TLK_GET_CONFIG_WITH_DMA(name)

#define TLK_GET_DMA_FUNCTION(num) UNISDK_DMA_FUNCTION_##num

#define TLK_DMA_DECLARE_FUNCTION(num)                                                              \
    TLK_IF_ENABLED(TLK_GET_CONFIG_WITH_DMA(TLK_GET_DMA_FUNCTION(num)), (TLK_DMA_FUNCTION_##num, ))

enum tlk_dma_function_channels
{
    TLK_FOR_CALL(UNISDK_DMA_FUNCTION_COUNT, TLK_DMA_DECLARE_FUNCTION) TLK_DMA_RESERVED_COUNT
};

tlk_static_assert(TLK_DMA_RESERVED_COUNT < UNISDK_DMA_CHN_COUNT,
                  "Too many requests for DMA channels!");

#define TLK_DMA_DECLARE_CHANNEL_FOR_RESERVATION(num) uint8_t tlk_dma##num : 1;
#define TLK_DMA_GET_RESERVATION(num)                 UNISDK_DMA_RESERVATIONS_##num

#define TLK_DMA_DECLARE_CHANNEL_WITH_RESERVATION(num)                                              \
    .tlk_dma##num =                                                                                \
        TLK_IF_ENABLED_ELSE(TLK_GET_CONFIG_WITH_DMA(TLK_DMA_GET_RESERVATION(num)), (1), (0)),

static struct
{
    TLK_FOR_CALL(UNISDK_DMA_CHN_COUNT, TLK_DMA_DECLARE_CHANNEL_FOR_RESERVATION)
} tlk_dma_channels = {TLK_FOR_CALL(UNISDK_DMA_CHN_COUNT, TLK_DMA_DECLARE_CHANNEL_WITH_RESERVATION)};
/* ==== CHANNELS MANAGEMENT ==== */

/* ==== DRIVER APIs ==== */
enum tlk_dma_chn tlk_dma_chn_request(void)
{
    for (uint8_t i = 0; i < UNISDK_DMA_CHN_COUNT; i++)
    {
        if (!TLK_BM_IS_SET(*(uint8_t*) &tlk_dma_channels, TLK_BIT(i)))
        {
            TLK_BM_SET(*(uint8_t*) &tlk_dma_channels, TLK_BIT(i));
            return i;
        }
    }

    return TLK_DMA_NONE;
}

void tlk_dma_chn_release(enum tlk_dma_chn chn)
{
    TLK_BM_CLR(*(uint8_t*) &tlk_dma_channels, TLK_BIT(chn));
}
/* ==== DRIVER APIs ==== */

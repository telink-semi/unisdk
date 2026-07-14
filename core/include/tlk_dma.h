#ifndef TLK_INCLUDE_DRIVERS_DMA_H_
#define TLK_INCLUDE_DRIVERS_DMA_H_

#include "properties/tlk_dma.h"

#define TLK_DECLARE_DMA_CHANNEL(num) TLK_DMA##num,
enum tlk_dma_chn
{
    TLK_DMA_NONE = -1,
    TLK_FOR_CALL(UNISDK_DMA_CHN_COUNT, TLK_DECLARE_DMA_CHANNEL) TLK_DMA_CHN_MAX
};

enum tlk_dma_addr_ctrl
{
    TLK_DMA_ADDR_INCREMENT = 0, /* Increment address,For example,if the transfer direction is: sram
                                   -> peripheral, the source address (sram) needs to be increased */
    TLK_DMA_ADDR_DECREMENT,     /* Decrement address,Not usually used.*/
    TLK_DMA_ADDR_FIX, /* Fixed address,For example,if the transfer direction is: sram -> peripheral,
                         the destination address (peripheral fifo) needs to be fixed.*/
};

enum tlk_dma_mode
{
    TLK_DMA_NORMAL_MODE = 0, /* The SRAM as either source or destination does not require a
                                handshake mechanism because there is no empty-full situation. */
    TLK_DMA_HANDSHAKE_MODE,  /*
                           For the peripheral FIFO as the source, there is the case that the FIFO is
                           empty when DMA fetching data, so the handshake mechanism is needed.  For
                           the peripheral FIFO as the destination, there is the case that the FIFO is
                           full when DMA writing data, so the handshake mechanism is needed.
                         */
};

enum tlk_dma_transfer_width
{
    TLK_DMA_BYTE_WIDTH = 0, /* byte transfer. */
    TLK_DMA_HWORD_WIDTH,    /* half word transfer. */
    TLK_DMA_WORD_WIDTH,     /* word transfer.  */
};

enum tlk_dma_burst_size
{
    TLK_DMA_BURST_1_TRANSFER = 0, /* 1 transfer  */
    TLK_DMA_BURST_2_TRANSFERS,    /* 2 transfers */
    TLK_DMA_BURST_4_TRANSFERS,    /* 4 transfers */
    TLK_DMA_BURST_8_TRANSFERS,    /* 8 transfers */
    TLK_DMA_BURST_16_TRANSFERS,   /* 16 transfers */
    TLK_DMA_BURST_32_TRANSFERS,   /* 32 transfers */
    TLK_DMA_BURST_64_TRANSFERS,   /* 64 transfers */
    TLK_DMA_BURST_128_TRANSFERS,  /* 128 transfers */
};

enum tlk_dma_irq_type
{
    TLK_DMA_IRQ_TC = 0, /* Terminal Count Interrupt */
    TLK_DMA_IRQ_ERR,    /* Error Interrupt */
    TLK_DMA_IRQ_ABT,    /* Abort Interrupt */
};

struct tlk_dma_config
{
    uint32_t dst_req_sel    : UNISDK_DMA_BIT_SIZE_REQ_SEL;      /* DstReqSel   :7:2 or 8:4   */
    uint32_t src_req_sel    : UNISDK_DMA_BIT_SIZE_REQ_SEL;      /* SrcReqSel   :13:8 or 13:9 */
    uint32_t dst_addr_ctrl  : UNISDK_DMA_BIT_SIZE_ADDR_CTRL;    /* DstAddrCtrl :15:14        */
    uint32_t src_addr_ctrl  : UNISDK_DMA_BIT_SIZE_ADDR_CTRL;    /* SrcAddrCtrl :17:16        */
    uint32_t dstmode        : UNISDK_DMA_BIT_SIZE_MODE;         /* DstMode     :18           */
    uint32_t srcmode        : UNISDK_DMA_BIT_SIZE_MODE;         /* SrcMode     :19           */
    uint32_t dstwidth       : UNISDK_DMA_BIT_SIZE_WIDTH;        /* DstWidth    :21:20        */
    uint32_t srcwidth       : UNISDK_DMA_BIT_SIZE_WIDTH;        /* SrcWidth    :23:22        */
    uint32_t src_burst_size : UNISDK_DMA_BIT_SIZE_BURST_SIZE;   /* SrcBurstSize:26:24        */
    uint32_t vacant_bit     : UNISDK_DMA_BIT_SIZE_VACANT;       /* vacant      :27           */
    uint32_t read_num_en    : UNISDK_DMA_BIT_SIZE_READ_NUM_EN;  /* Rnum_en     :28           */
    uint32_t priority       : UNISDK_DMA_BIT_SIZE_PRIORITY;     /* Pri         :29           */
    uint32_t write_num_en   : UNISDK_DMA_BIT_SIZE_WRITE_NUM_EN; /* wnum_en     :30           */
    uint32_t auto_en        : UNISDK_DMA_BIT_SIZE_AUTO_EN;      /* auto_en     :31           */
};

/**
 * @brief Request a DMA channel.
 *
 * @return The allocated DMA channel, or TLK_DMA_NONE if no channel is available.
 */
enum tlk_dma_chn tlk_dma_request_chn(void);
/**
 * @brief Release a previously requested DMA channel.
 *
 * @param chn  DMA channel to release.
 *
 * @return None.
 */
void tlk_dma_release_chn(enum tlk_dma_chn);

/**
 * @brief Configure a DMA channel with the specified settings.
 *
 * @param chn     DMA channel to configure.
 * @param config  Pointer to the DMA configuration structure.
 *
 * @return None.
 */
void tlk_dma_configure(enum tlk_dma_chn chn, struct tlk_dma_config* config);
/**
 * @brief Start a DMA transfer.
 *
 * @param chn      DMA channel.
 * @param src_addr Source address.
 * @param dst_addr Destination address.
 * @param size     Number of transfers.
 * @param width    Transfer width (byte, half-word, or word).
 *
 * @return None.
 */
void tlk_dma_start_transfer(enum tlk_dma_chn chn, uint32_t src_addr, uint32_t dst_addr,
                            uint32_t size, enum tlk_dma_transfer_width width);
/**
 * @brief Abort an ongoing DMA transfer.
 *
 * @param chn  DMA channel to abort.
 *
 * @return None.
 */
void tlk_dma_abort(enum tlk_dma_chn chn);

#ifdef CONFIG_TLK_DMA_IRQ_HANDLER
typedef void (*tlk_dma_chn_handler_t)(enum tlk_dma_irq_type irq_type);
/**
 * @brief Add a callback handler for DMA channel interrupts.
 *
 * @param chn     DMA channel.
 * @param handler Callback function to invoke on DMA interrupt.
 *
 * @return None.
 */
void tlk_dma_add_callback(enum tlk_dma_chn chn, tlk_dma_chn_handler_t handler);
#endif

#endif

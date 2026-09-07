#ifndef DMA_DRIVER_HPP
#define DMA_DRIVER_HPP

#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_dma.h"
#ifdef __cplusplus
}
#endif

class DmaDriver
{
  public:
    explicit DmaDriver(tlk_dma_chn chn) : chn_(chn) {}

    ~DmaDriver()
    {
        tlk_dma_release_chn(chn_);
    }

    DmaDriver(const DmaDriver&)            = delete;
    DmaDriver& operator=(const DmaDriver&) = delete;

    DmaDriver(DmaDriver&& other) noexcept : chn_(other.chn_)
    {
        other.chn_ = TLK_DMA_NONE;
    }

    DmaDriver& operator=(DmaDriver&& other) noexcept
    {
        if (this != &other)
        {
            if (chn_ != TLK_DMA_NONE)
            {
                tlk_dma_release_chn(chn_);
            }
            chn_       = other.chn_;
            other.chn_ = TLK_DMA_NONE;
        }
        return *this;
    }

    static DmaDriver request()
    {
        return DmaDriver(tlk_dma_request_chn());
    }

    void configure(tlk_dma_config* config)
    {
        tlk_dma_configure(chn_, config);
    }

    void startTransfer(uint32_t src_addr, uint32_t dst_addr, uint32_t size,
                       tlk_dma_transfer_width width)
    {
        tlk_dma_start_transfer(chn_, src_addr, dst_addr, size, width);
    }

    void abort()
    {
        tlk_dma_abort(chn_);
    }

#ifdef CONFIG_TLK_DMA_IRQ_HANDLER
    void addCallback(tlk_dma_chn_handler_t handler)
    {
        tlk_dma_add_callback(chn_, handler);
    }
#endif

    tlk_dma_chn channel() const
    {
        return chn_;
    }

  private:
    tlk_dma_chn chn_;
};

#endif

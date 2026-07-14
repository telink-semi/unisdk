#include "core/include/tlk_uart.h"
#include "api/include/tlk_time.h"
#include "common/include/tlk_init.h"
#include "core/include/tlk_clock.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_dma.h"
#include "core/include/tlk_gpio.h"
#include "core/include/tlk_plic.h"
#include "properties/tlk_plic.h"
#include "registers/tlk_chip.h"
#include "registers/tlk_dma.h"
#include "registers/tlk_uart.h"

/* ==== MATH HELPERS ==== */
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
/* ==== MATH HELPERS ==== */

/* ==== ENUM HELPERS ==== */
enum tlk_uart_xfer_mode
{
    TLK_UART_XFER_BLOCKING = 0, // Blocking
    TLK_UART_XFER_IRQ,          // Interrupt-based
    TLK_UART_XFER_DMA           // DMA-based
};

enum tlk_uart_rx_dma_mode
{
    TLK_UART_RX_DMA_FIXED = 0,
    TLK_UART_RX_DMA_WITH_TAIL,
};

enum tlk_uart_irq
{
    /* When the number of rxfifo reaches the set threshold(tlk_uart_set_rx_irq_trig_level), an
     * interrupt is generated, and the interrupt flag is automatically cleared.
     */
    TLK_UART_IRQ_RXFIFO = TLK_BIT(2),

    /* When the number of txfifo is less than or equal to the set
     * threshold(tlk_uart_set_tx_irq_trig_level), an interrupt is generated and the
     * interrupt flag is automatically cleared.
     */
    TLK_UART_IRQ_TXFIFO = TLK_BIT(3),

    /* When no data is received in rx_timeout, rx_done is generated. If
     * tlk_uart_set_auto_clr_rx_fifo_ptr is enabled, the interrupt flag is automatically
     * cleared. If tlk_uart_set_auto_clr_rx_fifo_ptr is disabled, the interrupt flag
     * must be manually cleared.
     */
    TLK_UART_IRQ_RXDONE = TLK_BIT(4),

    /* When there is no data in the tx_fifo, tx_done is generated, and the
     * interrupt flag bit needs to be manually cleared.
     */
    TLK_UART_IRQ_TXDONE = TLK_BIT(5),

    /* When the UART receives data incorrectly(such as a parity
     * error or a stop bit error), the interrupt is generated,the
     * interrupt flag bit needs to be manually cleared.
     */
    TLK_UART_IRQ_RXERR = TLK_BIT(6),
};
/* ==== ENUM HELPERS ==== */

/* ==== DMA CONFIG STRUCTURE HELPERS ==== */
#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_DMA)
#define TLK_UART_DECLARE_TX_DMA_CONFIG(num)                                                        \
    struct tlk_dma_config tlk_uart##num##_tx_dma_config = {                                        \
        .dst_req_sel    = UNISDK_DMA_FUNC_NUM_UART##num##_TX,                                      \
        .src_req_sel    = 0,                                                                       \
        .dst_addr_ctrl  = TLK_DMA_ADDR_FIX,                                                        \
        .src_addr_ctrl  = TLK_DMA_ADDR_INCREMENT,                                                  \
        .dstmode        = TLK_DMA_HANDSHAKE_MODE,                                                  \
        .srcmode        = TLK_DMA_NORMAL_MODE,                                                     \
        .dstwidth       = TLK_DMA_WORD_WIDTH,                                                      \
        .srcwidth       = TLK_DMA_WORD_WIDTH,                                                      \
        .src_burst_size = TLK_DMA_BURST_1_TRANSFER,                                                \
        .read_num_en    = 0,                                                                       \
        .priority       = 0,                                                                       \
        .write_num_en   = 0,                                                                       \
        .auto_en        = 0,                                                                       \
    };

#define TLK_UART_DECLARE_RX_DMA_CONFIG(num)                                                        \
    struct tlk_dma_config tlk_uart##num##_rx_dma_config = {                                        \
        .dst_req_sel    = 0,                                                                       \
        .src_req_sel    = UNISDK_DMA_FUNC_NUM_UART##num##_RX,                                      \
        .dst_addr_ctrl  = TLK_DMA_ADDR_INCREMENT,                                                  \
        .src_addr_ctrl  = TLK_DMA_ADDR_FIX,                                                        \
        .dstmode        = TLK_DMA_NORMAL_MODE,                                                     \
        .srcmode        = TLK_DMA_HANDSHAKE_MODE,                                                  \
        .dstwidth       = TLK_DMA_WORD_WIDTH,                                                      \
        .srcwidth       = TLK_DMA_WORD_WIDTH,                                                      \
        .src_burst_size = TLK_DMA_BURST_1_TRANSFER,                                                \
        .read_num_en    = 0,                                                                       \
        .priority       = 0,                                                                       \
        .write_num_en   = 0,                                                                       \
        .auto_en        = 0,                                                                       \
    };

#define TLK_UART_DECLARE_TX_DMA_CONFIG_IF_ENABLED(num)                                             \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_TX_DMA, (TLK_UART_DECLARE_TX_DMA_CONFIG(num)))
#define TLK_UART_DECLARE_RX_DMA_CONFIG_IF_ENABLED(num)                                             \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_RX_DMA, (TLK_UART_DECLARE_RX_DMA_CONFIG(num)))

TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_DECLARE_TX_DMA_CONFIG_IF_ENABLED)
TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_DECLARE_RX_DMA_CONFIG_IF_ENABLED)

#define TLK_UART_GET_TX_DMA_CONFIG_AS_CASE(num)                                                    \
    case num:                                                                                      \
        uart_tx_dma_config = &tlk_uart##num##_tx_dma_config;                                       \
        break;
#define TLK_UART_GET_RX_DMA_CONFIG_AS_CASE(num)                                                    \
    case num:                                                                                      \
        uart_rx_dma_config = &tlk_uart##num##_rx_dma_config;                                       \
        break;

#define TLK_UART_GET_TX_DMA_CONFIG_AS_CASE_IF_ENABLED(num)                                         \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_TX_DMA, (TLK_UART_GET_TX_DMA_CONFIG_AS_CASE(num)))
#define TLK_UART_GET_RX_DMA_CONFIG_AS_CASE_IF_ENABLED(num)                                         \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_RX_DMA, (TLK_UART_GET_RX_DMA_CONFIG_AS_CASE(num)))

#define TLK_UART_GET_TX_DMA_CONFIG(num)                                                            \
    struct tlk_dma_config* uart_tx_dma_config = NULL;                                              \
    switch (num)                                                                                   \
    {                                                                                              \
        TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_GET_TX_DMA_CONFIG_AS_CASE_IF_ENABLED)             \
    default:                                                                                       \
        uart_tx_dma_config = NULL;                                                                 \
    }

#define TLK_UART_GET_RX_DMA_CONFIG(num)                                                            \
    struct tlk_dma_config* uart_rx_dma_config = NULL;                                              \
    switch (num)                                                                                   \
    {                                                                                              \
        TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_GET_RX_DMA_CONFIG_AS_CASE_IF_ENABLED)             \
    default:                                                                                       \
        uart_rx_dma_config = NULL;                                                                 \
    }
#endif
/* ==== DMA CONFIG STRUCTURE HELPERS ==== */

/* ==== UART INSTANCE STRUCTURE HELPERS ==== */
struct tlk_uart_instance
{
#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_IRQ)
    volatile tlk_uart_rx_handler_t rx_handler;
    volatile uint32_t              rx_buff_size;
    uint8_t* volatile rx_buff;
    volatile uint32_t rx_cnt;

    volatile tlk_uart_tx_handler_t tx_handler;
    volatile uint32_t              tx_buff_size;
    const uint8_t* volatile tx_buff;
    volatile uint32_t tx_cnt;

    const uint8_t irq_num;
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_DMA)
    volatile uint8_t tx_dma_chn : 4;
    volatile uint8_t rx_dma_chn : 4;
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_FLOW_CONTROL)
    const uint8_t cts_pin_function;
    const uint8_t rts_pin_function;
#endif

    const uint8_t tx_pin_function;
    const uint8_t rx_pin_function;

    const uint16_t tx_mode : 2;
    const uint16_t rx_mode : 2;

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_DMA)
    const uint8_t rx_dma_mode : 1;
#endif

    volatile uint16_t tx_byte_index  : 2;
    volatile uint16_t rx_byte_index  : 2;
    volatile uint16_t waiting_for_tx : 1;
    volatile uint16_t waiting_for_rx : 1;
};

#define TLK_UART_DECLARE_INSTANCE(num)                                                             \
    struct tlk_uart_instance tlk_uart##num##_instance = {                                          \
        TLK_IF_ENABLED(CONFIG_TLK_UART_USED_IRQ, (.irq_num = UNISDK_PLIC_IRQ_NUM_UART##num, ))     \
            TLK_IF_ENABLED(CONFIG_TLK_UART_USED_FLOW_CONTROL,                                      \
                           (.cts_pin_function = TLK_GPIO_MUX_UART##num##_CTS, ))                   \
                TLK_IF_ENABLED(CONFIG_TLK_UART_USED_FLOW_CONTROL,                                  \
                               (.rts_pin_function = TLK_GPIO_MUX_UART##num##_RTS, ))               \
                                                                                                   \
                    .tx_pin_function = TLK_GPIO_MUX_UART##num##_TX,                                \
        .rx_pin_function             = TLK_GPIO_MUX_UART##num##_RX,                                \
        .tx_mode                     = CONFIG_TLK_UART##num##_TX_MODE,                             \
        .rx_mode                     = CONFIG_TLK_UART##num##_RX_MODE,                             \
                                                                                                   \
        TLK_IF_ENABLED(CONFIG_TLK_UART##num##_RX_DMA,                                              \
                       (.rx_dma_mode = CONFIG_TLK_UART##num##_RX_DMA_MODE))};

#define TLK_UART_DECLARE_INSTANCE_IF_ENABLED(num)                                                  \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_ENABLED, (TLK_UART_DECLARE_INSTANCE(num)))

TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_DECLARE_INSTANCE_IF_ENABLED)

#define TLK_UART_GET_INSTANCE_AS_CASE(num)                                                         \
    case num:                                                                                      \
        uart_instance = &tlk_uart##num##_instance;                                                 \
        break;

#define TLK_UART_GET_INSTANCE_AS_CASE_IF_ENABLED(num)                                              \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_ENABLED, (TLK_UART_GET_INSTANCE_AS_CASE(num)))

#define TLK_UART_GET_INSTANCE(num)                                                                 \
    struct tlk_uart_instance* uart_instance = NULL;                                                \
    switch (num)                                                                                   \
    {                                                                                              \
        TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_GET_INSTANCE_AS_CASE_IF_ENABLED)                  \
    }
/* ==== UART INSTANCE STRUCTURE HELPERS ==== */

/* ==== UART RETENTION STRUCTURE HELPERS ==== */
#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_PM_DEVICE)
struct tlk_uart_retention
{
    uint32_t                     baudrate;
    enum tlk_uart_parity         parity;
    enum tlk_uart_stop_bit       stop_bit;
    enum tlk_uart_rx_timeout_mul rx_timeout_mul;

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_FLOW_CONTROL)
    enum tlk_uart_flow_control  flow_control;
    enum tlk_uart_flow_polarity flow_polarity;

    struct tlk_gpio_port_pin cts_port_pin;
    struct tlk_gpio_port_pin rts_port_pin;
#endif

    struct tlk_gpio_port_pin rx_port_pin;
    struct tlk_gpio_port_pin tx_port_pin;
};

#define TLK_UART_DECLARE_RETENTION(num) struct tlk_uart_retention tlk_uart##num##_retention;

#define TLK_UART_DECLARE_RETENTION_IF_ENABLED(num)                                                 \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_PM_DEVICE, (TLK_UART_DECLARE_RETENTION(num)))

TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_DECLARE_RETENTION_IF_ENABLED)

#define TLK_UART_GET_RETENTION_AS_CASE(num)                                                        \
    case num:                                                                                      \
        uart_retention = &tlk_uart##num##_retention;                                               \
        break;

#define TLK_UART_GET_RETENTION_AS_CASE_IF_ENABLED(num)                                             \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_PM_DEVICE, (TLK_UART_GET_RETENTION_AS_CASE(num)))

#define TLK_UART_GET_RETENTION(num)                                                                \
    struct tlk_uart_retention* uart_retention = NULL;                                              \
    switch (num)                                                                                   \
    {                                                                                              \
        TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_GET_RETENTION_AS_CASE_IF_ENABLED)                 \
    default:                                                                                       \
        uart_retention = NULL;                                                                     \
    }
#endif
/* ==== UART RETENTION STRUCTURE HELPERS ==== */

/* ==== IRQ HELPERS ==== */
static _tlk_always_inline uint32_t tlk_uart_get_irq_status(enum tlk_uart_module uart_num,
                                                           enum tlk_uart_irq    status)
{
    return tlk_uart_reg(uart_num).irq_status & status;
}

static void tlk_uart_clr_irq_status(enum tlk_uart_module uart_num, enum tlk_uart_irq status)
{
    /*
     * the purpose of judging the status of TLK_UART_IRQ_RXDONE:
     * when TLK_UART_IRQ_RXDONE is cleared, the TLK_UART_IRQ_RXFIFO is also cleared, because rx_fifo
     * is cleared, and the software pointer is also cleared: dma: rx_fifo is cleared because when
     * the send length is larger than the receive length, when TLK_UART_IRQ_RXDONE is generated,
     * there is data in rx_fifo, and TLK_UART_IRQ_RXDONE interrupt will always be generated,
     * affecting the function. no_dma: for unified processing with DMA, because rx_fifo is cleared,
     * the software pointer also needs to be cleared, otherwise an exception occurs. the purpose of
     * judging the status of the TLK_UART_IRQ_RXFIFO interrupt: Because the state of the err needs
     * to be cleared by the clearing rx_buff when an err interrupt is generated, the software
     * pointer needs to be cleared.
     */
    TLK_UART_GET_INSTANCE(uart_num)

    if (status & TLK_UART_IRQ_RXDONE)
    {
        tlk_uart_reg(uart_num).irq_status = TLK_UART_IRQ_RXFIFO;
        uart_instance->rx_byte_index      = 0; // uart_clr_rx_index //clear software pointer
    }
    if (status & TLK_UART_IRQ_RXFIFO)
    {
        uart_instance->rx_byte_index = 0; // uart_clr_rx_index //clear software pointer
    }

    if (status & TLK_UART_IRQ_TXDONE)
    {
        tlk_uart_reg(uart_num).irq_status = TLK_UART_IRQ_TXFIFO;
        uart_instance->tx_byte_index      = 0; // uart_clr_rx_index //clear software pointer
    }
    if (status & TLK_UART_IRQ_TXFIFO)
    {
        uart_instance->tx_byte_index = 0; // uart_clr_rx_index //clear software pointer
    }

    tlk_uart_reg(uart_num).irq_status = status;
}

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_IRQ)
static _tlk_always_inline void tlk_uart_enable_irq(enum tlk_uart_module uart_num,
                                                   enum tlk_uart_irq    irqs)
{
    tlk_uart_reg(uart_num).rx_timeout1 |= irqs;
}

static _tlk_always_inline void tlk_uart_disable_irq(enum tlk_uart_module uart_num,
                                                    enum tlk_uart_irq    irqs)
{
    tlk_uart_reg(uart_num).rx_timeout1 &= ~irqs;
}
#endif
/* ==== IRQ HELPERS ==== */

/* ==== CONFIG HELPERS ==== */
static void tlk_uart_reset_hw_fsm(enum tlk_uart_module uart_num)
{
    TLK_UART_GET_INSTANCE(uart_num)

    if (uart_num == 0)
    {
        tlk_reset_reg.reset0_bit.uart0 = 0;
        tlk_reset_reg.reset0_bit.uart0 = 1;
    }
    else if (uart_num == 1)
    {
        tlk_reset_reg.reset0_bit.uart1 = 0;
        tlk_reset_reg.reset0_bit.uart1 = 1;
    }

    uart_instance->tx_byte_index = 0; // uart_clr_tx_index
    uart_instance->rx_byte_index = 0; // uart_clr_rx_index
}

static uint8_t tlk_uart_is_prime(uint32_t n)
{
    uint32_t i = 5;
    if (n <= 3)
    {
        return 1; // although n is prime, the bwpc must be larger than 2.
    }
    else if ((n % 2 == 0) || (n % 3 == 0))
    {
        return 0;
    }
    else
    {
        for (i = 5; i * i < n; i += 6)
        {
            if ((n % i == 0) || (n % (i + 2)) == 0)
            {
                return 0;
            }
        }
        return 1;
    }
}

static void tlk_uart_cal_div_and_bwpc(uint32_t baudrate, uint32_t pclk, uint16_t* div,
                                      uint8_t* bwpc)
{
    uint8_t  i = 0, j = 0;
    uint32_t primeInt = 0;
    uint8_t  primeDec = 0;
    uint32_t D_intdec[13], D_int[13];
    uint8_t  D_dec[13];

    primeInt = pclk / baudrate;
    primeDec = 10 * pclk / baudrate - 10 * primeInt;

    if (tlk_uart_is_prime(primeInt))
    {                  // primeInt is prime
        primeInt += 1; //+1 must be not prime. and primeInt must be larger than 2.
    }
    else
    {
        if (primeDec > 5)
        { // >5
            primeInt += 1;
            if (tlk_uart_is_prime(primeInt))
            {
                primeInt -= 1;
            }
        }
    }

    for (i = 3; i <= 15; i++)
    {
        D_intdec[i - 3] = (10 * primeInt) / (i + 1);                     // get the LSB
        D_dec[i - 3]    = D_intdec[i - 3] - 10 * (D_intdec[i - 3] / 10); // get the decimal section
        D_int[i - 3]    = D_intdec[i - 3] / 10;                          // get the integer section
    }

    // find the max and min one decimation point
    uint8_t  position_min = 0, position_max = 0;
    uint32_t min = 0xffffffff, max = 0x00;
    for (j = 0; j < 13; j++)
    {
        if ((D_dec[j] <= min) && (D_int[j] != 0x01))
        {
            min          = D_dec[j];
            position_min = j;
        }
        if (D_dec[j] >= max)
        {
            max          = D_dec[j];
            position_max = j;
        }
    }

    if ((D_dec[position_min] < 5) && (D_dec[position_max] >= 5))
    {
        if (D_dec[position_min] < (10 - D_dec[position_max]))
        {
            *bwpc = position_min + 3;
            *div  = D_int[position_min] - 1;
        }
        else
        {
            *bwpc = position_max + 3;
            *div  = D_int[position_max];
        }
    }
    else if ((D_dec[position_min] < 5) && (D_dec[position_max] < 5))
    {
        *bwpc = position_min + 3;
        *div  = D_int[position_min] - 1;
    }
    else
    {
        *bwpc = position_max + 3;
        *div  = D_int[position_max];
    }
}

static void tlk_uart_init(enum tlk_uart_module uart_num, uint16_t div, uint8_t bwpc,
                          enum tlk_uart_parity parity, enum tlk_uart_stop_bit stop_bit)
{
    tlk_uart_reg(uart_num).control0_bit.bpwc    = bwpc; // set bwpc
    tlk_uart_reg(uart_num).clock_div            = div;
    tlk_uart_reg(uart_num).clock_div_bit.div_en = 1; // set div_clock

    // parity config
    if (parity)
    {
        tlk_uart_reg(uart_num).control1_bit.parity_en = 1; // enable parity function

        if (parity == TLK_UART_PARITY_EVEN)
        {
            tlk_uart_reg(uart_num).control1_bit.parity_polarity = 0; // enable even parity
        }
        else if (parity == TLK_UART_PARITY_ODD)
        {
            tlk_uart_reg(uart_num).control1_bit.parity_polarity = 1; // enable odd parity
        }
    }
    else
    {
        tlk_uart_reg(uart_num).control1_bit.parity_en = 0; // disable parity function
    }

    tlk_uart_reg(uart_num).control1_bit.stop_bit         = stop_bit; // stop bit config
    tlk_uart_reg(uart_num).control0_bit.rxtimeout_rts_en = 0;        // uart_rts_stop_rxtimeout_dis
    tlk_uart_reg(uart_num).control4_bit.rxdone_rts_en    = 0;        // uart_rxdone_rts_dis
}

static uint8_t tlk_uart_calc_frame_bits(enum tlk_uart_parity   parity,
                                        enum tlk_uart_stop_bit stop_bit)
{
    uint8_t bits = 1 + 8; /* start + data */

    if (parity != TLK_UART_PARITY_NONE)
    {
        bits += 1;
    }

    switch (stop_bit)
    {
    case TLK_UART_STOP_BIT_ONE:
        bits += 1;
        break;
    case TLK_UART_STOP_BIT_ONE_DOT_FIVE:
    case TLK_UART_STOP_BIT_TWO:
        bits += 2;
        break;
    }

    return bits;
}

static _tlk_always_inline void tlk_uart_set_rx_timeout(enum tlk_uart_module uart_num, uint8_t bwpc,
                                                       uint8_t                      bit_cnt,
                                                       enum tlk_uart_rx_timeout_mul mul)
{
    tlk_uart_reg(uart_num).rx_timeout0           = (bwpc + 1) * bit_cnt; // one frame time
    tlk_uart_reg(uart_num).rx_timeout1_bit.scale = mul;
}

static _tlk_always_inline void tlk_uart_set_tx_irq_trig_level(enum tlk_uart_module uart_num,
                                                              uint8_t              tx_level)
{
    tlk_uart_reg(uart_num).control3_bit.tx_irq_trig_lvl = tx_level;
}

static _tlk_always_inline void tlk_uart_set_auto_clr_rx_fifo_ptr(enum tlk_uart_module uart_num,
                                                                 uint8_t              en)
{
    tlk_uart_reg(uart_num).control0_bit.auto_rxclr_en = en;
}

static _tlk_always_inline void tlk_uart_set_rx_irq_trig_level(enum tlk_uart_module uart_num,
                                                              uint8_t              rx_level)
{
    tlk_uart_set_auto_clr_rx_fifo_ptr(uart_num, 0);
    tlk_uart_reg(uart_num).control3_bit.rx_irq_trig_lvl = rx_level;
}
/* ==== CONFIG HELPERS ==== */

/* ==== SEND HELPERS ==== */
static _tlk_always_inline uint8_t tlk_uart_get_tx_fifo_count(enum tlk_uart_module uart_num)
{
    return tlk_uart_reg(uart_num).buf_cnt_bit.tx_buf_cnt;
}

static _tlk_always_inline bool tlk_uart_is_tx_fifo_full(enum tlk_uart_module uart_num)
{
    return tlk_uart_get_tx_fifo_count(uart_num) == 8;
}

static void tlk_uart_send_byte(enum tlk_uart_module uart_num, uint8_t data)
{
    TLK_UART_GET_INSTANCE(uart_num)

    tlk_uart_reg(uart_num).data_buf[uart_instance->tx_byte_index] = data;
    uart_instance->tx_byte_index++;
    uart_instance->tx_byte_index &= 0x03;
}
/* ==== SEND HELPERS ==== */

/* ==== RECEIVE HELPERS ==== */
static _tlk_always_inline uint8_t tlk_uart_get_rx_fifo_count(enum tlk_uart_module uart_num)
{
    return tlk_uart_reg(uart_num).buf_cnt_bit.rx_buf_cnt;
}

static uint8_t tlk_uart_read_byte(enum tlk_uart_module uart_num)
{
    TLK_UART_GET_INSTANCE(uart_num)

    uint8_t rx_data = tlk_uart_reg(uart_num).data_buf[uart_instance->rx_byte_index];
    uart_instance->rx_byte_index++;
    uart_instance->rx_byte_index &= 0x03;
    return rx_data;
}
/* ==== RECEIVE HELPERS ==== */

/* ==== CRITICAL PRINTOUT ==== */
#if TLK_IS_ENABLED(CONFIG_TLK_CRITICAL_PRINTOUT)
void tlk_uart_configure_critical(enum tlk_uart_module uart_num, uint32_t baudrate,
                                 struct tlk_gpio_port_pin tx_port_pin)
{
    TLK_UART_GET_INSTANCE(uart_num)
    uint16_t div  = 0;
    uint8_t  bwpc = 0;

    tlk_gpio_configure(tx_port_pin.port, tx_port_pin.pin, TLK_GPIO_INPUT_PULL_UP);
    tlk_gpio_set_mux(tx_port_pin.port, tx_port_pin.pin, uart_instance->tx_pin_function);
    tlk_gpio_disable(tx_port_pin.port, tx_port_pin.pin);

    tlk_uart_reset_hw_fsm(uart_num);
    tlk_uart_cal_div_and_bwpc(baudrate, tlk_sys_clk.pclk * 1000 * 1000, &div, &bwpc);
    tlk_uart_init(uart_num, div, bwpc, TLK_UART_PARITY_NONE, TLK_UART_STOP_BIT_ONE);

    tlk_uart_reg(uart_num).control0_bit.ndma_rxdone_en = 1; // uart_rxdone_sel  // UART_NO_DMA_MODE
    tlk_uart_reg(uart_num).control1_bit.cts_en = 0; // uart_set_cts_dis // disable CTS function
    tlk_uart_reg(uart_num).control2_bit.rts_en = 0; // uart_set_rts_dis // disable RTS function
    tlk_uart_set_tx_irq_trig_level(uart_num, 0);
}

void tlk_uart_send_critical(enum tlk_uart_module uart_num, const char* buf, uint32_t buff_size)
{
    for (uint32_t i = 0; i < buff_size; i++)
    {
        tlk_uart_send_byte(uart_num, buf[i]);
        while (!tlk_uart_get_irq_status(uart_num, TLK_UART_IRQ_TXDONE))
            ;
    }
}
#endif
/* ==== CRITICAL PRINTOUT ==== */

/* ==== DRIVER APIs ==== */
void tlk_uart_configure(enum tlk_uart_module uart_num, struct tlk_uart_config* config)
{
    TLK_UART_GET_INSTANCE(uart_num)

    uint16_t div        = 0;
    uint8_t  bwpc       = 0;
    uint8_t  frame_bits = 0;

    tlk_uart_reset_hw_fsm(uart_num);
    tlk_uart_cal_div_and_bwpc(config->baudrate, tlk_sys_clk.pclk * 1000 * 1000, &div, &bwpc);
    tlk_uart_init(uart_num, div, bwpc, config->parity, config->stop_bit);

    frame_bits = tlk_uart_calc_frame_bits(config->parity, config->stop_bit);
    tlk_uart_set_rx_timeout(uart_num, bwpc, frame_bits, config->rx_timeout_mul);

    tlk_uart_reg(uart_num).control0_bit.ndma_rxdone_en = 1; // uart_rxdone_sel  // UART_NO_DMA_MODE
    tlk_uart_reg(uart_num).control1_bit.cts_en = 0; // uart_set_cts_dis // disable CTS function
    tlk_uart_reg(uart_num).control2_bit.rts_en = 0; // uart_set_rts_dis // disable RTS function

    if (uart_instance->tx_mode == TLK_UART_XFER_BLOCKING)
    {
        tlk_uart_set_tx_irq_trig_level(uart_num, 0);
    }
    else
    {
        tlk_uart_set_tx_irq_trig_level(uart_num, 4);
    }

    tlk_uart_set_rx_irq_trig_level(uart_num, 4);

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_IRQ)
    if (uart_instance->tx_mode || uart_instance->rx_mode)
    {
        tlk_plic_interrupt_enable(uart_instance->irq_num);
    }
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_DMA)
    TLK_UART_GET_TX_DMA_CONFIG(uart_num)
    TLK_UART_GET_RX_DMA_CONFIG(uart_num)

    if (uart_instance->tx_mode == TLK_UART_XFER_DMA)
    {
        tlk_dma_configure(uart_instance->tx_dma_chn, uart_tx_dma_config);
    }

    if (uart_instance->rx_mode == TLK_UART_XFER_DMA)
    {
        tlk_uart_reg(uart_num).control0_bit.ndma_rxdone_en = 0; // uart_rxdone_sel // UART_DMA_MODE
        tlk_dma_configure(uart_instance->rx_dma_chn, uart_rx_dma_config);
    }
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_PM_DEVICE)
    TLK_UART_GET_RETENTION(uart_num)

    if (uart_retention != NULL)
    {
        uart_retention->baudrate       = config->baudrate;
        uart_retention->parity         = config->parity;
        uart_retention->stop_bit       = config->stop_bit;
        uart_retention->rx_timeout_mul = config->rx_timeout_mul;
    }
#endif

    tlk_uart_configure_pinmux(uart_num, config);
}

void tlk_uart_configure_pinmux(enum tlk_uart_module uart_num, struct tlk_uart_config* config)
{
    TLK_UART_GET_INSTANCE(uart_num)

    tlk_gpio_configure(config->tx_port_pin.port, config->tx_port_pin.pin, TLK_GPIO_INPUT_PULL_UP);
    tlk_gpio_set_mux(
        config->tx_port_pin.port, config->tx_port_pin.pin, uart_instance->tx_pin_function);
    tlk_gpio_disable(config->tx_port_pin.port, config->tx_port_pin.pin);

    tlk_gpio_configure(config->rx_port_pin.port, config->rx_port_pin.pin, TLK_GPIO_INPUT_PULL_UP);
    tlk_gpio_set_mux(
        config->rx_port_pin.port, config->rx_port_pin.pin, uart_instance->rx_pin_function);
    tlk_gpio_disable(config->rx_port_pin.port, config->rx_port_pin.pin);

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_PM_DEVICE)
    TLK_UART_GET_RETENTION(uart_num)

    if (uart_retention != NULL)
    {
        uart_retention->rx_port_pin = config->rx_port_pin;
        uart_retention->tx_port_pin = config->tx_port_pin;
    }
#endif
}

enum tlk_uart_status tlk_uart_send_bytes(enum tlk_uart_module uart_num, const uint8_t* buff,
                                         uint32_t buff_size, tlk_uart_tx_handler_t tx_handler,
                                         uint32_t timeout_us)
{
    TLK_UART_GET_INSTANCE(uart_num)

    if (uart_instance->waiting_for_tx)
    {
        return TLK_UART_ERROR;
    }

    (void) timeout_us; // unused parameter in IRQ and DMA modes

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_BLOCKING)
    if (uart_instance->tx_mode == TLK_UART_XFER_BLOCKING)
    {
        uint32_t start_us             = tlk_api_micros();
        uart_instance->waiting_for_tx = 1;

        for (uint32_t i = 0; i < buff_size; i++)
        {
            while (tlk_uart_is_tx_fifo_full(uart_num))
            {
                if ((tlk_api_micros() - start_us) >= timeout_us)
                {
                    tlk_uart_clr_irq_status(uart_num, TLK_UART_IRQ_TXDONE);

                    uart_instance->waiting_for_tx = 0;
                    return TLK_UART_TIMEOUT;
                }
            }

            tlk_uart_send_byte(uart_num, buff[i]);
        }

        while (!tlk_uart_get_irq_status(uart_num, TLK_UART_IRQ_TXDONE))
        {
            if ((tlk_api_micros() - start_us) >= timeout_us)
            {
                tlk_uart_clr_irq_status(uart_num, TLK_UART_IRQ_TXDONE);

                uart_instance->waiting_for_tx = 0;
                return TLK_UART_TIMEOUT;
            }
        }

        tlk_uart_clr_irq_status(uart_num, TLK_UART_IRQ_TXDONE);
        uart_instance->waiting_for_tx = 0;

        if (tx_handler)
            tx_handler();
    }
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_IRQ)
    if (uart_instance->tx_mode == TLK_UART_XFER_IRQ)
    {
        uart_instance->tx_cnt         = 0;
        uart_instance->tx_handler     = tx_handler;
        uart_instance->tx_buff_size   = buff_size;
        uart_instance->tx_buff        = buff;
        uart_instance->waiting_for_tx = 1;

        tlk_uart_clr_irq_status(uart_num, TLK_UART_IRQ_TXDONE);
        tlk_uart_enable_irq(uart_num, TLK_UART_IRQ_TXDONE | TLK_UART_IRQ_TXFIFO);
    }
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_DMA)
    if (uart_instance->tx_mode == TLK_UART_XFER_DMA)
    {
        uart_instance->tx_handler     = tx_handler;
        uart_instance->waiting_for_tx = 1;

        tlk_uart_clr_irq_status(uart_num, TLK_UART_IRQ_TXDONE);
        tlk_uart_enable_irq(uart_num, TLK_UART_IRQ_TXDONE);
        tlk_dma_start_transfer(uart_instance->tx_dma_chn,
                               (uint32_t) (buff),
                               (uint32_t) tlk_uart_reg(uart_num).data_buf,
                               buff_size,
                               TLK_DMA_WORD_WIDTH);
    }
#endif

    return TLK_UART_OK;
}

enum tlk_uart_status tlk_uart_receive_bytes(enum tlk_uart_module uart_num, uint8_t* buff,
                                            uint32_t buff_size, tlk_uart_rx_handler_t rx_handler)
{
    TLK_UART_GET_INSTANCE(uart_num)

    if (uart_instance->waiting_for_rx)
    {
        return TLK_UART_ERROR;
    }

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_IRQ)
    if (uart_instance->rx_mode == TLK_UART_XFER_IRQ)
    {
        uart_instance->rx_cnt         = 0;
        uart_instance->rx_handler     = rx_handler;
        uart_instance->rx_buff_size   = buff_size;
        uart_instance->rx_buff        = buff;
        uart_instance->waiting_for_rx = 1;

        tlk_uart_clr_irq_status(uart_num, TLK_UART_IRQ_RXDONE | TLK_UART_IRQ_RXERR);
        tlk_uart_enable_irq(uart_num,
                            TLK_UART_IRQ_RXDONE | TLK_UART_IRQ_RXFIFO | TLK_UART_IRQ_RXERR);
    }
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_DMA)
    if (uart_instance->rx_mode == TLK_UART_XFER_DMA)
    {
        uart_instance->rx_cnt         = 0;
        uart_instance->rx_handler     = rx_handler;
        uart_instance->rx_buff_size   = buff_size;
        uart_instance->rx_buff        = buff;
        uart_instance->waiting_for_rx = 1;

        tlk_uart_clr_irq_status(uart_num, TLK_UART_IRQ_RXDONE | TLK_UART_IRQ_RXERR);
        tlk_uart_enable_irq(uart_num, TLK_UART_IRQ_RXDONE | TLK_UART_IRQ_RXERR);

        if (uart_instance->rx_dma_mode == TLK_UART_RX_DMA_WITH_TAIL && buff_size < 4)
        {
            tlk_uart_enable_irq(uart_num, TLK_UART_IRQ_RXFIFO);
        }

        // Subtract 3 so the DMA transfer size is calculated using only complete
        // 4-byte words. Without this, the transfer size is rounded up and the DMA
        // may write past the end of the destination buffer.

        tlk_dma_start_transfer(uart_instance->rx_dma_chn,
                               (uint32_t) tlk_uart_reg(uart_num).data_buf,
                               (uint32_t) (buff),
                               buff_size - 3,
                               TLK_DMA_WORD_WIDTH);
    }
#endif

    return TLK_UART_OK;
}

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_FLOW_CONTROL)
void tlk_uart_configure_flow_control(enum tlk_uart_module                 uart_num,
                                     struct tlk_uart_flow_control_config* config)
{
    if (config->flow_control == TLK_UART_FLOW_RTS || config->flow_control == TLK_UART_FLOW_RTS_CTS)
    {
        // uart_rts_config
        tlk_uart_reg(uart_num).control2_bit.rts_manual_en = 0; // auto_mode_en
        tlk_uart_reg(uart_num).control2_bit.rts_pol       = config->flow_polarity;
        tlk_uart_reg(uart_num).control2_bit.rts_trig_lvl =
            CONFIG_TLK_RTS_TRIG_LEVEL; // uart_rts_trig_level_auto_mode

        tlk_uart_reg(uart_num).control2_bit.rts_en = 1; // uart_set_rts_en // enable RTS function

        if (config->flow_control == TLK_UART_FLOW_RTS)
        {
            tlk_uart_reg(uart_num).control1_bit.cts_en =
                0; // uart_set_cts_dis //disable CTS function
        }
    }

    if (config->flow_control == TLK_UART_FLOW_CTS || config->flow_control == TLK_UART_FLOW_RTS_CTS)
    {
        // uart_cts_config
        tlk_uart_reg(uart_num).control1_bit.cts_pol = config->flow_polarity;
        tlk_uart_reg(uart_num).control1_bit.cts_en  = 1; // uart_set_cts_en // enable CTS function

        if (config->flow_control == TLK_UART_FLOW_CTS)
        {
            tlk_uart_reg(uart_num).control2_bit.rts_en =
                0; // uart_set_rts_dis //disable RTS function
        }
    }

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_PM_DEVICE)
    TLK_UART_GET_RETENTION(uart_num)

    if (uart_retention != NULL)
    {
        uart_retention->flow_control  = config->flow_control;
        uart_retention->flow_polarity = config->flow_polarity;
    }
#endif

    tlk_uart_configure_flow_control_pinmux(uart_num, config);
}

void tlk_uart_configure_flow_control_pinmux(enum tlk_uart_module                 uart_num,
                                            struct tlk_uart_flow_control_config* config)
{
    TLK_UART_GET_INSTANCE(uart_num)

    if (config->rts_port_pin.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_set_mux(
            config->rts_port_pin.port, config->rts_port_pin.pin, uart_instance->rts_pin_function);
        tlk_gpio_disable(config->rts_port_pin.port, config->rts_port_pin.pin);
    }

    if (config->cts_port_pin.pin != TLK_GPIO_PIN_NONE)
    {
        tlk_gpio_configure(
            config->cts_port_pin.port, config->cts_port_pin.pin, TLK_GPIO_INPUT_PULL_UP);
        tlk_gpio_set_mux(
            config->cts_port_pin.port, config->cts_port_pin.pin, uart_instance->cts_pin_function);
        tlk_gpio_disable(config->cts_port_pin.port, config->cts_port_pin.pin);
    }

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_PM_DEVICE)
    TLK_UART_GET_RETENTION(uart_num)

    if (uart_retention != NULL)
    {
        uart_retention->rts_port_pin = config->rts_port_pin;
        uart_retention->cts_port_pin = config->cts_port_pin;
    }
#endif
}

#endif
/* ==== DRIVER APIs ==== */

/* ==== INTERRUPT HANDLER ==== */
#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_IRQ)
_tlk_attribute_ram_code_sec_ void tlk_uart_handle_irq(enum tlk_uart_module uart_num)
{
    TLK_UART_GET_INSTANCE(uart_num)

    /* TX handling */
    if (uart_instance->tx_mode == TLK_UART_XFER_IRQ)
    {
        if (tlk_uart_get_irq_status(uart_num, TLK_UART_IRQ_TXFIFO))
        {
            uint8_t fifo_cnt = MIN((uint32_t) (8 - tlk_uart_get_tx_fifo_count(uart_num)),
                                   (uart_instance->tx_buff_size - uart_instance->tx_cnt));

            for (uint8_t i = 0; i < fifo_cnt; i++)
            {
                tlk_uart_send_byte(uart_num, uart_instance->tx_buff[uart_instance->tx_cnt++]);
            }

            if (uart_instance->tx_cnt >= uart_instance->tx_buff_size)
            {
                tlk_uart_disable_irq(uart_num, TLK_UART_IRQ_TXFIFO);
            }
        }

        if (tlk_uart_get_irq_status(uart_num, TLK_UART_IRQ_TXDONE))
        {
            tlk_uart_disable_irq(uart_num, TLK_UART_IRQ_TXDONE | TLK_UART_IRQ_TXFIFO);
            tlk_uart_clr_irq_status(uart_num, TLK_UART_IRQ_TXDONE);
            uart_instance->waiting_for_tx = 0;

            if (uart_instance->tx_handler)
                uart_instance->tx_handler();
        }
    }

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_DMA)
    else if (uart_instance->tx_mode == TLK_UART_XFER_DMA)
    {
        if (tlk_uart_get_irq_status(uart_num, TLK_UART_IRQ_TXDONE))
        {
            tlk_uart_disable_irq(uart_num, TLK_UART_IRQ_TXDONE);
            tlk_uart_clr_irq_status(uart_num, TLK_UART_IRQ_TXDONE);
            uart_instance->waiting_for_tx = 0;

            if (uart_instance->tx_handler)
                uart_instance->tx_handler();
        }
    }
#endif
    /* TX handling */

    /* RX handling */
    if (tlk_uart_get_irq_status(uart_num, TLK_UART_IRQ_RXERR))
    {
        /* Clear software pointer.
         * It will clear rx_fifo, clear hardware pointer and rx_err_irq, rx_buff_irq, so it won't
         * enter rx_buff_irq interrupt. */
        tlk_uart_disable_irq(uart_num,
                             TLK_UART_IRQ_RXDONE | TLK_UART_IRQ_RXFIFO | TLK_UART_IRQ_RXERR);
        tlk_uart_clr_irq_status(uart_num, TLK_UART_IRQ_RXDONE | TLK_UART_IRQ_RXERR);

        uart_instance->waiting_for_rx = 0;
    }

    if (uart_instance->rx_mode == TLK_UART_XFER_IRQ)
    {
        if (tlk_uart_get_irq_status(uart_num, TLK_UART_IRQ_RXFIFO))
        {
            uint8_t fifo_cnt = MIN(tlk_uart_get_rx_fifo_count(uart_num),
                                   uart_instance->rx_buff_size - uart_instance->rx_cnt);

            for (uint8_t i = 0; i < fifo_cnt; i++)
            {
                uart_instance->rx_buff[uart_instance->rx_cnt++] = tlk_uart_read_byte(uart_num);
            }

            if (uart_instance->rx_cnt >= uart_instance->rx_buff_size)
            {
                tlk_uart_disable_irq(uart_num, TLK_UART_IRQ_RXFIFO);
            }
        }

        if (tlk_uart_get_irq_status(uart_num, TLK_UART_IRQ_RXDONE))
        {
            uint8_t fifo_tail = MIN(tlk_uart_get_rx_fifo_count(uart_num),
                                    uart_instance->rx_buff_size - uart_instance->rx_cnt);

            for (uint8_t i = 0; i < fifo_tail; i++)
            {
                uart_instance->rx_buff[uart_instance->rx_cnt++] = tlk_uart_read_byte(uart_num);
            }

            tlk_uart_disable_irq(uart_num,
                                 TLK_UART_IRQ_RXDONE | TLK_UART_IRQ_RXFIFO | TLK_UART_IRQ_RXERR);
            tlk_uart_clr_irq_status(uart_num, TLK_UART_IRQ_RXDONE | TLK_UART_IRQ_RXERR);
            uart_instance->waiting_for_rx = 0;

            if (uart_instance->rx_handler)
                uart_instance->rx_handler(uart_instance->rx_cnt);
        }
    }

#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_DMA)
    else if (uart_instance->rx_mode == TLK_UART_XFER_DMA)
    {
        if (uart_instance->rx_dma_mode == TLK_UART_RX_DMA_FIXED)
        {
            if (tlk_uart_get_irq_status(uart_num, TLK_UART_IRQ_RXDONE))
            {
                uart_instance->rx_cnt =
                    (uart_instance->rx_buff_size / 4 -
                     tlk_dma_reg.chn[uart_instance->rx_dma_chn].size_bit.tran_size) *
                    4;
                uart_instance->rx_cnt -= (8 - (tlk_uart_reg(uart_num).status_bit.rcnt)) % 4;

                tlk_uart_disable_irq(uart_num, TLK_UART_IRQ_RXDONE | TLK_UART_IRQ_RXERR);
                tlk_uart_clr_irq_status(uart_num, TLK_UART_IRQ_RXDONE | TLK_UART_IRQ_RXERR);
                uart_instance->waiting_for_rx = 0;

                if (uart_instance->rx_handler)
                    uart_instance->rx_handler(uart_instance->rx_cnt);
            }
        }

        else
        {
            if (tlk_uart_get_irq_status(uart_num, TLK_UART_IRQ_RXFIFO))
            {
                if (uart_instance->rx_cnt < uart_instance->rx_buff_size)
                {
                    uart_instance->rx_cnt =
                        (uart_instance->rx_buff_size / 4 -
                         tlk_dma_reg.chn[uart_instance->rx_dma_chn].size_bit.tran_size) *
                        4;

                    uint8_t fifo_tail = MIN(tlk_uart_get_rx_fifo_count(uart_num),
                                            uart_instance->rx_buff_size - uart_instance->rx_cnt);

                    for (uint8_t i = 0; i < fifo_tail; i++)
                    {
                        uart_instance->rx_buff[uart_instance->rx_cnt++] =
                            tlk_uart_read_byte(uart_num);
                    }
                }

                tlk_uart_disable_irq(uart_num, TLK_UART_IRQ_RXFIFO);
            }

            if (tlk_uart_get_irq_status(uart_num, TLK_UART_IRQ_RXDONE))
            {
                if (uart_instance->rx_cnt < uart_instance->rx_buff_size)
                {
                    uart_instance->rx_cnt =
                        (uart_instance->rx_buff_size / 4 -
                         tlk_dma_reg.chn[uart_instance->rx_dma_chn].size_bit.tran_size) *
                        4;

                    if (tlk_uart_get_rx_fifo_count(uart_num))
                    {
                        uint8_t fifo_tail =
                            MIN(tlk_uart_get_rx_fifo_count(uart_num),
                                uart_instance->rx_buff_size - uart_instance->rx_cnt);

                        for (uint8_t i = 0; i < fifo_tail; i++)
                        {
                            uart_instance->rx_buff[uart_instance->rx_cnt++] =
                                tlk_uart_read_byte(uart_num);
                        }
                    }
                    else
                    {
                        uart_instance->rx_cnt -= (8 - (tlk_uart_reg(uart_num).status_bit.rcnt)) % 4;
                    }
                }

                tlk_uart_disable_irq(uart_num, TLK_UART_IRQ_RXDONE | TLK_UART_IRQ_RXERR);
                tlk_uart_clr_irq_status(uart_num, TLK_UART_IRQ_RXDONE | TLK_UART_IRQ_RXERR);
                uart_instance->waiting_for_rx = 0;

                if (uart_instance->rx_handler)
                    uart_instance->rx_handler(uart_instance->rx_cnt);
            }
        }
    }
#endif

    /* RX handling */
}

#define TLK_UART_DECLARE_IRQ_HANDLER(num)                                                          \
    _tlk_attribute_ram_code_sec_ void tlk_uart##num##_irq_handler(void)                            \
    {                                                                                              \
        tlk_uart_handle_irq(TLK_UART##num);                                                        \
    }                                                                                              \
    TLK_PLIC_ISR_REGISTER(tlk_uart##num##_irq_handler, UNISDK_PLIC_IRQ_NUM_UART##num)

#define TLK_UART_DECLARE_IRQ_HANDLER_IF_ENABLED(num)                                               \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_USED_IRQ, (TLK_UART_DECLARE_IRQ_HANDLER(num)))

TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_DECLARE_IRQ_HANDLER_IF_ENABLED)
#endif
/* ==== INTERRUPT HANDLER ==== */

/* ==== AUTO CONFIG ==== */
#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_AUTO_CONFIG)
#define TLK_UART_DECLARE_AUTO_CONFIG(num)                                                          \
    __attribute__((used, noinline)) void tlk_uart##num##_auto_config(void)                         \
    {                                                                                              \
        struct tlk_uart_config config = {.baudrate       = CONFIG_TLK_UART##num##_BAUDRATE,        \
                                         .parity         = CONFIG_TLK_UART##num##_PARITY,          \
                                         .stop_bit       = CONFIG_TLK_UART##num##_STOP_BIT,        \
                                         .rx_timeout_mul = TLK_UART_RX_TIMEOUT_MUL_2,              \
                                         .tx_port_pin =                                            \
                                             (struct tlk_gpio_port_pin){                           \
                                                 .port = PINMUX_UART##num##_TX_PORT,               \
                                                 .pin  = PINMUX_UART##num##_TX_PIN,                \
                                             },                                                    \
                                         .rx_port_pin = (struct tlk_gpio_port_pin){                \
                                             .port = PINMUX_UART##num##_RX_PORT,                   \
                                             .pin  = PINMUX_UART##num##_RX_PIN,                    \
                                         }};                                                       \
        tlk_uart_configure(TLK_UART##num, &config);                                                \
                                                                                                   \
        TLK_IF_ENABLED(                                                                            \
            CONFIG_TLK_UART##num##_FLOW_CONTROL,                                                   \
            (struct tlk_uart_flow_control_config flow_control_config =                             \
                 {.flow_control  = CONFIG_TLK_UART##num##_FLOW_CONTROL_TYPE,                       \
                  .flow_polarity = CONFIG_TLK_UART##num##_FLOW_POLARITY,                           \
                  .rts_port_pin  = TLK_IF_ENABLED_ELSE(CONFIG_TLK_UART##num##_FLOW_CONTROL_RTS_ON, \
                                                       ((struct tlk_gpio_port_pin){                \
                                                           .port = PINMUX_UART##num##_RTS_PORT,    \
                                                           .pin  = PINMUX_UART##num##_RTS_PIN,     \
                                                       }),                                         \
                                                       ((struct tlk_gpio_port_pin){})),            \
                  .cts_port_pin  = TLK_IF_ENABLED_ELSE(CONFIG_TLK_UART##num##_FLOW_CONTROL_CTS_ON, \
                                                       ((struct tlk_gpio_port_pin){                \
                                                           .port = PINMUX_UART##num##_CTS_PORT,    \
                                                           .pin  = PINMUX_UART##num##_CTS_PIN,     \
                                                       }),                                         \
                                                       ((struct tlk_gpio_port_pin){}))};           \
             tlk_uart_configure_flow_control(TLK_UART##num, &flow_control_config);))               \
    }                                                                                              \
    TLK_REGISTER_PRE_INIT(tlk_uart##num##_auto_config, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_LOW)

#define TLK_UART_DECLARE_AUTO_CONFIG_IF_ENABLED(num)                                               \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_AUTO_CONFIG, (TLK_UART_DECLARE_AUTO_CONFIG(num)))

TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_DECLARE_AUTO_CONFIG_IF_ENABLED)
#endif
/* ==== AUTO CONFIG ==== */

/* ==== DMA HELPERS ==== */
#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_DMA)

#define TLK_UART_DECLARE_RX_DMA_HANDLER(num)                                                       \
    __attribute__((used, noinline)) void tlk_uart##num##_rx_dma_irq_handler(                       \
        enum tlk_dma_irq_type irq_type)                                                            \
    {                                                                                              \
        if (irq_type == TLK_DMA_IRQ_TC)                                                            \
        {                                                                                          \
            tlk_uart_enable_irq(num, TLK_UART_IRQ_RXFIFO);                                         \
        }                                                                                          \
    }

#define TLK_UART_DECLARE_RX_DMA_HANDLER_IF_ENABLED(num)                                            \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_RX_DMA_WITH_TAIL, (TLK_UART_DECLARE_RX_DMA_HANDLER(num)))

TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_DECLARE_RX_DMA_HANDLER_IF_ENABLED)

#define TLK_UART_DECLARE_REQUEST_DMA_CHN(num)                                                      \
    __attribute__((used, noinline)) void tlk_uart##num##_request_dma_chn(void)                     \
    {                                                                                              \
        if (tlk_uart##num##_instance.tx_mode == TLK_UART_XFER_DMA)                                 \
        {                                                                                          \
            tlk_uart##num##_instance.tx_dma_chn = tlk_dma_request_chn();                           \
        }                                                                                          \
        if (tlk_uart##num##_instance.rx_mode == TLK_UART_XFER_DMA)                                 \
        {                                                                                          \
            tlk_uart##num##_instance.rx_dma_chn = tlk_dma_request_chn();                           \
                                                                                                   \
            TLK_IF_ENABLED(CONFIG_TLK_UART##num##_RX_DMA_WITH_TAIL, ({                             \
                               if (tlk_uart##num##_instance.rx_dma_mode ==                         \
                                   TLK_UART_RX_DMA_WITH_TAIL)                                      \
                               {                                                                   \
                                   tlk_dma_add_callback(tlk_uart##num##_instance.rx_dma_chn,       \
                                                        tlk_uart##num##_rx_dma_irq_handler);       \
                               }                                                                   \
                           }));                                                                    \
        }                                                                                          \
    }                                                                                              \
    TLK_REGISTER_PRE_INIT(                                                                         \
        tlk_uart##num##_request_dma_chn, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)

#define TLK_UART_DECLARE_REQUEST_DMA_CHN_IF_ENABLED(num)                                           \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_USED_DMA, (TLK_UART_DECLARE_REQUEST_DMA_CHN(num)))

TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_DECLARE_REQUEST_DMA_CHN_IF_ENABLED)
#endif
/* ==== DMA HELPERS ==== */

/* ==== PM DEVICE ==== */
#if TLK_IS_ENABLED(CONFIG_TLK_UART_USED_PM_DEVICE)
#define TLK_UART_DECLARE_RESTORE_CONTEXT(num)                                                      \
    static void tlk_uart##num##_restore_context_suspend(void)                                      \
    {                                                                                              \
        tlk_uart##num##_instance.tx_byte_index = 0;                                                \
        struct tlk_uart_config config          = {                                                 \
            .tx_port_pin = tlk_uart##num##_retention.tx_port_pin,                                  \
            .rx_port_pin = tlk_uart##num##_retention.rx_port_pin,                                  \
        };                                                                                         \
        tlk_uart_configure_pinmux(TLK_UART##num, &config);                                         \
        TLK_IF_ENABLED(                                                                            \
            CONFIG_TLK_UART##num##_FLOW_CONTROL,                                                   \
            (struct tlk_uart_flow_control_config flow_control_config =                             \
                 {                                                                                 \
                     .rts_port_pin = tlk_uart##num##_retention.rts_port_pin,                       \
                     .cts_port_pin = tlk_uart##num##_retention.cts_port_pin,                       \
                 };                                                                                \
             tlk_uart_configure_flow_control_pinmux(TLK_UART##num, &flow_control_config);))        \
    }                                                                                              \
                                                                                                   \
    static void tlk_uart##num##_restore_context(void)                                              \
    {                                                                                              \
        struct tlk_uart_config config = {                                                          \
            .baudrate       = tlk_uart##num##_retention.baudrate,                                  \
            .parity         = tlk_uart##num##_retention.parity,                                    \
            .stop_bit       = tlk_uart##num##_retention.stop_bit,                                  \
            .rx_timeout_mul = tlk_uart##num##_retention.rx_timeout_mul,                            \
            .tx_port_pin    = tlk_uart##num##_retention.tx_port_pin,                               \
            .rx_port_pin    = tlk_uart##num##_retention.rx_port_pin,                               \
        };                                                                                         \
        tlk_uart_configure(TLK_UART##num, &config);                                                \
        TLK_IF_ENABLED(CONFIG_TLK_UART##num##_FLOW_CONTROL,                                        \
                       (struct tlk_uart_flow_control_config flow_control_config =                  \
                            {                                                                      \
                                .flow_control  = tlk_uart##num##_retention.flow_control,           \
                                .flow_polarity = tlk_uart##num##_retention.flow_polarity,          \
                                .rts_port_pin  = tlk_uart##num##_retention.rts_port_pin,           \
                                .cts_port_pin  = tlk_uart##num##_retention.cts_port_pin,           \
                            };                                                                     \
                        tlk_uart_configure_flow_control(TLK_UART##num, &flow_control_config);))    \
    }                                                                                              \
                                                                                                   \
    static void tlk_uart##num##_before_sleep(void)                                                 \
    {                                                                                              \
        tlk_gpio_configure(tlk_uart##num##_retention.tx_port_pin.port,                             \
                           tlk_uart##num##_retention.tx_port_pin.pin,                              \
                           TLK_GPIO_INPUT_PULL_UP_10K);                                            \
        tlk_gpio_configure(tlk_uart##num##_retention.rx_port_pin.port,                             \
                           tlk_uart##num##_retention.rx_port_pin.pin,                              \
                           TLK_GPIO_INPUT_PULL_UP_10K);                                            \
        tlk_gpio_disable(tlk_uart##num##_retention.tx_port_pin.port,                               \
                         tlk_uart##num##_retention.tx_port_pin.pin);                               \
        tlk_gpio_disable(tlk_uart##num##_retention.rx_port_pin.port,                               \
                         tlk_uart##num##_retention.rx_port_pin.pin);                               \
    }                                                                                              \
                                                                                                   \
    TLK_REGISTER_AFTER_SLEEP(                                                                      \
        tlk_uart##num##_restore_context, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)          \
    TLK_REGISTER_AFTER_SUSPEND(                                                                    \
        tlk_uart##num##_restore_context_suspend, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)  \
    TLK_REGISTER_BEFORE_SLEEP(                                                                     \
        tlk_uart##num##_before_sleep, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)             \
    TLK_REGISTER_BEFORE_SUSPEND(                                                                   \
        tlk_uart##num##_before_sleep, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_NORMAL)

#define TLK_UART_DECLARE_RESTORE_CONTEXT_IF_ENABLED(num)                                           \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_PM_DEVICE, (TLK_UART_DECLARE_RESTORE_CONTEXT(num)))

TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_DECLARE_RESTORE_CONTEXT_IF_ENABLED)
#endif
/* ==== PM DEVICE ==== */

/* ==== PREVENT SLEEP ==== */
#define TLK_UART_DECLARE_PREVENT_SLEEP(num)                                                        \
    static bool tlk_uart##num##_prevent_sleep(void)                                                \
    {                                                                                              \
        return tlk_uart##num##_instance.waiting_for_tx || tlk_uart##num##_instance.waiting_for_rx; \
    }                                                                                              \
                                                                                                   \
    TLK_REGISTER_PREVENT_SUSPEND(tlk_uart##num##_prevent_sleep)                                    \
    TLK_REGISTER_PREVENT_SLEEP(tlk_uart##num##_prevent_sleep)

#define TLK_UART_DECLARE_PREVENT_SLEEP_IF_ENABLED(num)                                             \
    TLK_IF_ENABLED(CONFIG_TLK_UART##num##_PREVENT_SLEEP, (TLK_UART_DECLARE_PREVENT_SLEEP(num)))

TLK_FOR_CALL(UNISDK_UART_COUNT, TLK_UART_DECLARE_PREVENT_SLEEP_IF_ENABLED)
/* ==== PREVENT SLEEP ==== */

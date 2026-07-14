#ifndef TLK_TLK_UART_REGISTERS_H_
#define TLK_TLK_UART_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_uart_clock_div_e {
    TLK_UART_CLOCK_DIV_DIV_EN = TLK_BIT(15),
} tlk_uart_clock_div_e;

typedef enum tlk_uart_control0_e {
    TLK_UART_CONTROL0_AUTO_RXCLR_EN    = TLK_BIT(4),
    TLK_UART_CONTROL0_NDMA_RXDONE_EN   = TLK_BIT(5),
    TLK_UART_CONTROL0_RXTIMEOUT_RTS_EN = TLK_BIT(6),
    TLK_UART_CONTROL0_ISO7816_EN       = TLK_BIT(7),
} tlk_uart_control0_e;

typedef enum tlk_uart_control1_e {
    TLK_UART_CONTROL1_CTS_POL         = TLK_BIT(0),
    TLK_UART_CONTROL1_CTS_EN          = TLK_BIT(1),
    TLK_UART_CONTROL1_PARITY_EN       = TLK_BIT(2),
    TLK_UART_CONTROL1_PARITY_POLARITY = TLK_BIT(3),
    TLK_UART_CONTROL1_TX_RX_POL       = TLK_BIT(6),
    TLK_UART_CONTROL1_LOOPBACK_EN     = TLK_BIT(7),
} tlk_uart_control1_e;

typedef enum tlk_uart_control2_e {
    TLK_UART_CONTROL2_RTS_POL        = TLK_BIT(4),
    TLK_UART_CONTROL2_RTS_MANUAL_VAL = TLK_BIT(5),
    TLK_UART_CONTROL2_RTS_MANUAL_EN  = TLK_BIT(6),
    TLK_UART_CONTROL2_RTS_EN         = TLK_BIT(7),
} tlk_uart_control2_e;

typedef enum tlk_uart_rx_timeout1_e {
    TLK_UART_RX_TIMEOUT1_RX_BUF_IRQ_EN = TLK_BIT(2),
    TLK_UART_RX_TIMEOUT1_TX_BUF_IRQ_EN = TLK_BIT(3),
    TLK_UART_RX_TIMEOUT1_RXDONE_IRQ_EN = TLK_BIT(4),
    TLK_UART_RX_TIMEOUT1_TXDONE_EN     = TLK_BIT(5),
    TLK_UART_RX_TIMEOUT1_RX_ERR_EN     = TLK_BIT(6),
} tlk_uart_rx_timeout1_e;

typedef enum tlk_uart_status_e {
    TLK_UART_STATUS_IRQ    = TLK_BIT(3),
    TLK_UART_STATUS_RXDONE = TLK_BIT(7),
} tlk_uart_status_e;

typedef enum tlk_uart_irq_status_e {
    TLK_UART_IRQ_STATUS_RX_BUF_IRQ = TLK_BIT(2),
    TLK_UART_IRQ_STATUS_TX_BUF_IRQ = TLK_BIT(3),
    TLK_UART_IRQ_STATUS_RXDONE_IRQ = TLK_BIT(4),
    TLK_UART_IRQ_STATUS_TXDONE     = TLK_BIT(5),
    TLK_UART_IRQ_STATUS_RX_ERR     = TLK_BIT(6),
    TLK_UART_IRQ_STATUS_TIMEOUT    = TLK_BIT(7),
} tlk_uart_irq_status_e;

typedef enum tlk_uart_control4_e {
    TLK_UART_CONTROL4_RXDONE_RTS_EN = TLK_BIT(0),
} tlk_uart_control4_e;

typedef struct __attribute__((packed)) {
    uint8_t data_buf[4]; // address: 0x80140080, offset: 0x0
    union {
        struct {
            unsigned div_l : 8;
            unsigned div_h : 7;
            unsigned div_en : 1;
        } __attribute__((packed)) clock_div_bit;
        struct {
            enum tlk_uart_clock_div_e __attribute__((packed)) clock_div : 16;
        }; // address: 0x80140084, offset: 0x4
    };
    union {
        struct {
            unsigned bpwc : 4;
            unsigned auto_rxclr_en : 1;
            unsigned ndma_rxdone_en : 1;
            unsigned rxtimeout_rts_en : 1;
            unsigned iso7816_en : 1;
        } __attribute__((packed)) control0_bit;
        struct {
            enum tlk_uart_control0_e __attribute__((packed)) control0 : 8;
        }; // address: 0x80140086, offset: 0x6
    };
    union {
        struct {
            unsigned cts_pol : 1;
            unsigned cts_en : 1;
            unsigned parity_en : 1;
            unsigned parity_polarity : 1;
            unsigned stop_bit : 2;
            unsigned tx_rx_pol : 1;
            unsigned loopback_en : 1;
        } __attribute__((packed)) control1_bit;
        struct {
            enum tlk_uart_control1_e __attribute__((packed)) control1 : 8;
        }; // address: 0x80140087, offset: 0x7
    };
    union {
        struct {
            unsigned rts_trig_lvl : 4;
            unsigned rts_pol : 1;
            unsigned rts_manual_val : 1;
            unsigned rts_manual_en : 1;
            unsigned rts_en : 1;
        } __attribute__((packed)) control2_bit;
        struct {
            enum tlk_uart_control2_e __attribute__((packed)) control2 : 8;
        }; // address: 0x80140088, offset: 0x8
    };
    union {
        struct {
            unsigned rx_irq_trig_lvl : 4;
            unsigned tx_irq_trig_lvl : 4;
        } __attribute__((packed)) control3_bit;
        uint8_t control3; // address: 0x80140089, offset: 0x9
    };
    uint8_t rx_timeout0; // address: 0x8014008a, offset: 0xa
    union {
        struct {
            unsigned scale : 2;
            unsigned rx_buf_irq_en : 1;
            unsigned tx_buf_irq_en : 1;
            unsigned rxdone_irq_en : 1;
            unsigned txdone_en : 1;
            unsigned rx_err_en : 1;
            unsigned reserved : 1;
        } __attribute__((packed)) rx_timeout1_bit;
        struct {
            enum tlk_uart_rx_timeout1_e __attribute__((packed)) rx_timeout1 : 8;
        }; // address: 0x8014008b, offset: 0xb
    };
    union {
        struct {
            unsigned rx_buf_cnt : 4;
            unsigned tx_buf_cnt : 4;
        } __attribute__((packed)) buf_cnt_bit;
        uint8_t buf_cnt; // address: 0x8014008c, offset: 0xc
    };
    union {
        struct {
            unsigned rcnt : 3;
            unsigned irq : 1;
            unsigned wbcnt : 3;
            unsigned rxdone : 1;
        } __attribute__((packed)) status_bit;
        struct {
            enum tlk_uart_status_e __attribute__((packed)) status : 8;
        }; // address: 0x8014008d, offset: 0xd
    };
    union {
        struct {
            unsigned rx_rem_cnt_d : 2;
            unsigned rx_buf_irq : 1;
            unsigned tx_buf_irq : 1;
            unsigned rxdone_irq : 1;
            unsigned txdone : 1;
            unsigned rx_err : 1;
            unsigned timeout : 1;
        } __attribute__((packed)) irq_status_bit;
        struct {
            enum tlk_uart_irq_status_e __attribute__((packed)) irq_status : 8;
        }; // address: 0x8014008e, offset: 0xe
    };
    union {
        struct {
            unsigned tx_state : 3;
            unsigned reserved : 1;
            unsigned rx_state : 4;
        } __attribute__((packed)) state_bit;
        uint8_t state; // address: 0x8014008f, offset: 0xf
    };
    union {
        struct {
            unsigned rxdone_rts_en : 1;
            unsigned reserved : 7;
        } __attribute__((packed)) control4_bit;
        struct {
            enum tlk_uart_control4_e __attribute__((packed)) control4 : 8;
        }; // address: 0x80140090, offset: 0x10
    };
} tlk_uart_reg_t;

#define TLK_UART_BASE_ADDR (0x80140080U)
#define TLK_UART0_ADDR (0x80140080U)
#define TLK_UART1_ADDR (0x801400c0U)
#define tlk_uart_reg(i) (*(volatile tlk_uart_reg_t *) ( \
    ((i) == 0) ? TLK_UART0_ADDR : \
    ((i) == 1) ? TLK_UART1_ADDR : \
    0 ))

#endif
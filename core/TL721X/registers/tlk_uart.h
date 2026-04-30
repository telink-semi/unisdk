#ifndef TLK_UART_REGISTERS_H_
#define TLK_UART_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t data_buf[4]; // address: 0x80140080, offset: 0x0
    union {
        struct {
            unsigned div_l : 8;
            unsigned div_h : 7;
            unsigned div_en : 1;
        } __attribute__((packed)) clock_div_bit;
        uint16_t clock_div; // address: 0x80140084, offset: 0x4
    };
    union {
        struct {
            unsigned bpwc : 4;
            unsigned auto_rxclr_en : 1;
            unsigned ndma_rxdone_en : 1;
            unsigned rxtimeout_rts_en : 1;
            unsigned iso7816_en : 1;
        } __attribute__((packed)) control0_bit;
        uint8_t control0; // address: 0x80140086, offset: 0x6
    };
    union {
        struct {
            unsigned cts_pol : 1;
            unsigned cts_en : 1;
            unsigned parity_en : 1;
            unsigned parity_polarity : 1;
            unsigned stop_bit : 2;
            unsigned ttl_en : 1;
            unsigned loopback_o : 1;
        } __attribute__((packed)) control1_bit;
        uint8_t control1; // address: 0x80140087, offset: 0x7
    };
    union {
        struct {
            unsigned rts_trig_lvl : 4;
            unsigned rts_pol : 1;
            unsigned rts_manual_val : 1;
            unsigned rts_manual_en : 1;
            unsigned rts_en : 1;
        } __attribute__((packed)) control2_bit;
        uint8_t control2; // address: 0x80140088, offset: 0x8
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
            unsigned txdone_irq_en : 1;
            unsigned rx_err_irq_en : 1;
            unsigned reserved : 1;
        } __attribute__((packed)) rx_timeout1_bit;
        uint8_t rx_timeout1; // address: 0x8014008b, offset: 0xb
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
        uint8_t status; // address: 0x8014008d, offset: 0xd
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
        uint8_t irq_status; // address: 0x8014008e, offset: 0xe
    };
    union {
        struct {
            unsigned tx_state : 3;
            unsigned rx_full : 1;
            unsigned rx_state : 4;
        } __attribute__((packed)) state_bit;
        uint8_t state; // address: 0x8014008f, offset: 0xf
    };
    union {
        struct {
            unsigned rxdone_rts_en : 1;
            unsigned timeout_en : 1;
            unsigned rx_timeout_reload_sel : 1;
            unsigned rts_stop_timeout_en : 1;
            unsigned pem_event_en : 1;
            unsigned reserved0 : 1;
            unsigned uart_en : 1;
            unsigned reserved1 : 1;
        } __attribute__((packed)) control4_bit;
        uint8_t control4; // address: 0x80140090, offset: 0x10
    };
    uint8_t rxtimeout_o_exp; // address: 0x80140091, offset: 0x11
    union {
        struct {
            unsigned pem_task_en : 5;
            unsigned reserved : 3;
        } __attribute__((packed)) pem_control_bit;
        uint8_t pem_control; // address: 0x80140092, offset: 0x12
    };
} tlk_uart_reg_t;

#define TLK_UART_BASE_ADDR (0x80140080U)
#define TLK_UART0_ADDR (0x80140080U)
#define TLK_UART1_ADDR (0x801400c0U)
#define TLK_UART2_ADDR (0x801402c0U)
#define tlk_uart_reg(i) (*(volatile tlk_uart_reg_t *) ( \
    ((i) == 0) ? TLK_UART0_ADDR : \
    ((i) == 1) ? TLK_UART1_ADDR : \
    ((i) == 2) ? TLK_UART2_ADDR : \
    0 ))

#endif


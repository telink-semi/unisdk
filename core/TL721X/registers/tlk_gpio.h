#ifndef TLK_GPIO_REGISTERS_H_
#define TLK_GPIO_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    struct {
        uint8_t input; // address: 0x80140c00, offset: 0x0
        uint8_t input_en; // address: 0x80140c01, offset: 0x1
        uint8_t output_en; // address: 0x80140c02, offset: 0x2
        uint8_t reserved0; // address: 0x80140c03, offset: 0x3
        uint8_t polarity; // address: 0x80140c04, offset: 0x4
        uint8_t drive_strength; // address: 0x80140c05, offset: 0x5
        uint8_t function; // address: 0x80140c06, offset: 0x6
        uint8_t irq_en; // address: 0x80140c07, offset: 0x7
        uint8_t port_m0_irq_en; // address: 0x80140c08, offset: 0x8
        uint8_t port_m1_irq_en; // address: 0x80140c09, offset: 0x9
        uint8_t pull_down; // address: 0x80140c0a, offset: 0xa
        uint8_t pull_up; // address: 0x80140c0b, offset: 0xb
        uint8_t output_set; // address: 0x80140c0c, offset: 0xc
        uint8_t output_clear; // address: 0x80140c0d, offset: 0xd
        uint8_t output_toggle; // address: 0x80140c0e, offset: 0xe
        uint8_t reserved; // address: 0x80140c0f, offset: 0xf
    } __attribute__((packed)) port_config[7]; // address: 0x80140c00, offset: 0x0
    uint8_t pin_func[6][8]; // address: 0x80140c70, offset: 0x70
    uint8_t reserved1[0x2]; // address: 0x80140ca0, offset: 0xa0
    union {
        struct {
            unsigned wakeup_irq_en : 1;
            unsigned gpio_irq_en : 1;
            unsigned mask_gpio_irq : 1;
            unsigned mask_gpio2risc0_irq : 1;
            unsigned mask_gpio2risc1_irq : 1;
            unsigned lvl_gpio_irq : 1;
            unsigned lvl_gpio2risc0 : 1;
            unsigned lvl_gpio2risc1 : 1;
        } __attribute__((packed)) irq_control_bit;
        uint8_t irq_control; // address: 0x80140ca2, offset: 0xa2
    };
    uint8_t reserved2; // address: 0x80140ca3, offset: 0xa3
    uint8_t group_irq_level; // address: 0x80140ca4, offset: 0xa4
    union {
        struct {
            unsigned value : 3;
            unsigned reserved : 5;
        } __attribute__((packed)) group_irq_select_bit;
        uint8_t group_irq_select; // address: 0x80140ca5, offset: 0xa5
    };
    uint8_t group_irq_mask; // address: 0x80140ca6, offset: 0xa6
    uint8_t reserved3; // address: 0x80140ca7, offset: 0xa7
    union {
        struct {
            unsigned gpio_irq : 1;
            unsigned gpio2risc0_irq : 1;
            unsigned gpio2risc1_irq : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) irq_status_bit;
        uint8_t irq_status; // address: 0x80140ca8, offset: 0xa8
    };
    uint8_t group_irq_status; // address: 0x80140ca9, offset: 0xa9
} tlk_gpio_reg_t;

#define TLK_GPIO_BASE_ADDR (0x80140c00U)
#define tlk_gpio_reg (*(volatile tlk_gpio_reg_t *) TLK_GPIO_BASE_ADDR)

#endif


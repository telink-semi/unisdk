#ifndef TLK_GPIO_REGISTERS_H_
#define TLK_GPIO_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    struct {
        uint8_t input; // address: 0x80140c00, offset: 0x0
        uint8_t input_en; // address: 0x80140c01, offset: 0x1
        uint8_t output_en; // address: 0x80140c02, offset: 0x2
        uint8_t polarity; // address: 0x80140c03, offset: 0x3
        uint8_t output_set; // address: 0x80140c04, offset: 0x4
        uint8_t output_clear; // address: 0x80140c05, offset: 0x5
        uint8_t function; // address: 0x80140c06, offset: 0x6
        uint8_t output_toggle; // address: 0x80140c07, offset: 0x7
        uint8_t irq_en[8]; // address: 0x80140c08, offset: 0x8
    } __attribute__((packed)) port_config[6]; // address: 0x80140c00, offset: 0x0
    uint8_t reserved0[0x10]; // address: 0x80140c60, offset: 0x60
    uint8_t pin_func[5][8]; // address: 0x80140c70, offset: 0x70
    uint8_t reserved1[0xa]; // address: 0x80140c98, offset: 0x98
    union {
        struct {
            unsigned wakeup_irq_en : 1;
            unsigned gpio_irq_en : 1;
            unsigned reserved0 : 2;
            unsigned pem_event_en : 1;
            unsigned reserved1 : 3;
        } __attribute__((packed)) irq_control_bit;
        uint8_t irq_control; // address: 0x80140ca2, offset: 0xa2
    };
    uint8_t reserved2; // address: 0x80140ca3, offset: 0xa3
    uint8_t irq_level_mode; // address: 0x80140ca4, offset: 0xa4
    uint8_t irq_status; // address: 0x80140ca5, offset: 0xa5
    uint8_t irq_mask; // address: 0x80140ca6, offset: 0xa6
} tlk_gpio_reg_t;

#define TLK_GPIO_BASE_ADDR (0x80140c00U)
#define tlk_gpio_reg (*(volatile tlk_gpio_reg_t *) TLK_GPIO_BASE_ADDR)

#endif


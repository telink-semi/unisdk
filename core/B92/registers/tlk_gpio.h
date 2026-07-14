#ifndef TLK_TLK_GPIO_REGISTERS_H_
#define TLK_TLK_GPIO_REGISTERS_H_

#include <stdint.h>
#include <common/include/tlk_bit.h>

typedef enum tlk_gpio_irq_control_e {
    TLK_GPIO_IRQ_CONTROL_WAKEUP_IRQ_EN       = TLK_BIT(0),
    TLK_GPIO_IRQ_CONTROL_GPIO_IRQ_EN         = TLK_BIT(1),
    TLK_GPIO_IRQ_CONTROL_MASK_GPIO_IRQ       = TLK_BIT(2),
    TLK_GPIO_IRQ_CONTROL_MASK_GPIO2RISC0_IRQ = TLK_BIT(3),
    TLK_GPIO_IRQ_CONTROL_MASK_GPIO2RISC1_IRQ = TLK_BIT(4),
    TLK_GPIO_IRQ_CONTROL_LVL_GPIO_IRQ        = TLK_BIT(5),
    TLK_GPIO_IRQ_CONTROL_LVL_GPIO2RISC0      = TLK_BIT(6),
    TLK_GPIO_IRQ_CONTROL_LVL_GPIO2RISC1      = TLK_BIT(7),
} tlk_gpio_irq_control_e;

typedef enum tlk_gpio_irq_status_e {
    TLK_GPIO_IRQ_STATUS_GPIO_IRQ       = TLK_BIT(0),
    TLK_GPIO_IRQ_STATUS_GPIO2RISC0_IRQ = TLK_BIT(1),
    TLK_GPIO_IRQ_STATUS_GPIO2RISC1_IRQ = TLK_BIT(2),
} tlk_gpio_irq_status_e;

typedef struct __attribute__((packed)) {
    struct {
        uint8_t input; // address: 0x80140300, offset: 0x0
        uint8_t input_en; // address: 0x80140301, offset: 0x1
        uint8_t output_en; // address: 0x80140302, offset: 0x2
        uint8_t output; // address: 0x80140303, offset: 0x3
        uint8_t polarity; // address: 0x80140304, offset: 0x4
        uint8_t drive_strength; // address: 0x80140305, offset: 0x5
        uint8_t function; // address: 0x80140306, offset: 0x6
        uint8_t irq_en; // address: 0x80140307, offset: 0x7
    } __attribute__((packed)) port_config[7]; // address: 0x80140300, offset: 0x0
    uint8_t port_m0_irq_en[7]; // address: 0x80140338, offset: 0x38
    uint8_t reserved0; // address: 0x8014033f, offset: 0x3f
    uint8_t port_m1_irq_en[7]; // address: 0x80140340, offset: 0x40
    uint8_t reserved1; // address: 0x80140347, offset: 0x47
    uint8_t pin_func[6][8]; // address: 0x80140348, offset: 0x48
    uint8_t reserved2[0x2]; // address: 0x80140378, offset: 0x78
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
        struct {
            enum tlk_gpio_irq_control_e __attribute__((packed)) irq_control : 8;
        }; // address: 0x8014037a, offset: 0x7a
    };
    union {
        struct {
            unsigned gpio_irq : 1;
            unsigned gpio2risc0_irq : 1;
            unsigned gpio2risc1_irq : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) irq_status_bit;
        struct {
            enum tlk_gpio_irq_status_e __attribute__((packed)) irq_status : 8;
        }; // address: 0x8014037b, offset: 0x7b
    };
    uint8_t group_irq_status; // address: 0x8014037c, offset: 0x7c
    uint8_t reserved3[0x19]; // address: 0x8014037d, offset: 0x7d
    union {
        struct {
            unsigned value : 3;
            unsigned reserved : 5;
        } __attribute__((packed)) group_irq_select_bit;
        uint8_t group_irq_select; // address: 0x80140396, offset: 0x96
    };
    uint8_t group_irq_mask; // address: 0x80140397, offset: 0x97
    uint8_t group_irq_level; // address: 0x80140398, offset: 0x98
} tlk_gpio_reg_t;

#define TLK_GPIO_BASE_ADDR (0x80140300U)
#define tlk_gpio_reg (*(volatile tlk_gpio_reg_t *) TLK_GPIO_BASE_ADDR)

#endif
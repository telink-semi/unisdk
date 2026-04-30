#ifndef TLK_ADC_REGISTERS_H_
#define TLK_ADC_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned p_input_pin : 4;
            unsigned n_input_pin : 4;
        } __attribute__((packed)) r_mux_bit[3];
        uint8_t r_mux[3]; // address: 0x801401c0, offset: 0x0
    };
    union {
        struct {
            unsigned m_tsamp : 4;
            unsigned l_tsamp : 4;
        } __attribute__((packed)) tsamp_bit;
        uint8_t tsamp; // address: 0x801401c3, offset: 0x3
    };
    union {
        struct {
            unsigned value : 4;
            unsigned reserved : 4;
        } __attribute__((packed)) r_tsamp_bit;
        uint8_t r_tsamp; // address: 0x801401c4, offset: 0x4
    };
    union {
        struct {
            unsigned m_vbat_div : 2;
            unsigned l_vbat_div : 2;
            unsigned r_vbat_div : 2;
            unsigned reserved : 2;
        } __attribute__((packed)) vbat_div_bit;
        uint8_t vbat_div; // address: 0x801401c5, offset: 0x5
    };
    union {
        struct {
            unsigned r_max_s : 4;
            unsigned sel_ai_scale : 2;
            unsigned sel_vref : 2;
        } __attribute__((packed)) channel_set_state_bit[3];
        uint8_t channel_set_state[3]; // address: 0x801401c6, offset: 0x6
    };
    uint8_t rng_set_state; // address: 0x801401c9, offset: 0x9
    union {
        struct {
            unsigned r_max_c : 10;
            unsigned reserved : 6;
        } __attribute__((packed)) capture_state_bit[3];
        uint16_t capture_state[3]; // address: 0x801401ca, offset: 0xa
    };
    uint16_t rng_capture_state; // address: 0x801401d0, offset: 0x10
    uint8_t reserved0[0x16]; // address: 0x801401d2, offset: 0x12
    union {
        struct {
            unsigned reserved0 : 4;
            unsigned scant_max : 3;
            unsigned reserved1 : 1;
        } __attribute__((packed)) config0_bit;
        uint8_t config0; // address: 0x801401e8, offset: 0x28
    };
    union {
        struct {
            unsigned clk_div : 4;
            unsigned reserved0 : 1;
            unsigned mode : 1;
            unsigned reserved1 : 2;
        } __attribute__((packed)) config1_bit;
        uint8_t config1; // address: 0x801401e9, offset: 0x29
    };
    union {
        struct {
            unsigned m_channel_en : 1;
            unsigned l_channel_en : 1;
            unsigned r_channel_en : 1;
            unsigned reserved : 1;
            unsigned rx_dma_en : 1;
            unsigned clk_en : 1;
            unsigned rx_interrupt_en : 1;
            unsigned trig_mode : 1;
        } __attribute__((packed)) config2_bit;
        uint8_t config2; // address: 0x801401ea, offset: 0x2a
    };
    union {
        struct {
            unsigned rxfifo_trig_num : 3;
            unsigned reserved : 1;
            unsigned buf_cnt : 4;
        } __attribute__((packed)) rxfifo_trig_num_bit;
        uint8_t rxfifo_trig_num; // address: 0x801401eb, offset: 0x2b
    };
    uint16_t rxfifo_data[2]; // address: 0x801401ec, offset: 0x2c
    union {
        struct {
            unsigned irq_rx_status : 1;
            unsigned fifo_clr : 1;
            unsigned trig_start : 1;
            unsigned reserved : 4;
            unsigned soft_start : 1;
        } __attribute__((packed)) soft_control_bit;
        uint8_t soft_control; // address: 0x801401f0, offset: 0x30
    };
    union {
        struct {
            unsigned pem_task_en : 1;
            unsigned pem_event0_en : 1;
            unsigned pem_event1_en : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) sample_times_bit;
        uint8_t sample_times; // address: 0x801401f1, offset: 0x31
    };
} tlk_adc_reg_t;

#define TLK_ADC_BASE_ADDR (0x801401c0U)
#define tlk_adc_reg (*(volatile tlk_adc_reg_t *) TLK_ADC_BASE_ADDR)

#endif


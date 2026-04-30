#ifndef TLK_OTP_REGISTERS_H_
#define TLK_OTP_REGISTERS_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    union {
        struct {
            unsigned reserved : 1;
            unsigned pce : 1;
            unsigned pprog : 1;
            unsigned pwe : 1;
            unsigned pas : 1;
            unsigned ptr : 1;
            unsigned ptc : 1;
            unsigned ecc_rdb : 1;
        } __attribute__((packed)) control0_bit;
        uint8_t control0; // address: 0x80140300, offset: 0x0
    };
    union {
        struct {
            unsigned ptm : 4;
            unsigned pldo : 1;
            unsigned pdstd : 1;
            unsigned reserved : 2;
        } __attribute__((packed)) control1_bit;
        uint8_t control1; // address: 0x80140301, offset: 0x1
    };
    union {
        struct {
            unsigned data_ecc : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) control2_bit;
        uint8_t control2; // address: 0x80140302, offset: 0x2
    };
    union {
        struct {
            unsigned reserved0 : 4;
            unsigned auto_inc : 1;
            unsigned man_mode : 1;
            unsigned man_pclk : 1;
            unsigned reserved1 : 1;
        } __attribute__((packed)) control3_bit;
        uint8_t control3; // address: 0x80140303, offset: 0x3
    };
    uint16_t pa; // address: 0x80140304, offset: 0x4
    union {
        struct {
            unsigned value : 6;
            unsigned reserved : 2;
        } __attribute__((packed)) paio_bit;
        uint8_t paio; // address: 0x80140306, offset: 0x6
    };
    union {
        struct {
            unsigned busy : 1;
            unsigned auto_pwup_trig : 1;
            unsigned keylock : 1;
            unsigned reserved : 5;
        } __attribute__((packed)) status_bit;
        uint8_t status; // address: 0x80140307, offset: 0x7
    };
    uint32_t write_data; // address: 0x80140308, offset: 0x8
    uint32_t read_data; // address: 0x8014030c, offset: 0xc
    union {
        struct {
            unsigned cap_edge : 5;
            unsigned reserved : 3;
        } __attribute__((packed)) t_control4_bit;
        uint8_t t_control4; // address: 0x80140310, offset: 0x10
    };
    union {
        struct {
            unsigned tim_config : 3;
            unsigned reserved : 5;
        } __attribute__((packed)) t_control5_bit;
        uint8_t t_control5; // address: 0x80140311, offset: 0x11
    };
    uint8_t reserved0[0x2]; // address: 0x80140312, offset: 0x12
    uint32_t auto_load_data[3]; // address: 0x80140314, offset: 0x14
} tlk_otp_reg_t;

#define TLK_OTP_BASE_ADDR (0x80140300U)
#define tlk_otp_reg (*(volatile tlk_otp_reg_t *) TLK_OTP_BASE_ADDR)

#endif


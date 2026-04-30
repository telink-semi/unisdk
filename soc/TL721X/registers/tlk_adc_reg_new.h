#ifndef ADC_REG_NEW_H__
#define ADC_REG_NEW_H__

// TODO: Integrate, Describe bflds

#include "tlk_soc.h"
#include "tlk_adc_reg.h"

#define REG_ADC_R_MUX(i)                REG_ADDR8(ADC_BASE_ADDR + (i))
#define REG_ADC_TSAMP                   REG_ADDR8(ADC_BASE_ADDR + 0x03)
#define REG_ADC_VBAT_DIV                REG_ADDR8(ADC_BASE_ADDR + 0x05)
#define REG_ADC_CHANNEL_SET_STATE(i)    REG_ADDR8(ADC_BASE_ADDR + 0x06 + (i))
#define REG_ADC_RNG_SET_STATE           REG_ADDR8(ADC_BASE_ADDR + 0x09)
#define REG_ADC_CAPTURE_STATE(i)        REG_ADDR16(ADC_BASE_ADDR + 0x0a + (i) * 0x02)
#define REG_ADC_RNG_CAPTURE_STATE       REG_ADDR16(ADC_BASE_ADDR + 0x10)
#define REG_ADC_CONFIG0                 REG_ADDR8(ADC_BASE_ADDR + 0x28)
#define REG_ADC_CONFIG1                 REG_ADDR8(ADC_BASE_ADDR + 0x29)
#define REG_ADC_CONFIG2                 REG_ADDR8(ADC_BASE_ADDR + 0x2a)
#define REG_ADC_RXFIFO_TRIG_NUM         REG_ADDR8(ADC_BASE_ADDR + 0x2b)
#define REG_ADC_RXFIFO_DAT(i)           REG_ADDR16(ADC_BASE_ADDR + 0x2c + 2 * (i))
#define REG_SOFT_CONTROL                REG_ADDR8(ADC_BASE_ADDR + 0x30)

#define AREG_ADC_PGA_CTRL               0xfc
#define AREG_ADC_SAMPLE_CLK_DIV         0xf4
#define AREG_AIN_SCALE                  0xfa
#define AREG_ADC_RES_M                  0xec
#define AREG_ADC_DATA_TRANSFER_CONTROL  0xf2

#endif
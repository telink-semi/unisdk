#ifndef ADC_REG_NEW_H__
#define ADC_REG_NEW_H__

#include "tlk_soc.h"
#include "registers/tlk_adc_reg.h"

#define AREG_ADC_CLK_SETTING            0x82
#define AREG_ADC_VREF                   0xea
#define AREG_ADC_AIN_CHN_MISC           0xeb
#define AREG_ADC_RES_M                  0xec
#define AREG_ADC_TSMAPLE_M              0xee
#define AREG_R_MAX_MC                   0xef
#define AREG_R_MAX_S                    0xf1
#define AREG_ADC_CHN_EN                 0xf2
#define AREG_ADC_DATA_SAMPLE_CONTROL    0xf3
#define AREG_ADC_SAMPLE_CLK_DIV         0xf4
#define AREG_ADC_DATA_STATUS            0xf6
#define AREG_ADC_MISC_L                 0xf7
#define AREG_ADC_VREF_VBAT_DIV          0xf9
#define AREG_AIN_SCALE                  0xfa
#define AREG_ADC_PGA_CTRL               0xfc

#endif
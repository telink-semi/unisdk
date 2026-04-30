/********************************************************************************************************
 * @file    tlk_otp.c
 *
 * @brief   This is the source file for TL721X
 *
 * @author  Driver Group
 * @date    2024
 *
 * @par     Copyright (c) 2024, Telink Semiconductor (Shanghai) Co., Ltd.
 *          All rights reserved.
 *
 *          The information contained herein is confidential property of Telink
 *          Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *          of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *          Co., Ltd. and the licensee or the terms described here-in. This heading
 *          MUST NOT be removed from this file.
 *
 *          Licensee shall not delete, modify or alter (or permit any third party to delete, modify, or
 *          alter) any information contained herein in whole or in part except as expressly authorized
 *          by Telink semiconductor (shanghai) Co., Ltd. Otherwise, licensee shall be solely responsible
 *          for any claim to the extent arising out of or relating to such deletion(s), modification(s)
 *          or alteration(s).
 *
 *          Licensees are granted free, non-transferable use of the information in this
 *          file under Mutual Non-Disclosure Agreement. NO WARRANTY of ANY KIND is provided.
 *
 *******************************************************************************************************/
#include "tlk_otp.h"
#include "core/include/tlk_sys.h"
#include "core/include/tlk_stimer.h"
#include "core/include/tlk_clock.h"
#include "tlk_otp_internal.h"
#include "tlk_core.h"
#include "registers/tlk_otp_reg.h"

#ifndef TLK_OTP_OPERATE_MODE
    #define TLK_OTP_OPERATE_MODE 1 /* 1: auto mode; 0: manual mode(for internal test). */
#endif

/**
 * @brief IP enable time, pwe=1 ---> enable, TLK_Tcs > 10(us).
 * @note  The usage of OTP will include RC as the clock source. 
 *        Considering the accuracy of RC, so this value is set to the twice of spec min.
 */
#define TLK_Tcs (20)

/**
 * @brief Program pulse width time, pwe high time, 20 < TLK_Tpw < 25(us).
 */
#define TLK_Tpw (22)

/**
 * @brief Program pulse interval time, pwe low time, 1 < TLK_Tpwi < 5(us).
 */
#define TLK_Tpwi (4)

/**
 * @brief Program mode recovery time, pporg=0 ---> pce=0, 5 < TLK_Tppr < 100(us).
 */
#define TLK_Tppr (10)

/**
 * @brief Program mode setup time, pprog=1 ---> pwe=1, 5 < TLK_Tpps < 20(us).
 */
#define TLK_Tpps (5)

/**
 * @brief Program mode hold time, pwe=0 ---> pprog=0, 5 < TLK_Tpph < 20(us).
 */
#define TLK_Tpph (10)

/**
 * @brief Deep standby to active mode setup time, deep standby to active mode setup time, TLK_Tsas >2(us).
 */
#define TLK_Tsas (5)

/**
 * @brief PTM mode setup time, ptm  -->  pce = 1, TLK_Tms>1(ns).
 */
#define TLK_Tms (1)

/**
 * @brief PTM mode hold time, ptm  -->  pce = 0, TLK_Tmh>1(ns).
 */
#define TLK_Tmh (1)

/**
 * @brief IP enable time in program, pce = 1 ---> pprog=1, 10 < TLK_Tcsp < 100(us).
 */
#define TLK_Tcsp (20)

/**
 * @brief LDO setup time, ldo setup time, TLK_Tpls > 10(us).
 */
#define TLK_Tpls (15)

/**
 * @brief PTM type.
 */
typedef enum
{
    TLK_OTP_PTM_READ                = 0x00, /**< PTM mode read */
    TLK_OTP_PTM_PROG                = 0x02, /**< PTM mode write */
    TLK_OTP_PTM_INIT_MARGIN_READ    = 0x01, /**< PTM mode init margin read */
    TLK_OTP_PTM_PGM_MARGIN_READ     = 0x04, /**< PTM mode pgm(program) margin read */
    TLK_OTP_PTM_HT_INIT_MARGIN_READ = 0x09, /**< PTM mode ht(high temp) init margin read (internal test)*/
    TLK_OTP_PTM_HT_PGM_MARGIN_READ  = 0x0c, /**< PTM mode ht pgm margin read (internal test)*/
} tlk_otp_ptm_type_e;

/**
 * @brief Auto power up time config.
 */
typedef enum
{
    TLK_OTP_TIM_CONFIG_24M  = 0x00, /**< PCLK (0-24M] */
    TLK_OTP_TIM_CONFIG_48M  = 0x01, /**< PCLK (24-48M] */
    TLK_OTP_TIM_CONFIG_96M  = 0x02, /**< PCLK (48-96M] */
    TLK_OTP_TIM_CONFIG_192M = 0x03, /**< PCLK (96-192M] */
    TLK_OTP_TIM_CONFIG_384M = 0x04, /**< PCLK (192-384M] */
} tlk_otp_tim_config_e;

typedef struct
{
    unsigned char ft_gpio_gain;
    unsigned char ft_gpio_offset;
    unsigned char ft_vbat_gain;
    unsigned char ft_vbat_offset;
    unsigned char cp_gpio_gain;
    unsigned char cp_gpio_offset;
    unsigned char cp_vbat_gain;
    unsigned char cp_vbat_offset;
} tlk_adc_ft_cp_calib_t;

_tlk_always_inline void tlk_sys_reset(void)
{
    reg_pwdn_en = 0x20;
}

/**********************************************************************************************************************
 *                                                External interface                                                  *
 *********************************************************************************************************************/
/*!
 * @name External functions
 * @{
 */

/**
 * @brief      This function serves to init otp clk. This interface must be called to initialize the otp clock before using otp.
 * @return     none
 */
void tlk_otp_clk_init(void)
{
    tlk_otp_tim_config_e tim_config    = 0;
    unsigned char    capt_edge_cnt = 0;

    if (tlk_sys_clk.pclk <= 24) {
        tim_config    = TLK_OTP_TIM_CONFIG_24M;
        capt_edge_cnt = 3;  /* 3 * (1000 / 24) > 110ns(Tcd) */
    } else if (tlk_sys_clk.pclk <= 48) {
        tim_config    = TLK_OTP_TIM_CONFIG_48M;
        capt_edge_cnt = 6;  /* 6 * (1000 / 48) */
    } else if (tlk_sys_clk.pclk <= 96) {
        tim_config    = TLK_OTP_TIM_CONFIG_96M;
        capt_edge_cnt = 12; /* 12 * (1000 / 96) */
    } else if (tlk_sys_clk.pclk <= 192) {
        tim_config    = TLK_OTP_TIM_CONFIG_192M;
        capt_edge_cnt = 24; /* 24 * (1000 / 192) */
    } else {
        tim_config    = TLK_OTP_TIM_CONFIG_384M;
        capt_edge_cnt = 60; /* 40 * (1000 / 384) */
    }

    /*
     * When auto power is used, these delays are generated by internal counters, and when the system clock pclk is switched to another frequency, \n
     * these delays will also be changed, so in order to ensure that these timings are unchanged for OTP, you need to configure reg_tim_cfg accordingly.
     */
    reg_otp_ctrl5 = (reg_otp_ctrl5 & ~(FLD_OTP_TIM_CFG)) | tim_config;

    /* Config pclk cnt to latch data when OTP PCLK rising, cnt * Tpclk > Tcd(max 110ns)*/
    reg_otp_ctrl4 = ((reg_otp_ctrl4 & ~(FLD_OTP_CAP_EDGE)) | (capt_edge_cnt & FLD_OTP_CAP_EDGE));
}

/**
 * @brief      This function serves to wait until the operation of OTP is done.
 * @param[in]  none
 * @return     none
 */
static inline void tlk_otp_wait_done(void)
{
    while (reg_otp_status & FLD_OTP_BUSY)
        ;
}

/**
 * @brief      This function serves to set otp active mode(auto/manual), if otp is in deep mode, need to operate on the otp, set active mode.
 * @return     none
 */
void tlk_otp_set_active_mode(void)
{
#if (TLK_OTP_OPERATE_MODE)
    reg_otp_status |= FLD_OTP_AUTO_PWUP_TRIG;
    tlk_otp_wait_done();
#else
    reg_otp_ctrl1 |= FLD_OTP_PLDO;  /* pldo = 1 */
    tlk_sys_delay(TLK_Tpls);
    reg_otp_ctrl1 |= FLD_OTP_PDSTD; /* pdstd = 1 */
    tlk_sys_delay(TLK_Tsas);

    tlk_otp_read_manual_mode();
#endif
}

/**
 * @brief      This function serves to otp set deep standby mode, can enter deep to save current.
 * @return     none
 */
void tlk_otp_set_deep_standby_mode(void)
{
    reg_otp_ctrl0 &= ~(FLD_OTP_PCE); /* pce=0 */
    /* Tash >= 0(ns) */
    reg_otp_ctrl1 &= ~(FLD_OTP_PDSTD); /* pdstb=0 */
    /* Tplh >= 0(ns) */
    reg_otp_ctrl1 &= ~(FLD_OTP_PLDO); /* pldo=0 */
}

/**
 * @brief     This function is a common sequence used by these interfaces:tlk_otp_write32/tlk_otp_read_cycle_auto/tlk_otp_read_cycle_manual.
 * @param[in] ptm_mode - ptm type.
 * @return    none
 */
static void tlk_otp_start(tlk_otp_ptm_type_e ptm_mode)
{
    reg_otp_ctrl0 &= ~(FLD_OTP_PCE);
    reg_otp_ctrl1 = ((reg_otp_ctrl1 & ~(FLD_OTP_PTM)) | ptm_mode); /* ptm mode */
    /* TLK_Tms >= 1(ns) */
    reg_otp_ctrl0 |= (FLD_OTP_PCE); /* pce = 1*/
}

/**
 * @brief      This function serves to auto read data from OTP memory.
 * @param[in]  ptm_mode - read mode.
 * @param[in]  addr - the address of the data,the otp memory that can access is from 0x0000-0x3FC,can't access other address.
 * @param[in]  word_len  - the length of the data,the unit is word(4 bytes).
 * @param[out]  buff - data buff.
 * @return     none
 */
_tlk_attribute_ram_code_sec_noinline_ static void tlk_otp_read_cycle_auto(tlk_otp_ptm_type_e ptm_mode, unsigned int addr, unsigned int word_len, unsigned int *buff)
{
    unsigned int r = tlk_core_interrupt_disable();
    /* when write to otp, ptm needs to be configured. */
    tlk_otp_start(ptm_mode);
    tlk_clock_cclk_delay((unsigned long long)(TLK_Tcs * tlk_sys_clk.cclk));

    reg_otp_pa = addr;
    buff[0]    = reg_otp_rd_dat; /* trigger read */
    tlk_otp_wait_done();

    /* pa auto inc */
    for (unsigned int i = 0; i < word_len; i++) {
        buff[i] = reg_otp_rd_dat;
        tlk_otp_wait_done();
    }

    reg_otp_ctrl0 &= ~(FLD_OTP_PCE); /* pce = 0*/
    tlk_core_restore_interrupt(r);
}

/**
 * @brief      This function serves to write data to OTP memory,4 bytes one time.
 * @param[in]  ptm_mode - write mode.
 * @param[in]  addr  - the address of the data,the otp memory that can access is from 0x0000-0x3FC,can't access other address.
 * @param[in]  data  - the data need to be write,4 bytes.
 * @return     none
 */
_tlk_attribute_ram_code_sec_noinline_ static void tlk_otp_write32(tlk_otp_ptm_type_e ptm_mode, unsigned int addr, unsigned int data)
{
    unsigned int r = tlk_core_interrupt_disable();
    /* when write to otp, ptm needs to be configured. */
    tlk_otp_start(ptm_mode);
    /* TLK_Tcsp */
    tlk_sys_delay(TLK_Tcsp);

    /* prog pas addr data */
    reg_otp_ctrl0 |= FLD_OTP_PPROG;
    reg_otp_ctrl0 |= FLD_OTP_PAS;
    reg_otp_pa     = addr;
    reg_otp_wr_dat = data;
    reg_otp_paio   = 0;
    /* TLK_Tpps */
    tlk_sys_delay(TLK_Tpps);

    /* redundancy programming  38*2 */
    for (unsigned char i = 1; i <= 76; i++) {
        reg_otp_ctrl0 |= FLD_OTP_PWE;
        tlk_sys_delay(TLK_Tpw);
        reg_otp_ctrl0 &= ~(FLD_OTP_PWE);
        if (i < 38) {
            reg_otp_paio = i;
        } else if (i == 38) {
            reg_otp_ctrl0 &= (~FLD_OTP_PAS);
            reg_otp_pa     = addr;
            reg_otp_wr_dat = data;
            reg_otp_paio   = 0;
        } else if ((i > 38) && (i < 76)) {
            reg_otp_paio = i - 38;
        } else if (i == 76) {
            break;
        }
        // because the for loop and the if judge the time,choose to use TLK_Tpwi/2.
        tlk_sys_delay(TLK_Tpwi / 2);
    }
    tlk_sys_delay(TLK_Tpph);
    reg_otp_ctrl0 &= ~(FLD_OTP_PPROG); /* pporg = 0 */
    tlk_sys_delay(TLK_Tppr);
    reg_otp_ctrl0 &= ~(FLD_OTP_PCE);   /* pce = 0 */
    /* TLK_Tmh >= 1(ns) */
    reg_otp_ctrl1 = ~(FLD_OTP_PTM);
    tlk_core_restore_interrupt(r);
}

/**
 * @brief      This function serves to read data from OTP memory, belong to otp normal read.
 *             otp has three kinds of read mode,in general,just use OTP_READ normal read operation, when the execution of burning operation,
 *             need to use margin read(tlk_otp_pgm_margin_read,tlk_otp_initial_margin_read),check whether the write is successful.
 * @param[in]  addr - the otp address of the data,it has to be a multiple of 4,the otp memory that can access is from 0x0000-0x3FC,can't access other address.
 * @param[in]  word_len  - the length of the data,the unit is word(4 bytes).
 * @param[out]  buff - data buff.
 * @return     none
 */
void tlk_otp_read(unsigned int addr, unsigned int word_len, unsigned int *buff)
{
#if (TLK_OTP_OPERATE_MODE)
    tlk_otp_read_cycle_auto(TLK_OTP_PTM_READ, addr, word_len, buff);
#else
    tlk_otp_read_cycle_manual(TLK_OTP_PTM_READ, addr, word_len, buff);
#endif
}

/**
 * @brief      This function serves to write data to OTP memory.
 *             the minimum unit of otp read-write operation is 4 bytes, that is a word. meanwhile, the otp cannot be burned repeatedly,
 *             this function is limited to writing only once,this function will determine if the otp is 0xffffffff, and if it is 0xffffffff,
 *             it will write the otp.
 * @param[in]  addr - the address of the data,it has to be a multiple of 4,the OTP memory that can access is from 0x0000-0x3FC,can't access other address.
 * @param[in]  word_len  - the length of the data,the unit is word(4 bytes).
 * @param[out] buff - data buff.
 * @return
 *             - 0 it means that the otp operation area is 0xffffffff or the write data,
 *                return 0 not mean that the burning was successful,need to use three kinds of read mode to check whether the writing was successful.
 *             - 1 it means that there is an operation value in the operation area,it is not 0xffffffff or the write data,no burning action is performed.
 */
unsigned char tlk_otp_write(unsigned int addr, unsigned int word_len, unsigned int *buff)
{
    for (unsigned int i = 0; i < word_len; i++) {
        unsigned int temp = 0;
        tlk_otp_read(addr + i * 4, 1, (unsigned int *)&temp);
        if (temp == 0xffffffff) {
            tlk_otp_write32(TLK_OTP_PTM_PROG, addr + i * 4, buff[i]);
        } else if (temp != buff[i]) {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief      This function serves to read data from OTP memory,belong to otp pgm margin read.
 *             otp has three kinds of read mode,in general, just use OTP_READ normal read operation, when the execution of burning operation,
 *             need to use margin read(tlk_otp_pgm_margin_read,tlk_otp_initial_margin_read),check whether the write is successful.
 * @param[in]  addr - the otp address of the data,it has to be a multiple of 4,the otp memory that can access is from 0x0000-0x3FC,can't access other address.
 * @param[in]  word_len  - the length of the data,the unit is word(4 bytes).
 * @param[out] buff - data buff.
 * @return     none
 */
void tlk_otp_pgm_margin_read(unsigned int addr, unsigned int word_len, unsigned int *buff)
{
#if (TLK_OTP_OPERATE_MODE)
    tlk_otp_read_cycle_auto(TLK_OTP_PTM_PGM_MARGIN_READ, addr, word_len, buff);
#else
    tlk_otp_read_cycle_manual(TLK_OTP_PTM_PGM_MARGIN_READ, addr, word_len, buff);
#endif
}

/**
 * @brief      This function serves to read data from OTP memory,belong to otp initial margin read.
 *             otp has three kinds of read mode,in general, just use OTP_READ normal read operation, when the execution of burning operation,
 *             need to use margin read(tlk_otp_pgm_margin_read,tlk_otp_initial_margin_read),check whether the write is successful.
 * @param[in]  addr - the otp address of the data,it has to be a multiple of 4,the otp memory that can access is from 0x0000-0x3FC,can't access other address.
 * @param[in]  word_len  - the length of the data,the unit is word(4 bytes).
 * @param[out] buff - data buff.
 * @return     none
 *
 */
void tlk_otp_initial_margin_read(unsigned int addr, unsigned int word_len, unsigned int *buff)
{
#if (TLK_OTP_OPERATE_MODE)
    tlk_otp_read_cycle_auto(TLK_OTP_PTM_INIT_MARGIN_READ, addr, word_len, buff);
#else
    tlk_otp_read_cycle_manual(TLK_OTP_PTM_INIT_MARGIN_READ, addr, word_len, buff);
#endif
}

/**
 * @brief        This function serves to check protection code according SDK version.
 * @param[in]    sdk_version, 0:driver sdk  0xff:sdk_version_ignore
 * @return       none.
 */
void tlk_otp_check_protection_code(unsigned char sdk_version)
{
    unsigned int pCode = 0;
    /* set otp active */
    tlk_otp_set_active_mode();
    tlk_otp_read(104, 1, (unsigned int *)&pCode);
    /* shutdown otp */
    tlk_otp_set_deep_standby_mode();
    pCode = pCode & 0x1f; //Bit0-4 is market protection code.

    switch (sdk_version) {
    case 0:
        //Different SDKs have different restrictions. Please modify the code according to your own situation.
        //The driver here is only for example reference.
        if (0xE0 > pCode) {
            tlk_sys_reset();
            while (1)
                ;
        }
        break;
    case 0xff:
        break;
    default:
        if (1) // Prevent macro setting exceptions from invalidating the ProtectionCode function
        {
            tlk_sys_reset();
            while (1)
                ;
        }
        break;
    }
}

/**
 * @}
 */

/**********************************************************************************************************************
 *                                                Internal interface                                                  *
 *********************************************************************************************************************/
/*!
 * @name Internal functions
 * @{
 */

/**
 * @brief      This function serves to get otp auto load data. As soon as the system is powered on, \n
 *             OTP will automatically load the 3-word data from OTP address 0 to the auto_load register.
 * @param[in]  index - auto load data index. Range 0-2.
 * @return     Auto load data(word).
 * @note       The auto load is triggered when the device is powered on for the first time, reset pin, or deep wake up.
 */
unsigned int tlk_otp_get_auto_load_data(unsigned char index)
{
    return reg_otp_auto_load_data(index);
}

/**
 * @brief      This function serves to enable key lock. After key lock enable, the data in OTP word address 3~10 cannot be read.
 * @return     none
 * @note       key_lock with a reset value of 0 and can only be written from 0 to 1, not from 1 to 0.
 */
void tlk_otp_key_lock(void)
{
    reg_otp_status |= FLD_OTP_KEYLOCK;
}

/**
 * @brief      This function serves to enable row test.
 * @return     none
 */
void tlk_otp_test_row_en(void)
{
    reg_otp_ctrl0 |= FLD_OTP_PTR;
    reg_otp_ctrl0 &= ~(FLD_OTP_PTC);
}

/**
 * @brief      This function serves to enable column test.
 * @return     none
 */
void tlk_otp_test_column_en(void)
{
    reg_otp_ctrl0 &= ~(FLD_OTP_PTR);
    reg_otp_ctrl0 |= FLD_OTP_PTC;
}

/**
 * @brief      This function serves to disable test mode.
 * @return     none
 * @note       PTR and PTC are both in normal read and write mode when they are at the same low level,
 *             and should be selected for both when they are at the same low level,
 *             but it is not mentioned in the IP manual.
 */
void tlk_otp_test_dis(void)
{
    reg_otp_ctrl0 &= ~(FLD_OTP_PTR | FLD_OTP_PTC);
}

/**
 * @brief      This function serves to select read mode as auto.
 * @return     none
 */
void tlk_otp_read_auto_mode(void)
{
    reg_otp_ctrl3 &= ~(FLD_OTP_MAN_MODE);
}

/**
 * @brief      This function serves to select read mode as manual.
 * @return     none
 */
void tlk_otp_read_manual_mode(void)
{
    reg_otp_ctrl3 |= FLD_OTP_MAN_MODE;
}

/**
 * @brief      This function serves to enable ECC verification function.
 * @return     none
 * @note       For each address read, the IP will deliver the 38 bits at PDOUT[37:0],
 *             PDOUT[37:32] is for 6 ECC parity bits, PDOUT[31:0 ] is for user storage data.
 *             After enabling, PDOUT[31:0] will be automatically corrected.
 */
void tlk_otp_ecc_en(void)
{
    reg_otp_ctrl0 &= ~(FLD_OTP_ECC_RDB);
}

/**
 * @brief      This function serves to disable ecc.
 * @return     none
 */
void tlk_otp_ecc_dis(void)
{
    reg_otp_ctrl0 |= FLD_OTP_ECC_RDB;
}

/**
 * @brief      This function serves to manual read data from OTP memory(for internal test).
 * @param[in]  ptm_mode - read mode.
 * @param[in]  addr - the address of the data,the otp memory that can access is from 0x0000-0x3FC,can't access other address.
 * @param[in]  word_len  - the length of the data,the unit is word(4 bytes).
 * @param[out] buff - data buff.
 * @return     none
 */
_tlk_attribute_ram_code_sec_noinline_ void tlk_otp_read_cycle_manual(tlk_otp_ptm_type_e ptm_mode, unsigned int addr, unsigned int word_len, unsigned int *buff)
{
    unsigned int r = tlk_core_interrupt_disable();
    /* when write to otp, ptm needs to be configured. */
    tlk_otp_start(ptm_mode);
    tlk_sys_delay(TLK_Tcs);

    for (unsigned int i = 0; i < word_len; i++) {
        reg_otp_pa = addr + i * 4;
        /* Tas >= 1(ns) */
        reg_otp_ctrl3 |= FLD_OTP_MAN_PCLK;
        /* Tkh >= 20(ns) && Tcd > 110ns*/
        tlk_clock_cclk_delay(120);
        reg_otp_ctrl3 &= ~(FLD_OTP_MAN_PCLK);
        buff[i] = reg_otp_rd_dat;
        /* Tkl >= 20(ns) */
        tlk_clock_cclk_delay(25);
    }

    reg_otp_ctrl0 &= ~(FLD_OTP_PCE);
    tlk_core_restore_interrupt(r);
}

/**
 * @brief      This function serves to read data from OTP memory,belong to otp high temp pgm margin read.
 *             otp has three kinds of read mode,in general, just use OTP_READ normal read operation, when the execution of burning operation,
 *             need to use margin read(tlk_otp_pgm_margin_read,tlk_otp_initial_margin_read),check whether the write is successful.
 * @param[in]  addr - the otp address of the data,it has to be a multiple of 4,the otp memory that can access is from 0x0000-0x3FC,can't access other address.
 * @param[in]  word_len  - the length of the data,the unit is word(4 bytes).
 * @param[out] buff - data buff.
 * @return     none
 */
void tlk_otp_ht_pgm_margin_read(unsigned int addr, unsigned int word_len, unsigned int *buff)
{
#if (TLK_OTP_OPERATE_MODE)
    tlk_otp_read_cycle_auto(TLK_OTP_PTM_HT_PGM_MARGIN_READ, addr, word_len, buff);
#else
    tlk_otp_read_cycle_manual(TLK_OTP_PTM_HT_PGM_MARGIN_READ, addr, word_len, buff);
#endif
}

/**
 * @brief      This function serves to read data from OTP memory,belong to otp high temp initial margin read.
 *             otp has three kinds of read mode,in general, just use OTP_READ normal read operation, when the execution of burning operation,
 *             need to use margin read(tlk_otp_pgm_margin_read,tlk_otp_initial_margin_read),check whether the write is successful.
 * @param[in]  addr - the otp address of the data,it has to be a multiple of 4,the otp memory that can access is from 0x0000-0x3FC,can't access other address.
 * @param[in]  word_len  - the length of the data,the unit is word(4 bytes).
 * @param[out] buff - data buff.
 * @return     none
 *
 */
void tlk_otp_ht_initial_margin_read(unsigned int addr, unsigned int word_len, unsigned int *buff)
{
#if (TLK_OTP_OPERATE_MODE)
    tlk_otp_read_cycle_auto(TLK_OTP_PTM_HT_INIT_MARGIN_READ, addr, word_len, buff);
#else
    tlk_otp_read_cycle_manual(TLK_OTP_PTM_HT_INIT_MARGIN_READ, addr, word_len, buff);
#endif
}

/**
 * @brief      This function serves to write data to OTP column memory.
 * @param[in]  ptm_mode - write mode.
 * @param[in]  addr  - the address of the data,the otp memory that can access is from 0x0000-0x3C0,can't access other address.
 * @param[in]  data  - the data need to be write,4 bytes.
 * @return     none
 */
_tlk_attribute_ram_code_sec_noinline_ static void tlk_otp_write32_column(tlk_otp_ptm_type_e ptm_mode, unsigned int addr, unsigned int data)
{
    unsigned int r = tlk_core_interrupt_disable();
    /* when write to otp, ptm needs to be configured. */
    tlk_otp_start(ptm_mode);
    /* TLK_Tcsp */
    tlk_sys_delay(TLK_Tcsp);

    /* prog pas addr data */
    reg_otp_ctrl0 |= FLD_OTP_PPROG;
    reg_otp_ctrl0 |= FLD_OTP_PAS;
    reg_otp_pa     = addr;
    reg_otp_wr_dat = data;
    reg_otp_paio   = 0;
    /* TLK_Tpps */
    tlk_sys_delay(TLK_Tpps);

    /* redundancy programming  38*2 */
    for (unsigned char i = 1; i <= 76; i++) {
        reg_otp_ctrl0 |= FLD_OTP_PWE;
        tlk_sys_delay(TLK_Tpw);
        reg_otp_ctrl0 &= ~(FLD_OTP_PWE);
        if (i < 38) {
            reg_otp_paio = 0;
        } else if (i == 38) {
            reg_otp_ctrl0 &= (~FLD_OTP_PAS);
            reg_otp_pa     = addr;
            reg_otp_wr_dat = data;
            reg_otp_paio   = 0;
        } else if ((i > 38) && (i < 76)) {
            reg_otp_paio = 0;
        } else if (i == 76) {
            break;
        }
        // because the for loop and the if judge the time,choose to use TLK_Tpwi/2.
        tlk_sys_delay(TLK_Tpwi / 2);
    }
    tlk_sys_delay(TLK_Tpph);
    reg_otp_ctrl0 &= ~(FLD_OTP_PPROG); /* pporg = 0 */
    tlk_sys_delay(TLK_Tppr);
    reg_otp_ctrl0 &= ~(FLD_OTP_PCE);   /* pce = 0 */
    /* TLK_Tmh >= 1(ns) */
    reg_otp_ctrl1 = ~(FLD_OTP_PTM);
    tlk_core_restore_interrupt(r);
}

/**
 * @brief      This function serves to write data to OTP row test memory.
 * @param[in]  addr  - the address of the data,the otp row test memory that can access is from 0x00-0x3F,can't access other address.
 * @param[in]  word_len  - the length of the data,the unit is word(4 bytes).
 * @param[out] buff - data buff.
 * @return
 *             - 0 it means that the otp operation area is 0xffffffff or the write data,
 *                return 0 not mean that the burning was successful,need to use three kinds of read mode to check whether the writing was successful.
 *             - 1 it means that there is an operation value in the operation area,it is not 0xffffffff or the write data,no burning action is performed.
 */
unsigned char tlk_otp_write_row(unsigned int addr, unsigned int word_len, unsigned int *buff)
{
    addr &= ~(TLK_BIT_RNG(6, 7) | (TLK_BIT_RNG(0, 1) << 8));
    return tlk_otp_write(addr, word_len, buff);
}

/**
 * @brief      This function serves to write data to OTP column test memory, 64 bytes one time, can only write bit0.
 * @param[in]  addr  - the address of the data,the otp column test memory that can access is from 0x000-0x3C0,can't access other address.
 * @param[in]  word_len  - the length of the data,the unit is word(4 bytes).
 * @param[out] buff - data buff.
 * @return
 *             - 0 it means that the otp operation area is 0xffffffff or the write data,
 *                return 0 not mean that the burning was successful,need to use three kinds of read mode to check whether the writing was successful.
 *             - 1 it means that there is an operation value in the operation area,it is not 0xffffffff or the write data,no burning action is performed.
 */
unsigned char tlk_otp_write_column(unsigned int addr, unsigned int word_len, unsigned int *buff)
{
    TLK_BM_CLR(addr, TLK_BIT_RNG(2, 5));
    for (unsigned int i = 0; i < word_len; i++) {
        unsigned int temp = 0;
        tlk_otp_read(addr + i * 4, 1, (unsigned int *)&temp);
        if (temp == 0xffffffff) {
            tlk_otp_write32_column(TLK_OTP_PTM_PROG, addr + i * 4, buff[i]);
        } else if (temp != buff[i]) {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief      This function serves to read data from OTP row test memory, belong to otp normal read.
 * @param[in]  addr  - the address of the data,the otp row test memory that can access is from 0x00-0x3F,can't access other address.
 * @param[in]  word_len  - the length of the data,the unit is word(4 bytes).
 * @param[out]  buff - data buff.
 * @return     none
 */
void tlk_otp_row_read(unsigned int addr, unsigned int word_len, unsigned int *buff)
{
    addr &= ~(TLK_BIT_RNG(6, 7) | (TLK_BIT_RNG(0, 1) << 8));
    tlk_otp_read(addr, word_len, buff);
}

/**
 * @brief      This function serves to read data from OTP column test memory, belong to otp normal read.
 * @param[in]  addr  - the address of the data,the otp column test memory that can access is from 0x00-0x3C0,can't access other address.
 * @param[in]  word_len  - the length of the data,the unit is word(4 bytes).
 * @param[out]  buff - data buff.
 * @return     none
 */
void tlk_otp_column_read(unsigned int addr, unsigned int word_len, unsigned int *buff)
{
    addr &= ~(TLK_BIT_RNG(6, 7) | (TLK_BIT_RNG(0, 1) << 8));
    tlk_otp_read(addr, word_len, buff);
}

/**
 * @brief      This function serves to read IEEE address from OTP.
 * @param[out] buf  - Pointer to IEEE address buffer(IEEE address is 8bytes)
 * @return     none
 */
void tlk_otp_get_ieee_addr(unsigned char *buf)
{
    tlk_otp_set_active_mode();
    tlk_otp_read(0x6c, 2, (unsigned int *)buf);
    tlk_otp_set_deep_standby_mode();
}

/**
 * @brief       This function is used to Tighten the judgment of illegal values for gpio calibration and vbat calibration in the otp.
 *              The ADC vref gain calibtation should range from 1100mV to 1350mV, the ADC vref offset calibration should range from -100mV to 100mV.
 * @param[in]   gain - the value of gpio_calib_vref_gain or vbat_calib_vref_gain
 *              offset - the value of gpio_calib_vref_offset or vbat_calib_vref_offset
 *              calib_func - Function pointer to gpio_calibration or vbat_calibration.
 * @return      TLK_DRV_API_FAILURE:the calibration function is invalid; TLK_DRV_API_SUCCESS:the calibration function is valid.
 */
uint8_t tlk_otp_set_adc_calib_value(unsigned char gain, signed char offset, void (*calib_func)(unsigned short, signed char))
{
    /**
     * The legal range of gain for both gpio and vbat in OTP is [0,250],
     * and the legal range of offset for both gpio and vbat is [-100,100].
     */
    if ((gain <= 250) && (offset >= -100) && (offset <= 100)) {//otp bit defaults to 1.
        (*calib_func)(gain + 1100, offset);
        return 0;
    } else {
        return 1;
    }
}

/**
 * @brief      This function is used to calib ADC 1.2V vref.
 * @param[in]  none
 * @return     TLK_DRV_API_SUCCESS - the calibration value update, TLK_DRV_API_FAILURE - the calibration value is not update.
 */
// tlk_drv_api_status_e tlk_otp_calib_adc_vref(void)
// {
//     tlk_adc_ft_cp_calib_t calib_value;
//     /********************************************************************************************
//         The ADC calibration value priority of TL721X is FT > CP.
//         The GPIO calibration value and the VBAT calibration value do not necessarily exist at the same time.
//     ********************************************************************************************/
//     tlk_otp_set_active_mode();
//     tlk_otp_read(0x94, 2, (unsigned int *)&calib_value);
//     tlk_otp_set_deep_standby_mode();

//     if (tlk_otp_set_adc_calib_value(calib_value.ft_vbat_gain, (signed char)calib_value.ft_vbat_offset, adc_set_vbat_calib_vref) || tlk_otp_set_adc_calib_value(calib_value.ft_gpio_gain, (signed char)calib_value.ft_gpio_offset, adc_set_gpio_calib_vref))     //vbat_ft and gpio_ft
//     {
//         if (tlk_otp_set_adc_calib_value(calib_value.cp_vbat_gain, (signed char)calib_value.cp_vbat_offset, adc_set_vbat_calib_vref) || tlk_otp_set_adc_calib_value(calib_value.cp_gpio_gain, (signed char)calib_value.cp_gpio_offset, adc_set_gpio_calib_vref)) //vbat_cp and gpio_cp
//         {
//             return TLK_DRV_API_FAILURE;
//         }
//     }
//     return TLK_DRV_API_SUCCESS;
// }


/**
 * @brief       This function serves to read vdd0p94 and vddo1p8 calibration data from OTP.
 * @return      res 0: ok, 1: vdd0p94 invalid, 2: vddo1p8 invalid, 3: vdd0p94 and vddo1p8 all invalid
 */
_tlk_attribute_ram_code_sec_noinline_ unsigned char tlk_otp_get_vdd0p94_vddo1p8_calib_value(tlk_power_cal_vdd0p94_t *tlk_power_vdd0p94_cal_info, uint8_t *tlk_g_pm_cal_vddo1p8_info)
{
    unsigned char res_cal_vdd0p94 = 0, res_cal_vddo1p8 = 0;

    unsigned int otp_value[4] = {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};

    tlk_otp_set_active_mode();

    /* 
     * 0x84 vdd0P94 FT 4bytes - otp_value[0]
     * 0x88 vdd0P94 CP 4bytes - otp_value[1]
     * 0x8c vdd1P8  FT 1byte  - otp_value[2]
     * 0x90 vdd1P8  CP 1byte  - otp_value[3]
     */
    tlk_otp_read(0x84, 4, otp_value); //readout vdd0p94 and vdd1p8 at once
    tlk_otp_set_deep_standby_mode();

    /*
     * vdd0P94 0-3 bytes FT, 4-7 bytes CP.
     * The correct data format should be 0x0_0_0_0_.(_:0~f).
     * The standard for determining the validity of data is every byte data is between 0-15 corresponding to tlk_pm_trim_vdd0p94_e.
     * Calibration priority FT > CP.
     * (added by jilong.liu, confirmed by zhengting.hu and hao.wang at 20241219)
     */
    for (int i = 0; i < 2; i++) {
        if (!(otp_value[i] & 0xf0f0f0f0)) {
            tlk_power_vdd0p94_cal_info->dcdc_0p95v = otp_value[i] & 0xff;
            tlk_power_vdd0p94_cal_info->ldo_0p95v  = (otp_value[i] >> 8) & 0xff;
            tlk_power_vdd0p94_cal_info->dcdc_1p05v = (otp_value[i] >> 16) & 0xff;
            tlk_power_vdd0p94_cal_info->ldo_1p05v  = (otp_value[i] >> 24) & 0xff;

            res_cal_vdd0p94 = 0;
            break;
        } else {
            res_cal_vdd0p94 = 1;
        }
    }

    /*
     * vddo1P8 1bytes FT, 1bytes CP. 
     * Since OTP must be written in 4 bytes, the remaining 3 bytes will written as 0xff.
     * The correct data format should be 0xffffff0_.(_:0~7).
     * The standard for determining the validity of data is every byte data is between 0-7 corresponding to tlk_pm_trim_vddo1p8_e.
     * Calibration priority FT > CP.
     * (added by jilong.liu, confirmed by zhengting.hu and hao.wang at 20241219)
     */
    for (int i = 2; i < 4; i++) {
        if (((otp_value[i] >> 8) == 0xffffff) && (!(otp_value[i] & 0xf8))) {
            *tlk_g_pm_cal_vddo1p8_info = otp_value[i] & 0xff;

            res_cal_vddo1p8 = 0;
            break;
        } else {
            res_cal_vddo1p8 = 2;
        }
    }

    return (res_cal_vdd0p94 + res_cal_vddo1p8);
}

/**
 * @}
 */

/********************************************************************************************************
 * @file    tlk_mspi.c
 *
 * @brief   This is the source file for TL321X
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
 *          Licensee shall not delete, modify or alter (or permit any third party to delete, modify,
 * or alter) any information contained herein in whole or in part except as expressly authorized by
 * Telink semiconductor (shanghai) Co., Ltd. Otherwise, licensee shall be solely responsible for any
 * claim to the extent arising out of or relating to such deletion(s), modification(s) or
 * alteration(s).
 *
 *          Licensees are granted free, non-transferable use of the information in this
 *          file under Mutual Non-Disclosure Agreement. NO WARRANTY of ANY KIND is provided.
 *
 *******************************************************************************************************/
#include "drivers/include/tlk_mspi.h"

/**
 * @brief       This function servers to write mspi fifo.
 * @param[in]   data    - the pointer to the data for write.
 * @param[in]   len     - write length.
 * @return      none
 */
_tlk_attribute_ram_code_sec_noinline_ void tlk_mspi_write(const unsigned char* data,
                                                          unsigned int         len)
{
    for (unsigned int i = 0; i < len; i++)
    {
        // while (reg_mspi_txfifo_status & FLD_MSPI_TXFIFO_FULL);    //do not need software to
        // block,use hardware block mcu.
        reg_mspi_wr_rd_data(i % 4) = data[i];
    }
    tlk_mspi_wait();
}

/**
 * @brief       This function servers to read mspi fifo.
 * @param[in]   data    - the pointer to the data for read.
 * @param[in]   len     - write length.
 * @return      none
 */
_tlk_attribute_ram_code_sec_noinline_ void tlk_mspi_read(unsigned char* data, unsigned int len)
{
    for (unsigned int i = 0; i < len; i++)
    {
        //      while (reg_mspi_rxfifo_status & FLD_MSPI_RXFIFO_EMPTY);   //do not need software to
        //      block,use hardware block mcu.
        data[i] = reg_mspi_wr_rd_data(i % 4);
    }
    tlk_mspi_wait();
}

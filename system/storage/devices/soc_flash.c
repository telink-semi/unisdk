#include "common/include/tlk_init.h"
#include "drivers/include/tlk_mspi.h"
#include "properties/tlk_chip.h"
#include "storage_device.h"
#include <errno.h>

#define SOC_FLASH_START_ADDR UNISDK_CHIP_MEMORY_ROM_STARTADDR
#define SOC_FLASH_SIZE       (UNISDK_CHIP_MEMORY_ROM_SIZE * 1024)
#define SOC_FLASH_PAGE_SIZE  4096
#define SOC_FLASH_CLEAN_BYTE 0xff

#define SOC_FLASH_MAX_WRITE_BLOCK 256
#define SOC_FLASH_BUSY_CNT_MAX    10000000
#define MMISC_CTL                 0x7d0
#define MMISC_CTL_BRPE_MASK       TLK_BIT(3)

enum
{
    SOC_FLASH_DREAD_CMD                    = 0x3b4097a9,
    SOC_FLASH_X4READ_CMD                   = 0xeb4493ba,
    SOC_FLASH_READ_SECURITY_REGISTERS_CMD  = 0x480097a8,
    SOC_FLASH_READ_UID_CMD_GD_PUYA_ZB_TH   = 0x4b0097a8,
    SOC_FLASH_GET_JEDEC_ID                 = 0x9f002080,
    SOC_FLASH_READ_STATUS_CMD_LOWBYTE      = 0x05002080,
    SOC_FLASH_READ_STATUS_CMD_HIGHBYTE     = 0x35002080,
    SOC_FLASH_READ_CONFIGURE_CMD           = 0x15002080,
    SOC_FLASH_WRITE_CMD                    = 0x020010a8,
    SOC_FLASH_QUAD_PAGE_PROGRAM_CMD        = 0x320010aa,
    SOC_FLASH_SECT_ERASE_CMD               = 0x200070a8,
    SOC_FLASH_WRITE_SECURITY_REGISTERS_CMD = 0x420010a8,
    SOC_FLASH_ERASE_SECURITY_REGISTERS_CMD = 0x440070a8,
    SOC_FLASH_WRITE_STATUS_CMD_LOWBYTE     = 0x01001080,
    SOC_FLASH_WRITE_STATUS_CMD_HIGHBYTE    = 0x31001080,
    SOC_FLASH_WRITE_CONFIGURE_CMD_1        = 0x31001080,
    SOC_FLASH_WRITE_CONFIGURE_CMD_2        = 0x11001080,
    SOC_FLASH_WRITE_DISABLE_CMD            = 0x04007080,
    SOC_FLASH_WRITE_ENABLE_CMD             = 0x06007080,
    SOC_FLASH_WRITE_DEEP_CMD               = 0xb9007080,
    SOC_FLASH_WRITE_RELEASE_CMD            = 0xab007080
};

_tlk_always_inline static bool mmisc_ctl_brpe_clear_and_get(void)
{
    uint32_t old;

    __asm__ volatile("csrrc %0, %1, %2"
                     : "=r"(old)
                     : "i"(MMISC_CTL), "r"(MMISC_CTL_BRPE_MASK)
                     : "memory");
    return (old & MMISC_CTL_BRPE_MASK);
}

_tlk_always_inline static void mmisc_ctl_brpe_restore(bool saved)
{
    if (saved)
    {
        __asm__ volatile("csrrs x0, %0, %1"
                         :
                         : "i"(MMISC_CTL), "r"(MMISC_CTL_BRPE_MASK)
                         : "memory");
    }
    else
    {
        __asm__ volatile("csrrc x0, %0, %1"
                         :
                         : "i"(MMISC_CTL), "r"(MMISC_CTL_BRPE_MASK)
                         : "memory");
    }
}

_tlk_always_inline static void soc_flash_send_cmd(uintptr_t addr, uint32_t cmd)
{
    tlk_mspi_set_address(addr);
    tlk_mspi_set_ctrl(cmd);
    tlk_mspi_set_reg_ctrl0(cmd >> 16);
    tlk_mspi_set_cmd(cmd >> 24);
    tlk_mspi_wait();
}

_tlk_always_inline static bool soc_flash_is_busy(uintptr_t addr, uint32_t cmd)
{
    tlk_mspi_rx_cnt(1);
    tlk_mspi_set_address(addr);
    uint8_t cipher_sta = tlk_mspi_cipher_ctrl_get();
    tlk_mspi_cipher_ctrl_set(0);
    tlk_mspi_set_ctrl(cmd);
    tlk_mspi_set_reg_ctrl0(cmd >> 16);
    tlk_mspi_set_cmd(cmd >> 24);
    uint8_t status;

    tlk_mspi_read(&status, sizeof(status));
    tlk_mspi_cipher_ctrl_set(cipher_sta);
    tlk_mspi_wait();
    return (status & TLK_BIT(0));
}

_tlk_always_inline static void soc_flash_wait_done(uintptr_t addr, uint32_t cmd)
{
    for (size_t i = 0; i < SOC_FLASH_BUSY_CNT_MAX; ++i)
    {
        if (!soc_flash_is_busy(addr, cmd))
        {
            break;
        }
    }
}

_tlk_always_inline static void soc_flash_mspi_read(uint32_t cmd, uintptr_t addr, void* data,
                                                   size_t data_len)
{
    tlk_mspi_rx_cnt(data_len);
    tlk_mspi_set_address(addr);
    tlk_mspi_set_ctrl(cmd);
    tlk_mspi_set_reg_ctrl0(cmd >> 16);
    tlk_mspi_set_cmd(cmd >> 24);
    tlk_mspi_read(data, data_len);
}

_tlk_always_inline static void soc_flash_mspi_write(uint32_t cmd, uintptr_t addr, const void* data,
                                                    size_t data_len, uint32_t w_en_cmd,
                                                    uint32_t busy_cmd)
{
    if (w_en_cmd)
    {
        soc_flash_send_cmd(addr, w_en_cmd);
    }
    tlk_mspi_tx_cnt(data_len);
    tlk_mspi_set_address(addr);
    tlk_mspi_set_ctrl(cmd);
    tlk_mspi_set_reg_ctrl0(cmd >> 16);
    tlk_mspi_set_cmd(cmd >> 24);
    tlk_mspi_write(data, data_len);
    if (busy_cmd)
    {
        soc_flash_wait_done(addr, busy_cmd);
    }
}

_tlk_attribute_ram_code_sec_noinline_ static void
soc_flash_mspi_wr_ram(uint32_t cmd, uintptr_t addr, const void* src, void* dst, size_t data_len,
                      bool is_encrypt, tlk_mspi_func_e mspi_wr, uint32_t w_en_cmd,
                      uint32_t busy_cmd)
{
    uint32_t r = tlk_core_interrupt_disable();

    tlk_mspi_stop_xip();

    uint8_t cipher_sta = tlk_mspi_cipher_ctrl_get();

    if (is_encrypt)
    {
        if (mspi_wr == TLK_MSPI_READ)
        {
            tlk_mspi_cipher_read_en();
            soc_flash_mspi_read(cmd, addr, dst, data_len);
        }
        else if (mspi_wr == TLK_MSPI_WRITE)
        {
            tlk_mspi_cipher_write_en();
            soc_flash_mspi_write(cmd, addr, src, data_len, w_en_cmd, busy_cmd);
        }
    }
    else
    {
        if (mspi_wr == TLK_MSPI_READ)
        {
            tlk_mspi_cipher_read_dis();
            soc_flash_mspi_read(cmd, addr, dst, data_len);
        }
        else if (mspi_wr == TLK_MSPI_WRITE)
        {
            tlk_mspi_cipher_write_dis();
            soc_flash_mspi_write(cmd, addr, src, data_len, w_en_cmd, busy_cmd);
        }
    }
    tlk_mspi_cipher_ctrl_set(cipher_sta);
    __asm__ __volatile__("nop");
    __asm__ __volatile__("nop");
    __asm__ __volatile__("nop");
    __asm__ __volatile__("nop");
    __asm__ __volatile__("nop");
    tlk_mspi_set_xip_en();
    tlk_core_restore_interrupt(r);
}

static int soc_flash_read(struct tlk_storage_device* dev, uintptr_t addr, void* buf, size_t len)
{
    (void) dev;
    bool brpe = mmisc_ctl_brpe_clear_and_get();

    soc_flash_mspi_wr_ram(SOC_FLASH_X4READ_CMD, addr, NULL, buf, len, false, TLK_MSPI_READ, 0, 0);
    mmisc_ctl_brpe_restore(brpe);
    return 0;
}

static int soc_flash_erase(struct tlk_storage_device* dev, uintptr_t addr, size_t len)
{
    (void) dev;
    (void) len;
    bool brpe = mmisc_ctl_brpe_clear_and_get();

    soc_flash_mspi_wr_ram(SOC_FLASH_SECT_ERASE_CMD,
                          addr,
                          NULL,
                          NULL,
                          0,
                          false,
                          TLK_MSPI_WRITE,
                          SOC_FLASH_WRITE_ENABLE_CMD,
                          SOC_FLASH_READ_STATUS_CMD_LOWBYTE);
    mmisc_ctl_brpe_restore(brpe);
    return 0;
}

static int soc_flash_write(struct tlk_storage_device* dev, uintptr_t addr, const void* buf,
                           size_t len)
{
    (void) dev;

    while (len)
    {
        size_t wr_len = MIN(len, SOC_FLASH_MAX_WRITE_BLOCK - addr % SOC_FLASH_MAX_WRITE_BLOCK);
        bool   brpe   = mmisc_ctl_brpe_clear_and_get();

        soc_flash_mspi_wr_ram(SOC_FLASH_QUAD_PAGE_PROGRAM_CMD,
                              addr,
                              buf,
                              NULL,
                              wr_len,
                              false,
                              TLK_MSPI_WRITE,
                              SOC_FLASH_WRITE_ENABLE_CMD,
                              SOC_FLASH_READ_STATUS_CMD_LOWBYTE);
        mmisc_ctl_brpe_restore(brpe);
        len -= wr_len;
        addr += wr_len;
        buf = (const uint8_t*) buf + wr_len;
    }
    return 0;
}

TLK_STORAGE_DEVICE_DEFINE(soc_flash_0, SOC_FLASH_START_ADDR, SOC_FLASH_SIZE, SOC_FLASH_PAGE_SIZE,
                          SOC_FLASH_CLEAN_BYTE, soc_flash_read, soc_flash_erase, soc_flash_write,
                          NULL);

__attribute__((used, noinline)) static void soc_flash_register(void)
{
    tlk_storage_device_add(&soc_flash_0);
}

TLK_REGISTER_PRE_INIT(soc_flash_register, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_LOW);

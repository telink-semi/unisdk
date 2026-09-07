#include "tlk_mcuboot.h"

#include "bootutil/bootutil.h"
#include "bootutil/image.h"
#include "core/include/tlk_core.h"
#include "mcuboot_boot_init.h"
#include "mcuboot_config/mcuboot_logging.h"

#include <stdint.h>

#if TLK_IS_ENABLED(CONFIG_TLK_MCUBOOT_SERIAL_RECOVERY)
#include "uart_boot_serial.h"
#endif

typedef void (*app_entry_t)(void);

__attribute__((weak)) bool tlk_mcuboot_recovery_requested(void)
{
    return false;
}

__attribute__((weak)) void tlk_mcuboot_prepare_recovery(void) {}

/*Called when tlk_mcuboot_recovery_requested() returned true*/
static void enter_serial_recovery(void)
{
#if TLK_IS_ENABLED(CONFIG_TLK_MCUBOOT_SERIAL_RECOVERY)
    tlk_mcuboot_prepare_recovery();

    const struct boot_uart_funcs* f = tlk_mcuboot_serial_uart_init();

    MCUBOOT_LOG_INF("Entering serial recovery on UART%d", (int) CONFIG_TLK_MCUBOOT_SERIAL_UART);
    boot_serial_start(f);
#else
    MCUBOOT_LOG_WRN("%s",
                    "Serial recovery requested but not built in "
                    "(CONFIG_TLK_MCUBOOT_SERIAL_RECOVERY=n)");
#endif
}

static void jump_to_image(const struct boot_rsp* rsp)
{
    uintptr_t entry_addr = (uintptr_t) rsp->br_image_off + rsp->br_hdr->ih_hdr_size;

    MCUBOOT_LOG_INF("Booting image at 0x%08lx", (unsigned long) entry_addr);

    tlk_core_interrupt_disable();
    ((app_entry_t) entry_addr)();
}

void tlk_mcuboot_run(void)
{
    MCUBOOT_LOG_INF("%s", "Starting MCUboot");

    if (tlk_mcuboot_using_dev_key())
    {
        MCUBOOT_LOG_WRN("%s", "Development signing key in use -- not for production");
    }

    /* Must run before the recovery branch */
    int32_t crypto_status = tlk_mcuboot_crypto_init();

    if (crypto_status != 0)
    {
        MCUBOOT_LOG_ERR("psa_crypto_init() failed: %ld -- every image check will now fail",
                        (long) crypto_status);
    }

    if (tlk_mcuboot_recovery_requested())
    {
        enter_serial_recovery();
    }

    struct boot_rsp rsp;
    FIH_DECLARE(fih_rc, FIH_FAILURE);

    FIH_CALL(boot_go, fih_rc, &rsp);
    if (FIH_NOT_EQ(fih_rc, FIH_SUCCESS))
    {
        MCUBOOT_LOG_ERR("%s", "Unable to find bootable image");

        tlk_core_interrupt_disable();
        while (1)
        {
        }
    }

    jump_to_image(&rsp);

    while (1)
    {
    }
}

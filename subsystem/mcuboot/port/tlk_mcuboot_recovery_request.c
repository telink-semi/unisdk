#include "core/include/tlk_core.h"
#include "tlk_mcuboot_recovery.h"
#include <stdint.h>

extern uint8_t __tlk_boot_mailbox_start[];

static volatile struct tlk_mcuboot_mailbox* tlk_mcuboot_mailbox_get(void)
{
    return (volatile struct tlk_mcuboot_mailbox*) (uintptr_t) __tlk_boot_mailbox_start;
}

/*
 * Caller must reset the chip immediately after this returns.
 */
void tlk_mcuboot_request_recovery(void)
{
    volatile struct tlk_mcuboot_mailbox* mailbox = tlk_mcuboot_mailbox_get();

    mailbox->magic = 0;

    mailbox->magic_inv = ~TLK_MCUBOOT_RECOVERY_MAGIC;

    mailbox->magic = TLK_MCUBOOT_RECOVERY_MAGIC;

    tlk_fence_iorw;
}

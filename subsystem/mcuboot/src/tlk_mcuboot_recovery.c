#include "tlk_mcuboot_recovery.h"

#include <stdint.h>

extern uint8_t __tlk_boot_mailbox_start[];

static volatile struct tlk_mcuboot_mailbox* tlk_mcuboot_mailbox_get(void)
{
    return (volatile struct tlk_mcuboot_mailbox*) (uintptr_t) __tlk_boot_mailbox_start;
}

bool tlk_mcuboot_recovery_mailbox_pending(void)
{
    volatile struct tlk_mcuboot_mailbox* mailbox = tlk_mcuboot_mailbox_get();

    if (mailbox->magic != TLK_MCUBOOT_RECOVERY_MAGIC)
    {
        return false;
    }

    if (mailbox->magic_inv != ~TLK_MCUBOOT_RECOVERY_MAGIC)
    {
        return false;
    }

    mailbox->magic     = 0;
    mailbox->magic_inv = 0;

    return true;
}

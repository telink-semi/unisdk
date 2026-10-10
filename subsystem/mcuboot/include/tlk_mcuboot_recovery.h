#ifndef TLK_MCUBOOT_RECOVERY_H
#define TLK_MCUBOOT_RECOVERY_H

#include <stdbool.h>
#include <stdint.h>

#define TLK_MCUBOOT_MAILBOX_SIZE 16

#define TLK_MCUBOOT_RECOVERY_MAGIC UINT32_C(0x544C4B44)

struct tlk_mcuboot_mailbox
{
    uint32_t magic;
    uint32_t magic_inv;
};

_Static_assert(sizeof(struct tlk_mcuboot_mailbox) <= TLK_MCUBOOT_MAILBOX_SIZE,
               "tlk_mcuboot_mailbox is too large");

void tlk_mcuboot_request_recovery(void);

/*returns true and consumes the request if the app asked for recovery*/
bool tlk_mcuboot_recovery_mailbox_pending(void);

#endif /* TLK_MCUBOOT_RECOVERY_H */

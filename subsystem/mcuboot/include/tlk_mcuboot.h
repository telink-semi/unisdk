#ifndef TLK_MCUBOOT_H
#define TLK_MCUBOOT_H

#include <stdbool.h>

void tlk_mcuboot_run(void);

bool tlk_mcuboot_recovery_requested(void);

void tlk_mcuboot_prepare_recovery(void);

#endif /* TLK_MCUBOOT_H */

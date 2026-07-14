#ifndef TLK_INCLUDE_DRIVERS_PMP_H_
#define TLK_INCLUDE_DRIVERS_PMP_H_

#include "properties/tlk_pmp.h"

struct tlk_pmp_config
{
    uint8_t r        : 1;
    uint8_t w        : 1;
    uint8_t x        : 1;
    uint8_t reserved : 4;
    uint8_t l        : 1;
} __attribute__((packed));

#define TLK_DECLARE_PMP_ENTRY(num) TLK_PMP_ENTRY##num = num,
enum tlk_pmp_entry
{
    TLK_FOR_CALL(UNISDK_PMP_ENTRIES, TLK_DECLARE_PMP_ENTRY)
};

/**
 * @brief      This function serves to configure a PMP entry using the TOR
 *             (Top Of Range) address matching scheme.
 *             The protected region is defined as [pmpaddr[entry-1] ... address),
 *             where the lower bound is inclusive and the upper bound is exclusive.
 *             For entry 0, the lower bound is implicitly 0x00000000.
 * @param[in]  entry    PMP entry number (0-7) to configure.
 * @param[in]  address  Top (exclusive upper bound) of the protected address range.
 * @param[in]  config   Pointer to the PMP entry configuration (R/W/X/L permissions).
 * @return     None
 */
void tlk_pmp_tor_config(enum tlk_pmp_entry entry, void* address, struct tlk_pmp_config* config);

/**
 * @brief      This function serves to configure a PMP entry using the NAPOT
 *             (Naturally Aligned Power Of Two) address matching scheme.
 *             The protected region is defined as [address ... address + size).
 *             The base address must be naturally aligned to the given size,
 *             and the size must be a power of two, no less than 8 bytes.
 * @param[in]  entry    PMP entry number (0-7) to configure.
 * @param[in]  address  Base address of the protected region.
 *                      Must be aligned to @p size.
 * @param[in]  size     Size of the protected region in bytes.
 *                      Must be a power of two and at least 8 bytes.
 * @param[in]  config   Pointer to the PMP entry configuration (R/W/X/L permissions).
 * @return     None
 */
void tlk_pmp_napot_config(enum tlk_pmp_entry entry, void* address, uint64_t size,
                          struct tlk_pmp_config* config);

/**
 * @brief      This function serves to disable a PMP entry by setting its
 *             address matching mode to OFF, effectively removing any
 *             memory protection imposed by that entry.
 *             The address is retained in the pmpaddr register and can be
 *             used as a lower bound for a subsequent TOR entry.
 * @note       If the entry has been locked (L bit set), this function has
 *             no effect. The entry can only be unlocked by a system reset.
 * @param[in]  entry    PMP entry number (0-7) to disable.
 * @param[in]  address  Address to retain in the pmpaddr register.
 *                      Pass NULL if the address is not relevant.
 * @return     None
 */
void tlk_pmp_entry_disable(enum tlk_pmp_entry entry, void* address);
#endif

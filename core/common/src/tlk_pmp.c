#include "core/include/tlk_pmp.h"
#include "common/include/tlk_init.h"
#include "core/include/tlk_core.h"
#include "registers/tlk_core_reg.h"

#define SCHEME_NAPOT 3
#define SCHEME_NA4   2
#define SCHEME_TOR   1
#define SCHEME_OFF   0

/* ==== RETENTION HELPERS ==== */
#ifdef CONFIG_TLK_PMP_PM_DEVICE
static uint32_t tlk_pmp_config_retention[2];
static uint32_t tlk_pmp_address_retention[UNISDK_PMP_ENTRIES];
#endif
/* ==== RETENTION HELPERS ==== */

/* ==== HELPERS ==== */
#define NAPOT(base, size)                                                                          \
    (uint32_t) (((size) > 0) ? ((((uint32_t) (base) & (~((uint32_t) (size) - 1))) >> 2) |          \
                                (((uint64_t) (size) - 1) >> 3))                                    \
                             : 0)

static void tlk_pmp_write_config(enum tlk_pmp_entry entry, uint8_t config)
{
    // check in which register the setting of this pmpm entry is located
    switch (entry >> 2)
    {
    case 0:
        tlk_write_csr(NDS_PMPCFG0,
                      ((tlk_read_csr(NDS_PMPCFG0) & (~((0xFF) << ((entry % 4) << 3)))) |
                       (((long) config) << ((entry % 4) << 3))));
        break;
    case 1:
        tlk_write_csr(NDS_PMPCFG1,
                      ((tlk_read_csr(NDS_PMPCFG1) & (~((0xFF) << ((entry % 4) << 3)))) |
                       (((long) config) << ((entry % 4) << 3))));
        break;
    }
}

static void tlk_pmp_write_address(enum tlk_pmp_entry entry, uint32_t address)
{
    switch (entry)
    {
    case 0:
        tlk_write_csr(NDS_PMPADDR0, address);
        break;
    case 1:
        tlk_write_csr(NDS_PMPADDR1, address);
        break;
    case 2:
        tlk_write_csr(NDS_PMPADDR2, address);
        break;
    case 3:
        tlk_write_csr(NDS_PMPADDR3, address);
        break;
    case 4:
        tlk_write_csr(NDS_PMPADDR4, address);
        break;
    case 5:
        tlk_write_csr(NDS_PMPADDR5, address);
        break;
    case 6:
        tlk_write_csr(NDS_PMPADDR6, address);
        break;
    case 7:
        tlk_write_csr(NDS_PMPADDR7, address);
        break;
    }
}
/* ==== HELPERS ==== */

/* ==== DRIVER APIs ==== */
void tlk_pmp_configure_tor(enum tlk_pmp_entry entry, void* address, struct tlk_pmp_config* config)
{
    uint8_t pmpcfg = *(uint8_t*) config;
    pmpcfg |= SCHEME_TOR << 3; // Enable the TOR mode. A[4:3] reserved[6:5]

    // set the entry address
    tlk_pmp_write_address(entry, (uint32_t) address >> 2);

    // set the entry config
    tlk_pmp_write_config(entry, pmpcfg);

    tlk_fence_iorw;
}

void tlk_pmp_configure_napot(enum tlk_pmp_entry entry, void* address, uint64_t size,
                             struct tlk_pmp_config* config)
{
    uint8_t pmpcfg = *(uint8_t*) config;
    pmpcfg |= SCHEME_NAPOT << 3; // Enable the NAPOT mode. A[4:3] reserved[6:5]

    // set the entry address
    tlk_pmp_write_address(entry, NAPOT(address, size));

    // set the entry config
    tlk_pmp_write_config(entry, pmpcfg);

    tlk_fence_iorw;
}

void tlk_pmp_disable_entry(enum tlk_pmp_entry entry, void* address)
{
    // set the entry address
    tlk_pmp_write_address(entry, (uint32_t) address >> 2);

    // set the entry config
    tlk_pmp_write_config(entry, 0);

    tlk_fence_iorw;
}
/* ==== DRIVER APIs ==== */

/* ==== PM DEVICE ==== */
#ifdef CONFIG_TLK_PMP_PM_DEVICE

static void tlk_pmp_save(void)
{
    tlk_pmp_config_retention[0] = tlk_read_csr(NDS_PMPCFG0);
    tlk_pmp_config_retention[1] = tlk_read_csr(NDS_PMPCFG1);

    tlk_pmp_address_retention[0] = tlk_read_csr(NDS_PMPADDR0);
    tlk_pmp_address_retention[1] = tlk_read_csr(NDS_PMPADDR1);
    tlk_pmp_address_retention[2] = tlk_read_csr(NDS_PMPADDR2);
    tlk_pmp_address_retention[3] = tlk_read_csr(NDS_PMPADDR3);
    tlk_pmp_address_retention[4] = tlk_read_csr(NDS_PMPADDR4);
    tlk_pmp_address_retention[5] = tlk_read_csr(NDS_PMPADDR5);
    tlk_pmp_address_retention[6] = tlk_read_csr(NDS_PMPADDR6);
    tlk_pmp_address_retention[7] = tlk_read_csr(NDS_PMPADDR7);
}

static void tlk_pmp_restore(void)
{
    tlk_write_csr(NDS_PMPADDR0, tlk_pmp_address_retention[0]);
    tlk_write_csr(NDS_PMPADDR1, tlk_pmp_address_retention[1]);
    tlk_write_csr(NDS_PMPADDR2, tlk_pmp_address_retention[2]);
    tlk_write_csr(NDS_PMPADDR3, tlk_pmp_address_retention[3]);
    tlk_write_csr(NDS_PMPADDR4, tlk_pmp_address_retention[4]);
    tlk_write_csr(NDS_PMPADDR5, tlk_pmp_address_retention[5]);
    tlk_write_csr(NDS_PMPADDR6, tlk_pmp_address_retention[6]);
    tlk_write_csr(NDS_PMPADDR7, tlk_pmp_address_retention[7]);

    tlk_write_csr(NDS_PMPCFG0, tlk_pmp_config_retention[0]);
    tlk_write_csr(NDS_PMPCFG1, tlk_pmp_config_retention[1]);
}

TLK_REGISTER_BEFORE_SUSPEND(tlk_pmp_save, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_HIGH)
TLK_REGISTER_BEFORE_SLEEP(tlk_pmp_save, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_HIGH)

TLK_REGISTER_AFTER_SUSPEND(tlk_pmp_restore, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_HIGH)
TLK_REGISTER_AFTER_SLEEP(tlk_pmp_restore, TLK_INIT_LEVEL_DRIVER, TLK_INIT_PRIORITY_HIGH)

#endif
/* ==== PM DEVICE ==== */

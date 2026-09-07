#ifndef PMP_DRIVER_HPP
#define PMP_DRIVER_HPP

#ifdef __cplusplus
extern "C"
{
#endif
#include "core/include/tlk_pmp.h"
#ifdef __cplusplus
}
#endif

class PMPDriver
{
  public:
    explicit PMPDriver(tlk_pmp_entry entry_, void* address_) : entry(entry_), address(address_) {}

    void torConfig(tlk_pmp_config* config)
    {
        tlk_pmp_tor_config(entry, address, config);
    }

    void napotConfig(uint64_t size, tlk_pmp_config* config)
    {
        tlk_pmp_napot_config(entry, address, size, config);
    }

    void entryDisable()
    {
        tlk_pmp_entry_disable(entry, address);
    }

  private:
    tlk_pmp_entry entry;
    void*         address;
};

#endif // PMP_DRIVER_HPP

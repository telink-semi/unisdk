#ifndef TLK_INCLUDE_DRIVERS_WATCHDOG_H_
#define TLK_INCLUDE_DRIVERS_WATCHDOG_H_

void tlk_wdt_32k_start(unsigned int timeout);
void tlk_wdt_32k_stop(void);

#endif
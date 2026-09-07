#ifndef MCUBOOT_LOGGING_H
#define MCUBOOT_LOGGING_H

#include "system/debug/log/tlk_log.h"

TLK_LOG_CREATE(mcuboot_log, "MCUBOOT");

#define MCUBOOT_LOG_MODULE_DECLARE(domain)
#define MCUBOOT_LOG_MODULE_REGISTER(domain)

#define MCUBOOT_LOG_ERR(...) TLK_LOG_ERROR(mcuboot_log, __VA_ARGS__)
#define MCUBOOT_LOG_WRN(...) TLK_LOG_WARN(mcuboot_log, __VA_ARGS__)
#define MCUBOOT_LOG_INF(...) TLK_LOG_INFO(mcuboot_log, __VA_ARGS__)
#define MCUBOOT_LOG_DBG(...) TLK_LOG_DEBUG(mcuboot_log, __VA_ARGS__)

#endif /* MCUBOOT_LOGGING_H */

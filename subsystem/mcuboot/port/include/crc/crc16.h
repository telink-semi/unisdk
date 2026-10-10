#ifndef CRC_CRC16_H
#define CRC_CRC16_H

#include <stddef.h>
#include <stdint.h>

#define CRC16_INITIAL_CRC 0

uint16_t crc16_ccitt(uint16_t crc, const void* buf, size_t len);

#endif /* CRC_CRC16_H */

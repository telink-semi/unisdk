#include "crc/crc16.h"

uint16_t crc16_ccitt(uint16_t crc, const void* buf, size_t len)
{
    const uint8_t* p = (const uint8_t*) buf;

    while (len--)
    {
        crc ^= (uint16_t) (*p++) << 8;
        for (int i = 0; i < 8; i++)
        {
            crc = (crc & 0x8000) ? (uint16_t) ((crc << 1) ^ 0x1021) : (uint16_t) (crc << 1);
        }
    }

    return crc;
}

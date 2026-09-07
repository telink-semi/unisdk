#include "common/include/tlk_str_utils.h"

static char tlk_str_utils_convert_byte_to_char(uint8_t bt, bool high)
{
    if (high)
    {
        bt >>= 4;
    }
    else
    {
        bt &= 0xf;
    }
    if (bt < 0xa)
    {
        return '0' + bt;
    }
    else
    {
        return 'a' + bt - 0xa;
    }
}

int tlk_str_utils_convert_buf_to_hex(char* dst, size_t dst_len, const void* src, size_t src_len)
{
    int result = -1;

    if (dst_len)
    {
        result = 0;
        for (size_t i = 0; i < src_len; ++i)
        {
            if ((size_t) result + 3 >= dst_len)
            {
                break;
            }
            dst[result]     = tlk_str_utils_convert_byte_to_char(((const uint8_t*) src)[i], true);
            dst[result + 1] = tlk_str_utils_convert_byte_to_char(((const uint8_t*) src)[i], false);
            result += 2;
        }
        dst[result] = 0;
    }
    return result;
}

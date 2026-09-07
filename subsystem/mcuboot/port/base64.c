#include "base64/base64.h"

#include <stdbool.h>
#include <stdint.h>

static const char b64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz"
                                   "0123456789+/";

int base64_encode(const void* in, int len, char* out, int add_null_term)
{
    const uint8_t* src  = (const uint8_t*) in;
    int            olen = 0;
    int            i;

    for (i = 0; i + 3 <= len; i += 3)
    {
        uint32_t v = ((uint32_t) src[i] << 16) | ((uint32_t) src[i + 1] << 8) | src[i + 2];

        out[olen++] = b64_alphabet[(v >> 18) & 0x3F];
        out[olen++] = b64_alphabet[(v >> 12) & 0x3F];
        out[olen++] = b64_alphabet[(v >> 6) & 0x3F];
        out[olen++] = b64_alphabet[v & 0x3F];
    }

    int rem = len - i;

    if (rem == 1)
    {
        uint32_t v = (uint32_t) src[i] << 16;

        out[olen++] = b64_alphabet[(v >> 18) & 0x3F];
        out[olen++] = b64_alphabet[(v >> 12) & 0x3F];
        out[olen++] = '=';
        out[olen++] = '=';
    }
    else if (rem == 2)
    {
        uint32_t v = ((uint32_t) src[i] << 16) | ((uint32_t) src[i + 1] << 8);

        out[olen++] = b64_alphabet[(v >> 18) & 0x3F];
        out[olen++] = b64_alphabet[(v >> 12) & 0x3F];
        out[olen++] = b64_alphabet[(v >> 6) & 0x3F];
        out[olen++] = '=';
    }

    if (add_null_term)
    {
        out[olen] = '\0';
    }

    return olen;
}

static int b64_val(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c - 'A';
    }
    if (c >= 'a' && c <= 'z')
    {
        return c - 'a' + 26;
    }
    if (c >= '0' && c <= '9')
    {
        return c - '0' + 52;
    }
    if (c == '+')
    {
        return 62;
    }
    if (c == '/')
    {
        return 63;
    }
    return -1;
}

static int b64_strlen(const char* in)
{
    int len = 0;

    while (in[len] != '\0')
    {
        len++;
    }
    return len;
}

int base64_decode_len(const char* in)
{
    int len = b64_strlen(in);
    int pad = 0;

    if (len == 0 || (len % 4) != 0)
    {
        return -1;
    }

    if (in[len - 1] == '=')
    {
        pad++;
    }
    if (in[len - 2] == '=')
    {
        pad++;
    }

    return (len / 4) * 3 - pad;
}

int base64_decode(const char* in, void* out)
{
    uint8_t* dst  = (uint8_t*) out;
    int      olen = 0;
    int      len  = b64_strlen(in);

    if (len == 0 || (len % 4) != 0)
    {
        return -1;
    }

    for (int i = 0; i < len; i += 4)
    {
        bool pad2 = in[i + 2] == '=';
        bool pad3 = in[i + 3] == '=';
        int  v0   = b64_val(in[i]);
        int  v1   = b64_val(in[i + 1]);
        int  v2   = pad2 ? 0 : b64_val(in[i + 2]);
        int  v3   = pad3 ? 0 : b64_val(in[i + 3]);

        if (v0 < 0 || v1 < 0 || (!pad2 && v2 < 0) || (!pad3 && v3 < 0))
        {
            return -1;
        }

        uint32_t triple =
            ((uint32_t) v0 << 18) | ((uint32_t) v1 << 12) | ((uint32_t) v2 << 6) | (uint32_t) v3;

        dst[olen++] = (uint8_t) (triple >> 16);
        if (!pad2)
        {
            dst[olen++] = (uint8_t) (triple >> 8);
        }
        if (!pad3)
        {
            dst[olen++] = (uint8_t) triple;
        }
    }

    return olen;
}

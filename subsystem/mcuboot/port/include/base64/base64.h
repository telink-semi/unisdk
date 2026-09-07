#ifndef BASE64_BASE64_H
#define BASE64_BASE64_H

#define BASE64_ENCODE_SIZE(in_size) ((((((in_size) - 1) / 3) * 4) + 4) + 1)

int base64_encode(const void* in, int len, char* out, int add_null_term);

int base64_decode_len(const char* in);

int base64_decode(const char* in, void* out);

#endif /* BASE64_BASE64_H */

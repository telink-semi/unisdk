#ifndef TLK_ASSERT_H_
#define TLK_ASSERT_H_

void __assert_func(const char* file, uint32_t line, const char* func, const char* expr);

#define tlk_assert(e) ((e) ? (void) 0 : __assert_func(__FILE__, __LINE__, __func__, #e))

#endif

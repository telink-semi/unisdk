/********************************************************************************************************
 * @file    tlk_bit.h
 *
 * @brief   This is the header file for Telink RISC-V MCU
 *
 * @author  Driver Group
 * @date    2019
 *
 * @par     Copyright (c) 2019, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#ifndef TLK_BIT_H_
#define TLK_BIT_H_

#define TLK_BIT(n)                 (1 << (n))
#define TLK_BIT_MASK_LEN(len)      (TLK_BIT(len) - 1)
#define TLK_BIT_RNG(s, e)          (TLK_BIT_MASK_LEN((e) - (s) + 1) << (s))

#define TLK_BM_CLR_MASK_V(x, mask) ((x) & ~(mask))


#define TLK_BM_SET(x, mask)        ((x) |= (mask))
#define TLK_BM_CLR(x, mask)        ((x) &= ~(mask))
#define TLK_BM_IS_SET(x, mask)     ((x) & (mask))
#define TLK_BM_IS_CLR(x, mask)     ((~x) & (mask))
#define TLK_BM_FLIP(x, mask)       ((x) ^= (mask))

/**
 *  define Reg operations
 */

// Return the bit index of the lowest 1 in y.   ex:  0b00110111000  --> 3
#define TLK_BIT_LOW_BIT(y) (((y) & TLK_BIT(0)) ? 0 : (((y) & TLK_BIT(1)) ? 1 : (((y) & TLK_BIT(2)) ? 2 : (((y) & TLK_BIT(3)) ? 3 : (((y) & TLK_BIT(4)) ? 4 : (((y) & TLK_BIT(5)) ? 5 : (((y) & TLK_BIT(6)) ? 6 : (((y) & TLK_BIT(7)) ? 7 : (((y) & TLK_BIT(8)) ? 8 : (((y) & TLK_BIT(9)) ? 9 : (((y) & TLK_BIT(10)) ? 10 : (((y) & TLK_BIT(11)) ? 11 : (((y) & TLK_BIT(12)) ? 12 : (((y) & TLK_BIT(13)) ? 13 : (((y) & TLK_BIT(14)) ? 14 : (((y) & TLK_BIT(15)) ? 15 : (((y) & TLK_BIT(16)) ? 16 : (((y) & TLK_BIT(17)) ? 17 : (((y) & TLK_BIT(18)) ? 18 : (((y) & TLK_BIT(19)) ? 19 : (((y) & TLK_BIT(20)) ? 20 : (((y) & TLK_BIT(21)) ? 21 : (((y) & TLK_BIT(22)) ? 22 : (((y) & TLK_BIT(23)) ? 23 : (((y) & TLK_BIT(24)) ? 24 : (((y) & TLK_BIT(25)) ? 25 : (((y) & TLK_BIT(26)) ? 26 : (((y) & TLK_BIT(27)) ? 27 : (((y) & TLK_BIT(28)) ? 28 : (((y) & TLK_BIT(29)) ? 29 : (((y) & TLK_BIT(30)) ? 30 : (((y) & TLK_BIT(31)) ? 31 : 32))))))))))))))))))))))))))))))))

// Return the bit index of the highest 1 in (y).   ex:  0b00110111000  --> 8
#define TLK_BIT_HIGH_BIT(y)                                                                                                     (((y) & TLK_BIT(31)) ? 31 : (((y) & TLK_BIT(30)) ? 30 : (((y) & TLK_BIT(29)) ? 29 : (((y) & TLK_BIT(28)) ? 28 : (((y) & TLK_BIT(27)) ? 27 : (((y) & TLK_BIT(26)) ? 26 : (((y) & TLK_BIT(25)) ? 25 : (((y) & TLK_BIT(24)) ? 24 : (((y) & TLK_BIT(23)) ? 23 : (((y) & TLK_BIT(22)) ? 22 : (((y) & TLK_BIT(21)) ? 21 : (((y) & TLK_BIT(20)) ? 20 : (((y) & TLK_BIT(19)) ? 19 : (((y) & TLK_BIT(18)) ? 18 : (((y) & TLK_BIT(17)) ? 17 : (((y) & TLK_BIT(16)) ? 16 : (((y) & TLK_BIT(15)) ? 15 : (((y) & TLK_BIT(14)) ? 14 : (((y) & TLK_BIT(13)) ? 13 : (((y) & TLK_BIT(12)) ? 12 : (((y) & TLK_BIT(11)) ? 11 : (((y) & TLK_BIT(10)) ? 10 : (((y) & TLK_BIT(9)) ? 9 : (((y) & TLK_BIT(8)) ? 8 : (((y) & TLK_BIT(7)) ? 7 : (((y) & TLK_BIT(6)) ? 6 : (((y) & TLK_BIT(5)) ? 5 : (((y) & TLK_BIT(4)) ? 4 : (((y) & TLK_BIT(3)) ? 3 : (((y) & TLK_BIT(2)) ? 2 : (((y) & TLK_BIT(1)) ? 1 : (((y) & TLK_BIT(0)) ? 0 : 32))))))))))))))))))))))))))))))))

#define TLK_COUNT_ARGS_IMPL2(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, N, ...) N
#define TLK_COUNT_ARGS_IMPL(args)                                                                                               COUNT_ARGS_IMPL2 args
#define TLK_COUNT_ARGS(...)                                                                                                     TLK_COUNT_ARGS_IMPL((__VA_ARGS__, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))

#define TLK_MACRO_CHOOSE_HELPER2(base, count)                                                                                   base##count
#define TLK_MACRO_CHOOSE_HELPER1(base, count)                                                                                   TLK_MACRO_CHOOSE_HELPER2(base, count)
#define TLK_MACRO_CHOOSE_HELPER(base, count)                                                                                    TLK_MACRO_CHOOSE_HELPER1(base, count)

#define TLK_MACRO_GLUE(x, y)                                                                                                    x y
#define TLK_VARARG(base, ...)                                                                                                   TLK_MACRO_GLUE(TLK_MACRO_CHOOSE_HELPER(base, TLK_COUNT_ARGS(__VA_ARGS__)), (__VA_ARGS__))

#define TLK_MV(m, v)                                                                                                            (((v) << TLK_BIT_LOW_BIT(m)) & (m))

// warning MASK_VALn  are internal used macro, please use MASK_VAL instead
#define TLK_MASK_VAL2(m, v)                                                            (TLK_MV(m, v))
#define TLK_MASK_VAL4(m1, v1, m2, v2)                                                  (TLK_MV(m1, v1) | TLK_MV(m2, v2))
#define TLK_MASK_VAL6(m1, v1, m2, v2, m3, v3)                                          (TLK_MV(m1, v1) | TLK_MV(m2, v2) | TLK_MV(m3, v3))
#define TLK_MASK_VAL8(m1, v1, m2, v2, m3, v3, m4, v4)                                  (TLK_MV(m1, v1) | TLK_MV(m2, v2) | TLK_MV(m3, v3) | TLK_MV(m4, v4))
#define TLK_MASK_VAL10(m1, v1, m2, v2, m3, v3, m4, v4, m5, v5)                         (TLK_MV(m1, v1) | TLK_MV(m2, v2) | TLK_MV(m3, v3) | TLK_MV(m4, v4) | TLK_MV(m5, v5))
#define TLK_MASK_VAL12(m1, v1, m2, v2, m3, v3, m4, v4, m5, v5, m6, v6)                 (TLK_MV(m1, v1) | TLK_MV(m2, v2) | TLK_MV(m3, v3) | TLK_MV(m4, v4) | TLK_MV(m5, v5) | TLK_MV(m6, v6))
#define TLK_MASK_VAL14(m1, v1, m2, v2, m3, v3, m4, v4, m5, v5, m6, v6, m7, v7)         (TLK_MV(m1, v1) | TLK_MV(m2, v2) | TLK_MV(m3, v3) | TLK_MV(m4, v4) | TLK_MV(m5, v5) | TLK_MV(m6, v6) | TLK_MV(m7, v7))
#define TLK_MASK_VAL16(m1, v1, m2, v2, m3, v3, m4, v4, m5, v5, m6, v6, m7, v7, m8, v8) (TLK_MV(m1, v1) | TLK_MV(m2, v2) | TLK_MV(m3, v3) | TLK_MV(m4, v4) | TLK_MV(m5, v5) | TLK_MV(m6, v6) | TLK_MV(m7, v7) | TLK_MV(m8, v8))

#define TLK_MASK_VAL(...)                                                              TLK_VARARG(MASK_VAL, __VA_ARGS__)


#endif

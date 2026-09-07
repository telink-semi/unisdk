/********************************************************************************************************
 * @file    tlk_compiler.h
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
#ifndef TLK_COMPILER_H_
#define TLK_COMPILER_H_

#define _tlk_attribute_ram_code_sec_ __attribute__((section(".ram_code")))
#define _tlk_attribute_ram_code_sec_noinline_                                                      \
    __attribute__((section(".ram_code"))) __attribute__((noinline))

#define _tlk_attribute_text_sec_noinline_                                                          \
    __attribute__((section(".text"))) __attribute__((noinline))

/**
 * Use this macro carefully. The always_inline attribute is a strong request to the compiler to
 * inline the function, but it does not guarantee inlining in every situation. In general, without
 * LTO, the function body must be visible in the same translation unit for the compiler to inline
 * it. With LTO enabled (-flto), the compiler can inline functions across translation units. Prefer
 * relying on the compiler's normal optimization whenever possible. With optimization enabled, the
 * compiler will generally inline functions when it determines that doing so is beneficial.
 * Therefore, always_inline should only be used when there is a specific reason to require inlining.
 * For PM-related functions and similar functions where execution from flash isn't possible, prefer
 * using the ram_code attribute instead.
 */
#define _tlk_always_inline_ inline __attribute__((always_inline))

#define _tlk_attribute_data_retention_sec_ __attribute__((section(".retention_data")))

#define _tlk_attribute_aes_data_sec_ __attribute__((section(".aes_data")))

#endif

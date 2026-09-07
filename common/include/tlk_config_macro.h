/*
 * Copyright (c) 2011-2014, Wind River Systems, Inc.
 * Copyright (c) 2020, Nordic Semiconductor ASA
 * Copyright (c) 2025, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*
 * The util macros to work with the defined configurations.
 * Taken from the Zephyr project.
 */

#include <stdbool.h>

/* TLK_IS_ENABLED and TLK_IS_DISABLED */

/**
 * @brief Check for macro definition in compiler-visible expressions
 *
 * This trick was pioneered in Linux as the config_enabled() macro.
 * The madness has the effect of taking a macro value that may be
 * defined to "1" (e.g. CONFIG_MYFEATURE), or may not be defined at
 * all and turning it into a literal expression that can be used at
 * "runtime".  That is, it works similarly to
 * "defined(CONFIG_MYFEATURE)" does except that it is an expansion
 * that can exist in a standard expression and be seen by the compiler
 * and optimizer.  Thus much ifdef usage can be replaced with cleaner
 * expressions like:
 *
 *     if (TLK_IS_ENABLED(CONFIG_MYFEATURE))
 *             myfeature_enable();
 *
 * INTERNAL
 * First pass just to expand any existing macros, we need the macro
 * value to be e.g. a literal "1" at expansion time in the next macro,
 * not "(1)", etc...  Standard recursive expansion does not work.
 */
#define TLK_IS_ENABLED(config_macro) _TLK_IS_ENABLED1(config_macro)

/**
 * TLK_IS_DISABLED does the opposite of TLK_IS_ENABLED
 */
#define TLK_IS_DISABLED(config_macro) !_TLK_IS_ENABLED1(config_macro)

/* Now stick on a "_TLK_XXXX" prefix, it will now be "_TLK_XXXX1" if config_macro
 * is "1", or just "_TLK_XXXX" if it's undefined.
 *   ENABLED:   Z_TLK_IS_ENABLED2(_TLK_XXXX1)
 *   DISABLED   Z_TLK_IS_ENABLED2(_TLK_XXXX)
 */
#define _TLK_IS_ENABLED1(config_macro) _TLK_IS_ENABLED2(_TLK_XXXX##config_macro)

/* Here's the core trick, we map "_TLK_XXXX1" to "_YYYY," (i.e. a string
 * with a trailing comma), so it has the effect of making this a
 * two-argument tuple to the preprocessor only in the case where the
 * value is defined to "1"
 *   ENABLED:    _YYYY,    <--- note comma!
 *   DISABLED:   _TLK_XXXX
 */
#define _TLK_XXXX1 _YYYY,

/* Then we append an extra argument to fool the gcc preprocessor into
 * accepting it as a varargs macro.
 *                         arg1   arg2  arg3
 *   ENABLED:   Z_TLK_IS_ENABLED3(_YYYY,    1,    0)
 *   DISABLED   Z_TLK_IS_ENABLED3(_TLK_XXXX 1,  0)
 */
#define _TLK_IS_ENABLED2(one_or_two_args) _TLK_IS_ENABLED3(one_or_two_args true, false)

/* And our second argument is thus now cooked to be 1 in the case
 * where the value is defined to 1, and 0 if not:
 */
#define _TLK_IS_ENABLED3(ignore_this, val, ...) val

/* TLK_IF_ENABLED and TLK_IF_DISABLED */

/**
 * @brief Insert code if @p _flag is defined and equals 1.
 *
 * Like TLK_COND_CODE_1(), this expands to @p _code if @p _flag is defined to 1;
 * it expands to nothing otherwise.
 *
 * Example:
 *
 *     TLK_IF_ENABLED(CONFIG_FLAG, (uint32_t foo;))
 *
 * If @p CONFIG_FLAG is defined to 1, this expands to:
 *
 *     uint32_t foo;
 *
 * and to nothing otherwise.
 *
 * It can be considered as a more compact alternative to:
 *
 *     #if defined(CONFIG_FLAG) && (CONFIG_FLAG == 1)
 *     uint32_t foo;
 *     #endif
 *
 * @param _flag evaluated flag
 * @param _code result if @p _flag expands to 1; must be in parentheses
 */
#define TLK_IF_ENABLED(_flag, _code) TLK_COND_CODE_1(_flag, _code, ())

/**
 * @brief Insert code if @p _flag is not defined as 1.
 *
 * This expands to nothing if @p _flag is defined and equal to 1;
 * it expands to @p _code otherwise.
 *
 * Example:
 *
 *     TLK_IF_DISABLED(CONFIG_FLAG, (uint32_t foo;))
 *
 * If @p CONFIG_FLAG isn't defined or different than 1, this expands to:
 *
 *     uint32_t foo;
 *
 * and to nothing otherwise.
 *
 * TLK_IF_DISABLED does the opposite of TLK_IF_ENABLED.
 *
 * @param _flag evaluated flag
 * @param _code result if @p _flag does not expand to 1; must be in parentheses
 */
#define TLK_IF_DISABLED(_flag, _code) TLK_COND_CODE_1(_flag, (), _code)

/**
 * @brief Insert code depending on whether @p _flag expands to 1 or not.
 *
 * This relies on similar tricks as TLK_IS_ENABLED(), but as the result of
 * @p _flag expansion, results in either @p _if_1_code or @p
 * _else_code is expanded.
 *
 * To prevent the preprocessor from treating commas as argument
 * separators, the @p _if_1_code and @p _else_code expressions must be
 * inside brackets/parentheses: <tt>()</tt>. These are stripped away
 * during macro expansion.
 *
 * Example:
 *
 *     TLK_COND_CODE_1(CONFIG_FLAG, (uint32_t x;), (there_is_no_flag();))
 *
 * If @p CONFIG_FLAG is defined to 1, this expands to:
 *
 *     uint32_t x;
 *
 * It expands to <tt>there_is_no_flag();</tt> otherwise.
 *
 * This could be used as an alternative to:
 *
 *     #if defined(CONFIG_FLAG) && (CONFIG_FLAG == 1)
 *     #define MAYBE_DECLARE(x) uint32_t x
 *     #else
 *     #define MAYBE_DECLARE(x) there_is_no_flag()
 *     #endif
 *
 *     MAYBE_DECLARE(x);
 *
 * However, the advantage of TLK_COND_CODE_1() is that code is resolved in
 * place where it is used, while the @p \#if method defines @p
 * MAYBE_DECLARE on two lines and requires it to be invoked again on a
 * separate line. This makes TLK_COND_CODE_1() more concise and also
 * sometimes more useful when used within another macro's expansion.
 *
 * @note @p _flag can be the result of preprocessor expansion, e.g.
 *	 an expression involving <tt>NUM_VA_ARGS_LESS_1(...)</tt>.
 *	 However, @p _if_1_code is only expanded if @p _flag expands
 *	 to the integer literal 1. Integer expressions that evaluate
 *	 to 1, e.g. after doing some arithmetic, will not work.
 *
 * @param _flag evaluated flag
 * @param _if_1_code result if @p _flag expands to 1; must be in parentheses
 * @param _else_code result otherwise; must be in parentheses
 */
#define TLK_COND_CODE_1(_flag, _if_1_code, _else_code)                                             \
    Z_TLK_COND_CODE_1(_flag, _if_1_code, _else_code)

/**
 * @brief Like TLK_COND_CODE_1() except tests if @p _flag is 0.
 *
 * This is like TLK_COND_CODE_1(), except that it tests whether @p _flag
 * expands to the integer literal 0. It expands to @p _if_0_code if
 * so, and @p _else_code otherwise; both of these must be enclosed in
 * parentheses.
 *
 * @param _flag evaluated flag
 * @param _if_0_code result if @p _flag expands to 0; must be in parentheses
 * @param _else_code result otherwise; must be in parentheses
 * @see TLK_COND_CODE_1()
 */
#define TLK_COND_CODE_0(_flag, _if_0_code, _else_code)                                             \
    Z_TLK_COND_CODE_0(_flag, _if_0_code, _else_code)

/* Used internally by TLK_COND_CODE_1 and TLK_COND_CODE_0. */
#define Z_TLK_COND_CODE_1(_flag, _if_1_code, _else_code)                                           \
    _TLK_COND_CODE(_TLK_XXXX##_flag, _if_1_code, _else_code)
#define Z_TLK_COND_CODE_0(_flag, _if_0_code, _else_code)                                           \
    _TLK_COND_CODE(_TLK_ZZZZ##_flag, _if_0_code, _else_code)
#define _TLK_ZZZZ0 _YYYY,
#define _TLK_COND_CODE(one_or_two_args, _if_code, _else_code)                                      \
    _TLK_GET_ARG2_DEBRACKET(one_or_two_args _if_code, _else_code)

/* Gets second argument and removes brackets around that argument. It
 * is expected that the parameter is provided in brackets/parentheses.
 */
#define _TLK_GET_ARG2_DEBRACKET(ignore_this, val, ...) TLK_DEBRACKET val

/* Used to remove brackets from around a single argument. */
#define TLK_DEBRACKET(...) __VA_ARGS__

/* TLK_IF_ENABLED_ELSE and TLK_IF_DISABLED_ELSE */

/**
 * TLK_IF_ENABLED_ELSE does the same as TLK_IF_ENABLED, but with the else option
 */
#define TLK_IF_ENABLED_ELSE(_flag, _if_code, _else_code)                                           \
    TLK_COND_CODE_1(_flag, _if_code, _else_code)

/**
 * TLK_IF_DISABLED_ELSE does the same as TLK_IF_DISABLED, but with the else option
 */
#define TLK_IF_DISABLED_ELSE(_flag, _if_code, _else_code)                                          \
    TLK_COND_CODE_1(_flag, _else_code, _if_code)

/* FOR LOOPS */
#define TLK_REPEAT_CALL_1(M)  M(0)
#define TLK_REPEAT_CALL_2(M)  TLK_REPEAT_CALL_1(M) M(1)
#define TLK_REPEAT_CALL_3(M)  TLK_REPEAT_CALL_2(M) M(2)
#define TLK_REPEAT_CALL_4(M)  TLK_REPEAT_CALL_3(M) M(3)
#define TLK_REPEAT_CALL_5(M)  TLK_REPEAT_CALL_4(M) M(4)
#define TLK_REPEAT_CALL_6(M)  TLK_REPEAT_CALL_5(M) M(5)
#define TLK_REPEAT_CALL_7(M)  TLK_REPEAT_CALL_6(M) M(6)
#define TLK_REPEAT_CALL_8(M)  TLK_REPEAT_CALL_7(M) M(7)
#define TLK_REPEAT_CALL_9(M)  TLK_REPEAT_CALL_8(M) M(8)
#define TLK_REPEAT_CALL_10(M) TLK_REPEAT_CALL_9(M) M(9)
#define TLK_REPEAT_CALL_11(M) TLK_REPEAT_CALL_10(M) M(10)
#define TLK_REPEAT_CALL_12(M) TLK_REPEAT_CALL_11(M) M(11)
#define TLK_REPEAT_CALL_13(M) TLK_REPEAT_CALL_12(M) M(12)
#define TLK_REPEAT_CALL_14(M) TLK_REPEAT_CALL_13(M) M(13)
#define TLK_REPEAT_CALL_15(M) TLK_REPEAT_CALL_14(M) M(14)
#define TLK_REPEAT_CALL_16(M) TLK_REPEAT_CALL_15(M) M(15)
#define TLK_REPEAT_CALL_17(M) TLK_REPEAT_CALL_16(M) M(16)
#define TLK_REPEAT_CALL_18(M) TLK_REPEAT_CALL_17(M) M(17)
#define TLK_REPEAT_CALL_19(M) TLK_REPEAT_CALL_18(M) M(18)
#define TLK_REPEAT_CALL_20(M) TLK_REPEAT_CALL_19(M) M(19)
#define TLK_REPEAT_CALL_21(M) TLK_REPEAT_CALL_20(M) M(20)
#define TLK_REPEAT_CALL_22(M) TLK_REPEAT_CALL_21(M) M(21)
#define TLK_REPEAT_CALL_23(M) TLK_REPEAT_CALL_22(M) M(22)
#define TLK_REPEAT_CALL_24(M) TLK_REPEAT_CALL_23(M) M(23)
#define TLK_REPEAT_CALL_25(M) TLK_REPEAT_CALL_24(M) M(24)
#define TLK_REPEAT_CALL_26(M) TLK_REPEAT_CALL_25(M) M(25)
#define TLK_REPEAT_CALL_27(M) TLK_REPEAT_CALL_26(M) M(26)
#define TLK_REPEAT_CALL_28(M) TLK_REPEAT_CALL_27(M) M(27)
#define TLK_REPEAT_CALL_29(M) TLK_REPEAT_CALL_28(M) M(28)
#define TLK_REPEAT_CALL_30(M) TLK_REPEAT_CALL_29(M) M(29)
#define TLK_REPEAT_CALL_31(M) TLK_REPEAT_CALL_30(M) M(30)
#define TLK_REPEAT_CALL_32(M) TLK_REPEAT_CALL_31(M) M(31)
#define TLK_REPEAT_CALL_33(M) TLK_REPEAT_CALL_32(M) M(32)
#define TLK_REPEAT_CALL_34(M) TLK_REPEAT_CALL_33(M) M(33)
#define TLK_REPEAT_CALL_35(M) TLK_REPEAT_CALL_34(M) M(34)
#define TLK_REPEAT_CALL_36(M) TLK_REPEAT_CALL_35(M) M(35)
#define TLK_REPEAT_CALL_37(M) TLK_REPEAT_CALL_36(M) M(36)
#define TLK_REPEAT_CALL_38(M) TLK_REPEAT_CALL_37(M) M(37)
#define TLK_REPEAT_CALL_39(M) TLK_REPEAT_CALL_38(M) M(38)
#define TLK_REPEAT_CALL_40(M) TLK_REPEAT_CALL_39(M) M(39)
#define TLK_REPEAT_CALL_41(M) TLK_REPEAT_CALL_40(M) M(40)
#define TLK_REPEAT_CALL_42(M) TLK_REPEAT_CALL_41(M) M(41)
#define TLK_REPEAT_CALL_43(M) TLK_REPEAT_CALL_42(M) M(42)
#define TLK_REPEAT_CALL_44(M) TLK_REPEAT_CALL_43(M) M(43)
#define TLK_REPEAT_CALL_45(M) TLK_REPEAT_CALL_44(M) M(44)
#define TLK_REPEAT_CALL_46(M) TLK_REPEAT_CALL_45(M) M(45)
#define TLK_REPEAT_CALL_47(M) TLK_REPEAT_CALL_46(M) M(46)
#define TLK_REPEAT_CALL_48(M) TLK_REPEAT_CALL_47(M) M(47)
#define TLK_REPEAT_CALL_49(M) TLK_REPEAT_CALL_48(M) M(48)
#define TLK_REPEAT_CALL_50(M) TLK_REPEAT_CALL_49(M) M(49)
#define TLK_REPEAT_CALL_51(M) TLK_REPEAT_CALL_50(M) M(50)
#define TLK_REPEAT_CALL_52(M) TLK_REPEAT_CALL_51(M) M(51)
#define TLK_REPEAT_CALL_53(M) TLK_REPEAT_CALL_52(M) M(52)
#define TLK_REPEAT_CALL_54(M) TLK_REPEAT_CALL_53(M) M(53)
#define TLK_REPEAT_CALL_55(M) TLK_REPEAT_CALL_54(M) M(54)
#define TLK_REPEAT_CALL_56(M) TLK_REPEAT_CALL_55(M) M(55)
#define TLK_REPEAT_CALL_57(M) TLK_REPEAT_CALL_56(M) M(56)
#define TLK_REPEAT_CALL_58(M) TLK_REPEAT_CALL_57(M) M(57)
#define TLK_REPEAT_CALL_59(M) TLK_REPEAT_CALL_58(M) M(58)
#define TLK_REPEAT_CALL_60(M) TLK_REPEAT_CALL_59(M) M(59)

#define _TLK_FOR_CALL(I, M) TLK_REPEAT_CALL_##I(M)

#define TLK_FOR_CALL(I, M) _TLK_FOR_CALL(I, M)

#define TLK_REPEAT_1(M)  M
#define TLK_REPEAT_2(M)  TLK_REPEAT_1(M) M
#define TLK_REPEAT_3(M)  TLK_REPEAT_2(M) M
#define TLK_REPEAT_4(M)  TLK_REPEAT_3(M) M
#define TLK_REPEAT_5(M)  TLK_REPEAT_4(M) M
#define TLK_REPEAT_6(M)  TLK_REPEAT_5(M) M
#define TLK_REPEAT_7(M)  TLK_REPEAT_6(M) M
#define TLK_REPEAT_8(M)  TLK_REPEAT_7(M) M
#define TLK_REPEAT_9(M)  TLK_REPEAT_8(M) M
#define TLK_REPEAT_10(M) TLK_REPEAT_9(M) M
#define TLK_REPEAT_11(M) TLK_REPEAT_10(M) M
#define TLK_REPEAT_12(M) TLK_REPEAT_11(M) M
#define TLK_REPEAT_13(M) TLK_REPEAT_12(M) M
#define TLK_REPEAT_14(M) TLK_REPEAT_13(M) M
#define TLK_REPEAT_15(M) TLK_REPEAT_14(M) M
#define TLK_REPEAT_16(M) TLK_REPEAT_15(M) M
#define TLK_REPEAT_17(M) TLK_REPEAT_16(M) M
#define TLK_REPEAT_18(M) TLK_REPEAT_17(M) M
#define TLK_REPEAT_19(M) TLK_REPEAT_18(M) M
#define TLK_REPEAT_20(M) TLK_REPEAT_19(M) M
#define TLK_REPEAT_21(M) TLK_REPEAT_20(M) M
#define TLK_REPEAT_22(M) TLK_REPEAT_21(M) M
#define TLK_REPEAT_23(M) TLK_REPEAT_22(M) M
#define TLK_REPEAT_24(M) TLK_REPEAT_23(M) M
#define TLK_REPEAT_25(M) TLK_REPEAT_24(M) M
#define TLK_REPEAT_26(M) TLK_REPEAT_25(M) M
#define TLK_REPEAT_27(M) TLK_REPEAT_26(M) M
#define TLK_REPEAT_28(M) TLK_REPEAT_27(M) M
#define TLK_REPEAT_29(M) TLK_REPEAT_28(M) M
#define TLK_REPEAT_30(M) TLK_REPEAT_29(M) M
#define TLK_REPEAT_31(M) TLK_REPEAT_30(M) M
#define TLK_REPEAT_32(M) TLK_REPEAT_31(M) M
#define TLK_REPEAT_33(M) TLK_REPEAT_32(M) M
#define TLK_REPEAT_34(M) TLK_REPEAT_33(M) M
#define TLK_REPEAT_35(M) TLK_REPEAT_34(M) M
#define TLK_REPEAT_36(M) TLK_REPEAT_35(M) M
#define TLK_REPEAT_37(M) TLK_REPEAT_36(M) M
#define TLK_REPEAT_38(M) TLK_REPEAT_37(M) M
#define TLK_REPEAT_39(M) TLK_REPEAT_38(M) M
#define TLK_REPEAT_40(M) TLK_REPEAT_39(M) M
#define TLK_REPEAT_41(M) TLK_REPEAT_40(M) M
#define TLK_REPEAT_42(M) TLK_REPEAT_41(M) M
#define TLK_REPEAT_43(M) TLK_REPEAT_42(M) M
#define TLK_REPEAT_44(M) TLK_REPEAT_43(M) M
#define TLK_REPEAT_45(M) TLK_REPEAT_44(M) M
#define TLK_REPEAT_46(M) TLK_REPEAT_45(M) M
#define TLK_REPEAT_47(M) TLK_REPEAT_46(M) M
#define TLK_REPEAT_48(M) TLK_REPEAT_47(M) M
#define TLK_REPEAT_49(M) TLK_REPEAT_48(M) M
#define TLK_REPEAT_50(M) TLK_REPEAT_49(M) M
#define TLK_REPEAT_51(M) TLK_REPEAT_50(M) M
#define TLK_REPEAT_52(M) TLK_REPEAT_51(M) M
#define TLK_REPEAT_53(M) TLK_REPEAT_52(M) M
#define TLK_REPEAT_54(M) TLK_REPEAT_53(M) M
#define TLK_REPEAT_55(M) TLK_REPEAT_54(M) M
#define TLK_REPEAT_56(M) TLK_REPEAT_55(M) M
#define TLK_REPEAT_57(M) TLK_REPEAT_56(M) M
#define TLK_REPEAT_58(M) TLK_REPEAT_57(M) M
#define TLK_REPEAT_59(M) TLK_REPEAT_58(M) M
#define TLK_REPEAT_60(M) TLK_REPEAT_59(M) M

#define _TLK_FOR(I, M) TLK_REPEAT_##I(M)

#define TLK_FOR(I, M) _TLK_FOR(I, M)

/* CONCAT */

#define _TLK_CONCAT(x, y) x##y
#define TLK_CONCAT(x, y)  _TLK_CONCAT(x, y)

/* To String */
#define _TLK_TO_STR(x) #x
#define TLK_TO_STR(x)  _TLK_TO_STR(x)

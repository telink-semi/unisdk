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



/* IS_ENABLED and IS_DISABLED */

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
 *     if (IS_ENABLED(CONFIG_MYFEATURE))
 *             myfeature_enable();
 *
 * INTERNAL
 * First pass just to expand any existing macros, we need the macro
 * value to be e.g. a literal "1" at expansion time in the next macro,
 * not "(1)", etc...  Standard recursive expansion does not work.
 */
#define IS_ENABLED(config_macro) _IS_ENABLED1(config_macro)

/**
 * IS_DISABLED does the opposite of IS_ENABLED
 */
#define IS_DISABLED(config_macro) !_IS_ENABLED1(config_macro)

/* Now stick on a "_XXXX" prefix, it will now be "_XXXX1" if config_macro
 * is "1", or just "_XXXX" if it's undefined.
 *   ENABLED:   Z_IS_ENABLED2(_XXXX1)
 *   DISABLED   Z_IS_ENABLED2(_XXXX)
 */
#define _IS_ENABLED1(config_macro) _IS_ENABLED2(_XXXX##config_macro)

/* Here's the core trick, we map "_XXXX1" to "_YYYY," (i.e. a string
 * with a trailing comma), so it has the effect of making this a
 * two-argument tuple to the preprocessor only in the case where the
 * value is defined to "1"
 *   ENABLED:    _YYYY,    <--- note comma!
 *   DISABLED:   _XXXX
 */
#define _XXXX1 _YYYY,

/* Then we append an extra argument to fool the gcc preprocessor into
 * accepting it as a varargs macro.
 *                         arg1   arg2  arg3
 *   ENABLED:   Z_IS_ENABLED3(_YYYY,    1,    0)
 *   DISABLED   Z_IS_ENABLED3(_XXXX 1,  0)
 */
#define _IS_ENABLED2(one_or_two_args) _IS_ENABLED3(one_or_two_args true, false)

/* And our second argument is thus now cooked to be 1 in the case
 * where the value is defined to 1, and 0 if not:
 */
#define _IS_ENABLED3(ignore_this, val, ...) val



/* IF_ENABLED and IF_DISABLED */

/**
 * @brief Insert code if @p _flag is defined and equals 1.
 *
 * Like COND_CODE_1(), this expands to @p _code if @p _flag is defined to 1;
 * it expands to nothing otherwise.
 *
 * Example:
 *
 *     IF_ENABLED(CONFIG_FLAG, (uint32_t foo;))
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
#define IF_ENABLED(_flag, _code) \
	COND_CODE_1(_flag, _code, ())

/**
 * @brief Insert code if @p _flag is not defined as 1.
 *
 * This expands to nothing if @p _flag is defined and equal to 1;
 * it expands to @p _code otherwise.
 *
 * Example:
 *
 *     IF_DISABLED(CONFIG_FLAG, (uint32_t foo;))
 *
 * If @p CONFIG_FLAG isn't defined or different than 1, this expands to:
 *
 *     uint32_t foo;
 *
 * and to nothing otherwise.
 *
 * IF_DISABLED does the opposite of IF_ENABLED.
 *
 * @param _flag evaluated flag
 * @param _code result if @p _flag does not expand to 1; must be in parentheses
 */
#define IF_DISABLED(_flag, _code) \
	COND_CODE_1(_flag, (), _code)

/**
 * @brief Insert code depending on whether @p _flag expands to 1 or not.
 *
 * This relies on similar tricks as IS_ENABLED(), but as the result of
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
 *     COND_CODE_1(CONFIG_FLAG, (uint32_t x;), (there_is_no_flag();))
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
 * However, the advantage of COND_CODE_1() is that code is resolved in
 * place where it is used, while the @p \#if method defines @p
 * MAYBE_DECLARE on two lines and requires it to be invoked again on a
 * separate line. This makes COND_CODE_1() more concise and also
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
#define COND_CODE_1(_flag, _if_1_code, _else_code) \
	Z_COND_CODE_1(_flag, _if_1_code, _else_code)

/**
 * @brief Like COND_CODE_1() except tests if @p _flag is 0.
 *
 * This is like COND_CODE_1(), except that it tests whether @p _flag
 * expands to the integer literal 0. It expands to @p _if_0_code if
 * so, and @p _else_code otherwise; both of these must be enclosed in
 * parentheses.
 *
 * @param _flag evaluated flag
 * @param _if_0_code result if @p _flag expands to 0; must be in parentheses
 * @param _else_code result otherwise; must be in parentheses
 * @see COND_CODE_1()
 */
#define COND_CODE_0(_flag, _if_0_code, _else_code) \
	Z_COND_CODE_0(_flag, _if_0_code, _else_code)

/* Used internally by COND_CODE_1 and COND_CODE_0. */
#define Z_COND_CODE_1(_flag, _if_1_code, _else_code) \
	__COND_CODE(_XXXX##_flag, _if_1_code, _else_code)
#define Z_COND_CODE_0(_flag, _if_0_code, _else_code) \
	__COND_CODE(_ZZZZ##_flag, _if_0_code, _else_code)
#define _ZZZZ0 _YYYY,
#define __COND_CODE(one_or_two_args, _if_code, _else_code) \
	__GET_ARG2_DEBRACKET(one_or_two_args _if_code, _else_code)

/* Gets second argument and removes brackets around that argument. It
 * is expected that the parameter is provided in brackets/parentheses.
 */
#define __GET_ARG2_DEBRACKET(ignore_this, val, ...) __DEBRACKET val

/* Used to remove brackets from around a single argument. */
#define __DEBRACKET(...) __VA_ARGS__




/* IF_ENABLED_ELSE and IF_DISABLED_ELSE */

/**
 * IF_ENABLED_ELSE does the same as IF_ENABLED, but with the else option
 */
 #define IF_ENABLED_ELSE(_flag, _if_code, _else_code) \
	COND_CODE_1(_flag, _if_code, _else_code)

/**
 * IF_DISABLED_ELSE does the same as IF_DISABLED, but with the else option
 */
 #define IF_DISABLED_ELSE(_flag, _if_code, _else_code) \
	COND_CODE_1(_flag, _else_code, _if_code)



/* FOR LOOPS */
#define REPEAT_CALL_1(M) M(0)
#define REPEAT_CALL_2(M) REPEAT_CALL_1(M) M(1)
#define REPEAT_CALL_3(M) REPEAT_CALL_2(M) M(2)
#define REPEAT_CALL_4(M) REPEAT_CALL_3(M) M(3)
#define REPEAT_CALL_5(M) REPEAT_CALL_4(M) M(4)
#define REPEAT_CALL_6(M) REPEAT_CALL_5(M) M(5)
#define REPEAT_CALL_7(M) REPEAT_CALL_6(M) M(6)
#define REPEAT_CALL_8(M) REPEAT_CALL_7(M) M(7)
#define REPEAT_CALL_9(M) REPEAT_CALL_8(M) M(8)
#define REPEAT_CALL_10(M) REPEAT_CALL_9(M) M(9)
#define REPEAT_CALL_11(M) REPEAT_CALL_10(M) M(10)
#define REPEAT_CALL_12(M) REPEAT_CALL_11(M) M(11)
#define REPEAT_CALL_13(M) REPEAT_CALL_12(M) M(12)
#define REPEAT_CALL_14(M) REPEAT_CALL_13(M) M(13)
#define REPEAT_CALL_15(M) REPEAT_CALL_14(M) M(14)
#define REPEAT_CALL_16(M) REPEAT_CALL_15(M) M(10)
#define REPEAT_CALL_17(M) REPEAT_CALL_16(M) M(11)
#define REPEAT_CALL_18(M) REPEAT_CALL_17(M) M(12)
#define REPEAT_CALL_19(M) REPEAT_CALL_18(M) M(13)
#define REPEAT_CALL_20(M) REPEAT_CALL_19(M) M(14)

#define _FOR_CALL(I, M) REPEAT_CALL_##I(M)

#define FOR_CALL(I, M) _FOR_CALL(I, M)


#define REPEAT_1(M) M
#define REPEAT_2(M) REPEAT_1(M) M
#define REPEAT_3(M) REPEAT_2(M) M
#define REPEAT_4(M) REPEAT_3(M) M
#define REPEAT_5(M) REPEAT_4(M) M
#define REPEAT_6(M) REPEAT_5(M) M
#define REPEAT_7(M) REPEAT_6(M) M
#define REPEAT_8(M) REPEAT_7(M) M
#define REPEAT_9(M) REPEAT_8(M) M
#define REPEAT_10(M) REPEAT_9(M) M
#define REPEAT_11(M) REPEAT_10(M) M
#define REPEAT_12(M) REPEAT_11(M) M
#define REPEAT_13(M) REPEAT_12(M) M
#define REPEAT_14(M) REPEAT_13(M) M
#define REPEAT_15(M) REPEAT_14(M) M
#define REPEAT_16(M) REPEAT_15(M) M
#define REPEAT_17(M) REPEAT_16(M) M
#define REPEAT_18(M) REPEAT_17(M) M
#define REPEAT_19(M) REPEAT_18(M) M
#define REPEAT_20(M) REPEAT_19(M) M

#define _FOR(I, M) REPEAT_##I(M)

#define FOR(I, M) _FOR(I, M)

/* CONCAT */

#define _CONCAT(x, y) x##y
#define CONCAT(x, y) _CONCAT(x, y)

/* To String */
#define TLK_TO_STR_HELPER(x) #x
#define TLK_TO_STR(x) TLK_TO_STR_HELPER(x)
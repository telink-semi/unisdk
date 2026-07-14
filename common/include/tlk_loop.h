#ifndef TLK_LOOP_H_
#define TLK_LOOP_H_

typedef void (*tlk_loop_func_t)(void);

/**
 * @brief Registers a loop callback function into the loop registry.
 *
 * Declares a static constant function pointer placed in the
 * ".loop_array" linker section. During system initialization
 * or runtime, the scheduler iterates over this section and
 * invokes each registered loop function.
 *
 * This macro does not generate executable code directly.
 * It performs static registration via linker section aggregation.
 *
 * @param fn Function to register as a loop callback.
 *           Must match the ::tlk_loop_func_t signature.
 *
 * @note
 * - Requires GCC/Clang support for `__attribute__((section))`.
 * - The linker script must define the ".loop_array" section
 *   and provide start/end symbols for iteration.
 * - The `used` attribute prevents dead-strip removal.
 * - Must be invoked at file scope (not inside functions).
 *
 * @warning
 * - `fn` must be a valid function symbol (not an expression).
 * - Signature mismatch with ::tlk_loop_func_t results in undefined behavior.
 * - Registration order depends on link order.
 *
 * @par Expansion
 * Expands to:
 * @code
 * static const tlk_loop_func_t tlk_loop_func_<fn>
 *     __attribute__((section(".loop_array"), used)) = fn;
 * @endcode
 *
 * @par Example
 * @code
 * void user_loop(void);
 *
 * TLK_REGISTER_LOOP(user_loop);
 * @endcode
 */
#define TLK_REGISTER_LOOP(fn)                                                                      \
    static const tlk_loop_func_t tlk_loop_func_##fn                                                \
        __attribute__((section(".loop_array"), used)) = fn;

#endif

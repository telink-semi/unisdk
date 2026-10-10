#ifndef TLK_INCLUDE_DRIVERS_RANDOM_H_
#define TLK_INCLUDE_DRIVERS_RANDOM_H_

#if TLK_IS_DISABLED(CONFIG_TLK_RANDOM)
#warning Random driver is disabled in Kconfig, but its header is being used; note that its implementation will not be linked until you enable it in Kconfig.
#endif

/**
 * @brief Generates a 32-bit pseudo-random or hardware-random number.
 *
 * Behavior depends on the mode selected via Kconfig:
 *
 * | Mode            | Description |
 * |-----------------|--------------------------------------------------------------------------------------------------------------------------------------------|
 * | `PRNG`          | Software generator based on a mathematical algorithm, seeded at startup. | |
 * `TRNG + PRNG`   | Hardware TRNG is used once at startup to seed the software generator, then shut
 * down.                                                      | | `TRNG`          | Each call blocks
 * until the hardware TRNG is ready and returns a freshly generated value. | | `TRNG + DRBG`   |
 * Same as `TRNG`, but the hardware block operates in TRNG + DRBG mode, providing additional entropy
 * post-processing.                         |
 * |-----------------|--------------------------------------------------------------------------------------------------------------------------------------------|
 *
 * @return Random 32-bit value.
 */
uint32_t tlk_random(void);

#endif

#ifndef TLK_INCLUDE_DRIVERS_CLOCK_H_
#define TLK_INCLUDE_DRIVERS_CLOCK_H_

#define TLK_WAIT_FOR_TRUE_OR_TIMEOUT(condition, timeout_us, timeout_handler)                       \
    do                                                                                             \
    {                                                                                              \
        uint64_t start = tlk_clock_cclk_get_tick();                                                \
        while (!(TLK_DEBRACKET condition))                                                         \
        {                                                                                          \
            if ((tlk_clock_cclk_get_tick() - start) >                                              \
                (tlk_sys_clk.cclk * TLK_DEBRACKET timeout_us))                                     \
            {                                                                                      \
                TLK_DEBRACKET timeout_handler                                                      \
            }                                                                                      \
        }                                                                                          \
    } while (0);

enum tlk_clock_status
{
    TLK_CLOCK_OK,
    TLK_CLOCK_ERROR,
};

enum tlk_clock_32k_source
{
    TLK_CLOCK_32K_RC,
    TLK_CLOCK_32K_XTAL,
};

enum tlk_clock_sys_clk_source
{
    TLK_CLOCK_NONE = -1,
    TLK_CLOCK_24M_RC,
    TLK_CLOCK_24M_XTAL,
    TLK_CLOCK_PLL,
};

struct tlk_clock_sys_clk_config
{
    enum tlk_clock_sys_clk_source source;
    uint16_t                      source_clk;
    uint8_t                       cclk;
    uint8_t                       hclk;
    uint8_t                       pclk;
    uint8_t                       mspi_clk;
};

struct tlk_clock_24m_xtal_cap
{
    bool    use_external_cap;
    uint8_t internal_cap_value;
};

extern struct tlk_clock_sys_clk_config tlk_default_sys_clk;
extern struct tlk_clock_sys_clk_config tlk_sys_clk;

/**
 * @brief Set the 32K clock source.
 *
 * @param source - 32K clock source.
 *
 * @return None.
 */
void tlk_clock_32k_configure(enum tlk_clock_32k_source source);

/**
 * @brief Get the current 32K clock source.
 *
 * @return Current 32K clock source.
 */
enum tlk_clock_32k_source tlk_clock_32k_get_source(void);

/**
 * @brief Configure the frequency of CCLK/HCLK/PCLK and MSPI_CLK.
 *
 * You need to wait until all the peripherals that use these clocks are idle before you can switch
 * frequencies.
 *
 * @param sys_clk_config - desired frequencies.
 *
 * @return TLK_CLOCK_OK if set successfully, otherwise TLK_CLOCK_ERROR.
 */
enum tlk_clock_status
tlk_clock_sys_clk_configure(const struct tlk_clock_sys_clk_config* sys_clk_config);

/**
 * @brief Get current CCLK tick.
 *
 * @return Current CCLK tick.
 */
uint64_t tlk_clock_cclk_get_tick(void);

/**
 * @brief Set the blocking delay according to CCLK.
 *
 * @param delay_tick - desired delay in ticks.
 */
void tlk_clock_cclk_delay(uint64_t delay_tick);

/**
 * @brief Configure the capacitor of the 24M XTAL.
 *
 * @param cfg - configuration structure.
 */
void tlk_clock_24m_xtal_configure_cap(struct tlk_clock_24m_xtal_cap cfg);

/**
 * @brief Enable the 24M XTAL.
 *
 * @return None.
 */
void tlk_clock_24m_xtal_enable(void);

/**
 * @brief Determine the stability of the 24M XTAL.
 *
 * @return TLK_CLOCK_OK if 24M XTAL is ready, otherwise TLK_CLOCK_ERROR.
 */
enum tlk_clock_status tlk_clock_24m_xtal_is_ready(void);

/**
 * @brief Determine whether the PLL clock is locked.
 *
 * @return TLK_CLOCK_OK if PLL is ready, otherwise TLK_CLOCK_ERROR.
 */
enum tlk_clock_status tlk_clock_pll_is_ready(void);

/**
 * @brief Ensure the 24M RC clock is enabled.
 *
 * Increments an internal use counter and enables the clock if it was previously off.
 *
 * @return true if the clock was already enabled, false if it was enabled by this call.
 */
bool tlk_clock_24m_rc_acquire(void);

/**
 * @brief Mark that the 24M RC clock is not used anymore by the caller.
 *
 * Decrements an internal use counter and disables the clock if it is not used anymore.
 *
 * @return true if the clock is still used, false if it was disabled by this call.
 */
bool tlk_clock_24m_rc_release(void);

/**
 * @brief Calibrate the 32K RC clock.
 *
 * @return TLK_CLOCK_ERROR if calibration timeout reached, otherwise TLK_CLOCK_OK.
 */
enum tlk_clock_status tlk_clock_32k_rc_calibrate(void);

/**
 * @brief Calibrate the 24M RC clock.
 *
 * @return TLK_CLOCK_ERROR if calibration timeout reached, otherwise TLK_CLOCK_OK.
 */
enum tlk_clock_status tlk_clock_24m_rc_calibrate(void);

#endif

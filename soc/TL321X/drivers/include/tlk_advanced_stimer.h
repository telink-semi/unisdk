#ifndef TLK_ADVANCED_STIMER_H_
#define TLK_ADVANCED_STIMER_H_

#define TLK_STIMER_CLOCK_16M 16000000
#define TLK_STIMER_CLOCK_24M 24000000

typedef enum
{
    FLD_SYSTEM_CAPT_MODE     = TLK_BIT_RNG(0, 1),
    FLD_SYSTEM_CAPT_EN       = TLK_BIT(2),
    FLD_SYSTEM_PEM_EVENT_EN  = TLK_BIT(3),
    FLD_SYSTEM_PEM_TASK_0_EN = TLK_BIT(4),
    FLD_SYSTEM_PEM_TASK_1_EN = TLK_BIT(5),
    FLD_SYSTEM_PEM_TASK_2_EN = TLK_BIT(6),
} tlk_advanced_system_timer_capture_mode_e;

typedef enum
{
    FLD_SYSTEM_32K_MODE_NORMAL = 0,
    FLD_SYSTEM_32K_UPDATE_UPON = TLK_BIT(0),
    FLD_SYSTEM_32K_RUN_UPON_NXT = TLK_BIT(1),
} tlk_advanced_system_timer_32k_cnt_mode_e;

/**
 * @brief   This enumeration defines how many 32k ticks are reached, update the tick value of the increased system timer into the register.
 *          //32k_cnt = 2^(16-(cal_32k_mode>>4)), system_timer_cnt = 750*32k_cnt.(system timer is 24M).
 */
typedef enum
{
    TLK_STIMER_TRACK_32KCNT_2     = 0xf0, /* 2 32k ticks, corresponds 0x5dc system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_4     = 0xe0, /* 4 32k ticks, corresponds 0xbb8 system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_8     = 0xd0, /* 8 32k ticks, corresponds 0x1770 system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_16    = 0xc0, /* 16 32k ticks, corresponds 0x2ee0 system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_32    = 0xb0, /* 32 32k ticks, corresponds 0x5dc0 system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_64    = 0xa0, /* 64 32k ticks, corresponds 0xbb80 system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_128   = 0x90, /* 128 32k ticks, corresponds 0x17700 system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_256   = 0x80, /* 256 32k ticks, corresponds 0x2ee00 system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_512   = 0x70, /* 512 32k ticks, corresponds 0x5dc00 system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_1024  = 0x60, /* 1024 32k ticks, corresponds 0xbb800 system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_2048  = 0x50, /* 2048 32k ticks, corresponds 0x177000 system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_4096  = 0x40, /* 4096 32k ticks, corresponds 0x2ee000 system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_8192  = 0x30, /* 8192 32k ticks, corresponds 0x5dc000 system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_16384 = 0x20, /* 16384 32k ticks, corresponds 0xbb8000 system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_32768 = 0x10, /* 32768 32k ticks, corresponds 0x1770000 system timer ticks. */
    TLK_STIMER_TRACK_32KCNT_65536 = 0x00, /* 65536 32k ticks, corresponds 0x2ee0000 system timer ticks. */
} tlk_stimer_track_cnt_e;

/**
 * @brief   This interface is used to obtain 32k track count.
 * @param[in] mode - the enumeration of extended 32k stimer modes.
 * @return  32k track count.
 */
void tlk_stimer_set_32k_count_mode(tlk_advanced_system_timer_32k_cnt_mode_e mode);

/**
 * @brief   This interface is used to set 32k track count.
 * @param[in]   tick - the value of 32k tick.
 * @return  32k track count.
 */
void tlk_stimer_set_32k_tick(uint32_t tick);

/**
 * @brief   This interface is used to obtain 32k tick count.
 * @return  32k tick count.
 */
uint32_t tlk_stimer_get_32k_tick(void);

/**
 * @brief   This interface is used to configure the 32k track mode.
 *          (2^(16-(cnt>>4)))cycles of 32k clock indicates the tick number of the system timer.
 * @return  none.
 */
void tlk_stimer_set_32k_track_cnt(tlk_stimer_track_cnt_e cnt);

/**
 * @brief   This interface is used to obtain 32k track count.
 * @return  32k track count.
 */
uint32_t tlk_stimer_get_32k_track_cnt(void);

/**
 * @brief   This interface is used to configure the capture mode of stimer.
 * @param[in] tlk_advanced_system_timer_capture_mode_e - the desired capture mode.
 * @return  32k track count.
 */
void tlk_stimer_set_capture_mode(tlk_advanced_system_timer_capture_mode_e mode);

/**
 * @brief   This interface is used to unconfigure the capture mode of stimer.
 * @param[in] tlk_advanced_system_timer_capture_mode_e - the desired capture mode to switch off
 * @return  32k track count.
 */
void tlk_stimer_reset_capture_mode(tlk_advanced_system_timer_capture_mode_e mode);

/**
 * @brief   This interface is used to obtain stimer input capture value.
 * @return  input capture value.
 */
uint32_t tlk_stimer_get_input_capt_value(void);


/**
 * @brief   This interface is used to obtain stimer 32k tracking cycles value.
 * @return  input capture value.
 */
uint32_t tlk_stimer_get_32k_tracking_cycles(void);

#endif

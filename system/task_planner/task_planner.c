#include "task_planner.h"
#include "core/include/tlk_plic_sw.h"
#include "core/include/tlk_stimer.h"
#include "tlk_core.h"
#include "common/include/tlk_init.h"
#include <string.h>

#if IS_ENABLED(CONFIG_TLK_PM)
#include "core/include/tlk_pm.h"

#define systicks_to_mticks(sticks)                                                                 \
	(((uint64_t)(sticks)*UNISDK_MTIMER_CYCLES_PER_SECOND) / TLK_SYSTEM_TIMER_TICK_1S)

#endif

void idle_task(void);

extern tTask __tasks_start;
extern tTask __tasks_end;

#if IS_ENABLED(CONFIG_TLK_SOFTWARE_TIMERS)
extern tSwTimer __timers_start;
extern tSwTimer __timers_end;
#endif

#if IS_ENABLED(CONFIG_TLK_TASK_PLANNER_WORK_QUEUE)
typedef struct
{
    tlk_work_fn_t fn;
    void *arg;
} tWorkItem;

typedef struct 
{
    volatile uint32_t work_q_head;
    volatile uint32_t work_q_tail;
    volatile uint32_t work_q_count;
    volatile bool locked;
    tWorkItem sWorkItem[CONFIG_TLK_TASK_PLANNER_WORK_QUEUE_SIZE];
} tWork_Queue;

static volatile tWork_Queue sWorkQueue = {.work_q_head = 0, .work_q_tail = 0, .work_q_count = 0, .locked = false};

#define is_tlk_work() tlk_work_pending()
#else
#define is_tlk_work() 0
#endif

static volatile uint64_t task_next_deadline = UINT64_MAX;
static volatile uint64_t timer_next_deadline = UINT64_MAX;

void task_planner_plan_next_event(void)
{
    uint64_t now  = tlk_mtimer_get_mtime();
    uint64_t next = (task_next_deadline < timer_next_deadline) ? task_next_deadline : timer_next_deadline;
    uint64_t cmp = next + 1;
    // Guarantee cmp is strictly in the future to clear MTIP
    if ((int64_t)(now - cmp) >= 0) {
        cmp = now + 1;
    }

    tlk_mtimer_set_mtime_compare(cmp);
    tlk_core_mie_enable(TLK_FLD_MIE_MTIE | TLK_FLD_MIE_MSIE | TLK_FLD_MIE_MEIE);
}

#if IS_ENABLED(CONFIG_TLK_SOFTWARE_TIMERS)
void tlk_sw_timer_start(tSwTimer *tm)
{
    tlk_core_interrupt_disable();
    tm->expire = tlk_mtimer_get_mtime() + tm->period;
    tm->active = 1;
    if (tm->expire < timer_next_deadline)
    timer_next_deadline = tm->expire;
    task_planner_plan_next_event();
    tlk_core_interrupt_enable();
}

void tlk_sw_timer_stop(tSwTimer *tm)
{
    tm->active = 0;
}

void tlk_sw_set_period(tSwTimer *tm, uint64_t period)
{
    tm->period = period;
}

void tlk_sw_set_mode(tSwTimer *tm, teSwTimer_mode mode)
{
    tm->mode = mode;
}

#endif

void task_planner_run(void)
{
    while (1)
    {
        uint64_t now = tlk_mtimer_get_mtime();
        task_next_deadline = UINT64_MAX;

        for (tTask *t = &__tasks_start; t < &__tasks_end; t++)
        {
            if (t->active)
            {
                if ((int64_t)(now - t->wake) >= 0)
                    t->fn(t);

                if (t->wake < task_next_deadline)
                    task_next_deadline = t->wake;
            }
        }

        task_planner_plan_next_event();
        idle_task();
    }
}


void idle_task(void)
{
    
    uint64_t mtimecmp = tlk_mtimer_get_mtime_compare();
    if ((tlk_mtimer_get_mtime() < mtimecmp) && !is_tlk_work())
    {

        #if IS_ENABLED(CONFIG_TLK_PM)
        uint64_t now = tlk_mtimer_get_mtime();
        uint32_t tl_stimer_sleep_tick = tlk_stimer_get_tick();
        uint32_t sleep_duration_ms = ((mtimecmp - now) / (UNISDK_MTIMER_CYCLES_PER_SECOND /1000) );
        enum tlk_pm_sleep_mode mode = IF_ENABLED_ELSE(CONFIG_TLK_PM_RAM_RETENTION_ENABLE, 
            (TLK_PM_SLEEP_MODE_DEEP_RETENTION), 
            (IF_ENABLED_ELSE(CONFIG_TLK_PM_DEEP_SLEEP, 
                (TLK_PM_SLEEP_MODE_DEEP_SLEEP), 
                (TLK_PM_SLEEP_MODE_SUSPEND))));
        enum tlk_pm_sleep_status pm_sleep_state = tlk_pm_sleep(mode, sleep_duration_ms);
        if (pm_sleep_state == TLK_PM_SLEEP_OK || pm_sleep_state == TLK_PM_SLEEP_SUSPEND_FALLBACK)
        {
            now += systicks_to_mticks(tlk_stimer_get_tick() - tl_stimer_sleep_tick);
            tlk_mtimer_set_mtime(now);
            // fire mtimer IRQ immediately, if wakeup later than mtimecmp and IRQ possibly missed.
            // tlk_mtimer_set_mtime_compare(now+1);
            return;
        }
        else
        #endif
        {   
            __asm__ __volatile__("wfi");
        }
    }
}

void tlk_task_planner_mtimer_irq(void)
{
    #if IS_ENABLED(CONFIG_TLK_SOFTWARE_TIMERS)
    uint64_t now = tlk_mtimer_get_mtime();
    timer_next_deadline = UINT64_MAX;
    
    for (tSwTimer *tm = &__timers_start; tm < &__timers_end; tm++)
    {
        if (tm->active)
        {
            if ((int64_t)(now - tm->expire) >= 0)
            {
                tm->cb(tm->arg);

                if (tm->mode == SW_TIMER_MODE_PERIODIC)
                {
                    tm->expire += tm->period;
                }
                else
                {
                    tm->active = 0;
                }
            }

            if (tm->active && tm->expire < timer_next_deadline)
                timer_next_deadline = tm->expire;
        }
        
    }

    #endif
    task_planner_plan_next_event();
}

void task_planner_init(void)
{
    extern uint32_t _TASKS_ARRAY_LMA_START[];
    //load tasks from flash to RAM

    memcpy(&__tasks_start,
    _TASKS_ARRAY_LMA_START,
    #if IS_ENABLED(CONFIG_TLK_SOFTWARE_TIMERS)
        (uint8_t *)&__timers_end -
    #else
        (uint8_t *)&__tasks_end -
    #endif
    (uint8_t *)&__tasks_start);
    tlk_mtimer_register_callback(tlk_task_planner_mtimer_irq);
    tlk_core_interrupt_enable();
}

TLK_REGISTER_PRE_INIT(task_planner_init, TLK_INIT_LEVEL_APPLICATION, TLK_INIT_PRIORITY_NORMAL);

#if IS_ENABLED (CONFIG_TLK_TASK_PLANNER_DEFAULT_TASK)
extern void user_loop(void);
void default_task(tTask *t)
{
    user_loop();
    TASK_DELAY(t, TASK_MS(CONFIG_TLK_TASK_PLANNER_DEFAULT_TASK_LOOP_PERIOD_MS));
}

TASK_SPAWN(default_task_obj, default_task);
#endif

#if IS_ENABLED(CONFIG_TLK_TASK_PLANNER_WORK_QUEUE)

static void tlk_work_queue_task(tTask *t);
TASK_SPAWN(work_queue_task_obj, tlk_work_queue_task);

static inline bool tlk_work_queue_is_empty(void)
{
    return sWorkQueue.work_q_count == 0;
}

static inline bool tlk_work_queue_is_full(void)
{
    return sWorkQueue.work_q_count >= CONFIG_TLK_TASK_PLANNER_WORK_QUEUE_SIZE;
}

bool tlk_work_pending(void)
{
    return !tlk_work_queue_is_empty();
}

static bool tlk_work_queue_pop(tWorkItem *item)
{
    if (tlk_work_queue_is_empty()) {
        return false; }

    *item = sWorkQueue.sWorkItem[sWorkQueue.work_q_head];
    sWorkQueue.work_q_head = (sWorkQueue.work_q_head + 1U) % CONFIG_TLK_TASK_PLANNER_WORK_QUEUE_SIZE;
    sWorkQueue.work_q_count--;
    return true;
}

bool tlk_work_submit(tlk_work_fn_t fn, void *arg)
{
    bool ok = false;

    if (fn == NULL)
        return false;

    tlk_core_interrupt_disable();

    if (!sWorkQueue.locked)
    {
        sWorkQueue.locked = true;
        if (!tlk_work_queue_is_full())
        {
            sWorkQueue.sWorkItem[sWorkQueue.work_q_tail].fn  = fn;
            sWorkQueue.sWorkItem[sWorkQueue.work_q_tail].arg = arg;
            sWorkQueue.work_q_tail = (sWorkQueue.work_q_tail + 1U) % CONFIG_TLK_TASK_PLANNER_WORK_QUEUE_SIZE;
            sWorkQueue.work_q_count++;

            /* Wake the work queue task immediately */
            TASK_YIELD(&work_queue_task_obj);
            tlk_core_mie_enable(TLK_FLD_MIE_MSIE);
            // tlk_plic_sw_interrupt_enable();
            tlk_plic_sw_set_pending();

            ok = true;
        }
        sWorkQueue.locked = false;
    }

    tlk_core_interrupt_enable();
    return ok;
}

static void tlk_work_queue_task(tTask *t)
{
    tWorkItem item;

    while (1)
    {
        tlk_core_interrupt_disable();
        bool has_work = tlk_work_queue_pop(&item);
        tlk_core_interrupt_enable();

        if (!has_work)
            break;

        if (item.fn)
            item.fn(item.arg);
    }

    /*
     * No more jobs: park forever.
     * A new tlk_work_submit() will set wake = now and scheduler will run us.
     */
    t->wake = UINT64_MAX;
}
#endif

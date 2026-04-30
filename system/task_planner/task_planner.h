#ifndef TASK_PLANNER_H_
#define TASK_PLANNER_H_

#include "core/include/tlk_mtimer.h"
#include "properties/tlk_mtimer.h"

typedef struct task
{
    void (*fn)(struct task *t);
    uint64_t wake;
    uint8_t active;
}  __attribute__((aligned(4))) tTask;


/**
 * @brief Registers a task in the linker-defined task array.
 *
 * Expands to a static instance of ::tTask placed in the ".tasks_array"
 * linker section. The task function must not be NULL. A compile-time
 * assertion is generated to enforce this constraint.
 *
 * @param name      Identifier for the generated static tTask object.
 * @param function  Pointer to the task function to be registered.
 *
 * @note
 * - This macro declares a static object.
 * - The object is marked with `used` to prevent linker removal.
 * - Requires GCC-compatible compiler supporting `__attribute__((section))`.
 *
 * @warning
 * - The `function` argument must be a valid function pointer.
 * - `name` must be a valid C identifier.
 * - Not safe for use inside conditional blocks due to declaration semantics.
 *
 * @par Side Effects
 * - Introduces a symbol in the ".tasks_array" linker section.
 * - Contributes to the final firmware image layout.
 *
 * @par Example
 * @code
 * void LogTask(void);
 *
 * TASK_REGISTER(logTask, LogTask);
 * @endcode
 */
#define TASK_REGISTER(name, function) \
    _Static_assert((function) != NULL, "Task function cannot be NULL"); \
    static tTask name \
    __attribute__((section(".tasks_array"), used)) \
     = {.fn = function, .wake = 0,  .active = 0 ,}; 

/**
 * @brief Delays a task by a relative number of timer ticks.
 *
 * Sets the task wake timestamp to the current machine time plus
 * the specified tick offset.
 *
 * @param t     Pointer to ::tTask instance.
 * @param tick  Relative delay in timer ticks.
 *
 * @note
 * - Uses tlk_mtimer_get_mtime() as the time source.
 * - Delay resolution depends on timer frequency.
 *
 * @warning
 * - `t` must be a valid pointer.
 * - Overflow may occur if `tick` exceeds 64-bit range.
 *
 * @par Side Effects
 * Modifies `t->wake`.
 */
#define TASK_DELAY(t, tick) \
    do { (t)->wake = tlk_mtimer_get_mtime() + (uint64_t)(tick); } while (0)

/**
 * @brief Yields the task for immediate rescheduling.
 *
 * Sets the task wake timestamp to the current machine time,
 * making it eligible for execution on the next scheduler cycle.
 *
 * @param t Pointer to ::tTask instance.
 *
 * @warning
 * - `t` must be a valid pointer.
 *
 * @par Side Effects
 * Modifies `t->wake`.
 */
#define TASK_YIELD(t) \
    do { (t)->wake = tlk_mtimer_get_mtime(); } while (0)

/**
 * @brief Deactivates a task.
 *
 * Marks the task as inactive, preventing it from being scheduled.
 *
 * @param t Pointer to ::tTask instance.
 *
 * @note
 * The scheduler must check the `active` field to enforce pause.
 *
 * @par Side Effects
 * Sets `t->active` to 0.
 *
 * @warning
 * - `t` must be a valid pointer.
 * - Not atomic — external synchronization may be required.
 */
#define TASK_PAUSE(t) \
      (t)->active = 0;  

/**
 * @brief Reactivates a paused task.
 *
 * Marks the task as active, allowing it to be scheduled again.
 *
 * @param t Pointer to ::tTask instance.
 *
 * @par Side Effects
 * Sets `t->active` to 1.
 *
 * @warning
 * - `t` must be a valid pointer.
 * - Does not modify wake time.
 */      
#define TASK_RESUME(t) \
      (t)->active = 1;  

/**
 * @brief Statically registers and activates a task.
 *
 * Declares a static ::tTask instance placed in the
 * ".tasks_array.tasks" linker section.
 *
 * The task is initialized as active and ready to run.
 *
 * @param name      Identifier of the generated task object.
 * @param function  Task function pointer.
 *
 * @note
 * - Requires GCC-compatible compiler for `__attribute__((section))`.
 * - The linker script must define ".tasks_array.tasks".
 * - Object is marked `used` to prevent dead-strip removal.
 *
 * @warning
 * - `name` must be a valid C identifier.
 * - Must be used at file scope (not inside functions).
 * - `function` must match the expected task function signature.
 *
 * @par Side Effects
 * Introduces a symbol into a custom linker section.
 */
#define TASK_SPAWN(name, function) \
    static tTask name \
    __attribute__((section(".tasks_array.tasks"), used)) \
     = {.fn = function, .wake = 0,  .active = 1 ,}; 

/**
 * @brief Converts milliseconds to timer cycles.
 *
 * Computes the number of machine timer cycles corresponding
 * to the given millisecond value.
 *
 * @param ms Milliseconds.
 *
 * @return Equivalent number of timer cycles.
 *
 * @note
 * Depends on UNISDK_MTIMER_CYCLES_PER_SECOND definition.
 *
 * @warning
 * - Expression is not parenthesized internally; wrap in parentheses when used in complex expressions.
 * - Potential integer overflow if `ms` is large.
 *
 * @par Example
 * @code
 * TASK_DELAY(task, TASK_MS(100));
 * @endcode
 */     
#define TASK_MS(ms) (UNISDK_MTIMER_CYCLES_PER_SECOND / 1000) * ms


#if IS_ENABLED(CONFIG_TLK_SOFTWARE_TIMERS)

typedef void (*sw_timer_cb_t)(void *arg);

typedef enum {
    SW_TIMER_MODE_ONE_SHOT = 0,
    SW_TIMER_MODE_PERIODIC = 1
} teSwTimer_mode;

typedef struct {
    uint64_t    expire;      // absolute mtime deadline
    uint64_t    period;      // 0 for one-shot
    uint8_t     active;
    teSwTimer_mode mode;
    sw_timer_cb_t cb;
    void       *arg;
} __attribute__((aligned(4))) tSwTimer;

/**
 * @brief Starts a software timer.
 *
 * Activates the specified software timer and schedules it
 * according to its configured period and mode.
 *
 * @param tm Pointer to ::tSwTimer instance.
 *
 * @note
 * - The timer must be properly initialized before calling.
 * - The scheduling base depends on the system time source.
 *
 * @warning
 * - `tm` must not be NULL.
 * - Not inherently thread-safe; external synchronization may be required
 *   if called from multiple contexts (e.g., ISR + task).
 *
 * @par Side Effects
 * Sets `tm->active` and updates internal timing state.
 */
void tlk_sw_timer_start(tSwTimer *tm);

/**
 * @brief Stops a running software timer.
 *
 * Deactivates the timer and prevents further callback execution.
 *
 * @param tm Pointer to ::tSwTimer instance.
 *
 * @note
 * Stopping a timer does not modify its configured period or mode.
 *
 * @warning
 * - `tm` must not be NULL.
 *
 * @par Side Effects
 * Clears `tm->active`.
 */
void tlk_sw_timer_stop(tSwTimer *tm);

/**
 * @brief Sets the period of a software timer.
 *
 * Configures the timer interval in system timer units.
 *
 * @param tm     Pointer to ::tSwTimer instance.
 * @param period Timer period in machine time units.
 *
 * @note
 * - Units must match the scheduler's time base.
 * - Does not automatically restart the timer.
 *
 * @warning
 * - `tm` must not be NULL.
 * - If modified while active, behavior depends on scheduler implementation.
 *
 * @par Side Effects
 * Updates `tm->period`.
 */
void tlk_sw_set_period(tSwTimer *tm, uint64_t period);

/**
 * @brief Sets the operating mode of a software timer.
 *
 * Configures whether the timer operates in one-shot
 * or periodic mode.
 *
 * @param tm   Pointer to ::tSwTimer instance.
 * @param mode Timer mode of type ::teSwTimer_mode.
 *
 * @note
 * Mode interpretation depends on scheduler implementation.
 *
 * @warning
 * - `tm` must not be NULL.
 * - Changing mode while active may require restart.
 *
 * @par Side Effects
 * Updates `tm->mode`.
 */
void tlk_sw_set_mode(tSwTimer *tm, teSwTimer_mode mode);

/**
 * @brief Statically registers a software timer instance.
 *
 * Declares a static ::tSwTimer object placed in the
 * ".tasks_array.timers" linker section.
 *
 * The timer is initialized but inactive. It must be started
 * explicitly using ::tlk_sw_timer_start().
 *
 * @param name       Identifier of the generated timer object.
 * @param callback   Timer callback function.
 * @param tmr_mode   Timer mode (::teSwTimer_mode).
 * @param shot_time  Initial period value in timer units.
 *
 * @note
 * - Requires GCC/Clang support for `__attribute__((section))`.
 * - The linker script must define ".tasks_array.timers".
 * - Object is marked `used` to prevent removal by the linker.
 *
 * @warning
 * - Must be used at file scope (not inside a function).
 * - `name` must be a valid C identifier.
 * - `callback` must match the expected callback signature.
 *
 * @par Side Effects
 * Introduces a symbol into a custom linker section, contributing
 * to static timer registration during system initialization.
 *
 * @par Example
 * @code
 * void MyTimerCallback(void);
 *
 * TLK_SW_TIMER_REGISTER(myTimer,
 *                       MyTimerCallback,
 *                       SW_TIMER_MODE_PERIODIC,
 *                       1000);
 * @endcode
 */
#define TLK_SW_TIMER_REGISTER(name, callback, tmr_mode, shot_time) \
    static tSwTimer name \
    __attribute__((section(".tasks_array.timers"), used)) \
     = {0, shot_time, 0, tmr_mode, callback, NULL}; 

#endif

#if IS_ENABLED(CONFIG_TLK_TASK_PLANNER_WORK_QUEUE)

typedef void (*tlk_work_fn_t)(void *arg);
/**
 * @brief Checks whether there are pending work items.
 *
 * Determines if any work items have been queued but not yet
 * processed by the work scheduler.
 *
 * @retval true   At least one work item is pending.
 * @retval false  No pending work items in the queue.
 *
 * @note
 * - This function is typically used for polling or diagnostics.
 * - Does not modify the state of the work queue.
 *
 * @warning
 * - Result may change immediately after return in concurrent
 *   environments (e.g., interrupts or multi-threading).
 *
 * @par Side Effects
 * None.
 *
 * @par Example
 * @code
 * if (tlk_work_pending()) {
 *     // Handle or wait for pending work
 * }
 * @endcode
 */
bool tlk_work_pending(void);

/**
 * @brief Submits a work item for deferred execution.
 *
 * Enqueues a function to be executed later by the work scheduler,
 * optionally passing a user-defined argument.
 *
 * @param fn   Work function to execute (::tlk_work_fn_t).
 * @param arg  User-provided argument passed to the function.
 *
 * @retval true   Work item successfully queued.
 * @retval false  Failed to queue work item (e.g., queue full or invalid input).
 *
 * @note
 * - Execution context depends on the underlying scheduler
 *   (e.g., main loop, worker thread, or interrupt context).
 * - The function pointer must remain valid until execution.
 *
 * @warning
 * - Do not pass pointers to stack variables with limited lifetime.
 * - Ensure the work function is reentrant or otherwise safe
 *   for the execution context.
 *
 * @par Side Effects
 * - Adds an item to the internal work queue.
 * - May trigger scheduler activity depending on implementation.
 *
 * @par Example
 * @code
 * void MyWorkHandler(void *arg) {
 *     // Process work
 * }
 *
 * tlk_work_submit(MyWorkHandler, NULL);
 * @endcode
 */
bool tlk_work_submit(tlk_work_fn_t fn, void *arg);
#endif

#endif
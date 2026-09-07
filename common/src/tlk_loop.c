#include "common/include/tlk_loop.h"

extern tlk_loop_func_t __loop_start;
extern tlk_loop_func_t __loop_end;

__attribute__((weak)) void user_loop(void) {}

#if TLK_IS_ENABLED(CONFIG_TLK_TASK_PLANNER_MODE_LOOP)
TLK_REGISTER_LOOP(user_loop);

static void tlk_loop(void)
{
    for (tlk_loop_func_t* func = &__loop_start; func < &__loop_end; ++func)
    {
        if (*func)
        {
            (*func)();
        }
    }
}

__attribute__((used, noinline)) void tlk_platform_loop(void)
{
    while (1)
    {
        tlk_loop();
    }
}
#endif

#include "common/include/tlk_init.h"

extern tlk_pre_init_func_t __pre_init_start[];
extern tlk_pre_init_func_t __pre_init_end[];

void tlk_pre_init(void) {
    size_t cnt = __pre_init_end - __pre_init_start;

    for (size_t i = 0; i < cnt; ++i) {
        __pre_init_start[i]();
    }
}



extern tlk_before_suspend_func_t __before_suspend_start[];
extern tlk_before_suspend_func_t __before_suspend_end[];

void tlk_before_suspend(void) {
    size_t cnt = __before_suspend_end - __before_suspend_start;

    for (size_t i = 0; i < cnt; ++i) {
        __before_suspend_start[i]();
    }
}


extern tlk_after_suspend_func_t __after_suspend_start[];
extern tlk_after_suspend_func_t __after_suspend_end[];

void tlk_after_suspend(void) {
    size_t cnt = __after_suspend_end - __after_suspend_start;

    for (size_t i = 0; i < cnt; ++i) {
        __after_suspend_start[i]();
    }
}



extern tlk_before_sleep_func_t __before_sleep_start[];
extern tlk_before_sleep_func_t __before_sleep_end[];

void tlk_before_sleep(void) {
    size_t cnt = __before_sleep_end - __before_sleep_start;

    for (size_t i = 0; i < cnt; ++i) {
        __before_sleep_start[i]();
    }
}


extern tlk_after_sleep_func_t __after_sleep_start[];
extern tlk_after_sleep_func_t __after_sleep_end[];

void tlk_after_sleep(void) {
    size_t cnt = __after_sleep_end - __after_sleep_start;

    for (size_t i = 0; i < cnt; ++i) {
        __after_sleep_start[i]();
    }
}



extern tlk_prevent_suspend_func_t __prevent_suspend_start[];
extern tlk_prevent_suspend_func_t __prevent_suspend_end[];

bool tlk_prevent_suspend(void) {
    size_t cnt = __prevent_suspend_end - __prevent_suspend_start;

    for (size_t i = 0; i < cnt; ++i) {
        if (__prevent_suspend_start[i]()) {
            return true;
        }
    }

    return false;
}



extern tlk_prevent_sleep_func_t __prevent_sleep_start[];
extern tlk_prevent_sleep_func_t __prevent_sleep_end[];

bool tlk_prevent_sleep(void) {
    size_t cnt = __prevent_sleep_end - __prevent_sleep_start;

    for (size_t i = 0; i < cnt; ++i) {
        if (__prevent_sleep_start[i]()) {
            return true;
        }
    }

    return false;
}

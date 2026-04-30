#ifndef TLK_INIT_H_
#define TLK_INIT_H_

#define TLK_INIT_LEVEL_SYSTEM 2
#define TLK_INIT_LEVEL_DRIVER 5
#define TLK_INIT_LEVEL_APPLICATION 8

#define TLK_INIT_PRIORITY_HIGH 2
#define TLK_INIT_PRIORITY_NORMAL 5
#define TLK_INIT_PRIORITY_LOW 8

typedef void (*tlk_pre_init_func_t)(void);

#define _TLK_REGISTER_PRE_INIT(fn, level, priority) \
    static const tlk_pre_init_func_t __tlk_pre_init_##fn \
    __attribute__((section(".pre_init_array_" #level "_" #priority), used)) = fn;

#define TLK_REGISTER_PRE_INIT(fn, level, priority) _TLK_REGISTER_PRE_INIT(fn, level, priority)

void tlk_pre_init(void);



typedef void (*tlk_before_suspend_func_t)(void);

#define _TLK_REGISTER_BEFORE_SUSPEND(fn, level, priority) \
    static const tlk_before_suspend_func_t __tlk_before_suspend_##fn \
    __attribute__((section(".before_suspend_array_" #level "_" #priority), used)) = fn;

#define TLK_REGISTER_BEFORE_SUSPEND(fn, level, priority) _TLK_REGISTER_BEFORE_SUSPEND(fn, level, priority)

void tlk_before_suspend(void);


typedef void (*tlk_after_suspend_func_t)(void);

#define _TLK_REGISTER_AFTER_SUSPEND(fn, level, priority) \
    static const tlk_after_suspend_func_t __tlk_after_suspend_##fn \
    __attribute__((section(".after_suspend_array_" #level "_" #priority), used)) = fn;

#define TLK_REGISTER_AFTER_SUSPEND(fn, level, priority) _TLK_REGISTER_AFTER_SUSPEND(fn, level, priority)

void tlk_after_suspend(void);



typedef void (*tlk_before_sleep_func_t)(void);

#define _TLK_REGISTER_BEFORE_SLEEP(fn, level, priority) \
    static const tlk_before_sleep_func_t __tlk_before_sleep_##fn \
    __attribute__((section(".before_sleep_array_" #level "_" #priority), used)) = fn;

#define TLK_REGISTER_BEFORE_SLEEP(fn, level, priority) _TLK_REGISTER_BEFORE_SLEEP(fn, level, priority)

void tlk_before_sleep(void);


typedef void (*tlk_after_sleep_func_t)(void);

#define _TLK_REGISTER_AFTER_SLEEP(fn, level, priority) \
    static const tlk_after_sleep_func_t __tlk_after_sleep_##fn \
    __attribute__((section(".after_sleep_array_" #level "_" #priority), used)) = fn;

#define TLK_REGISTER_AFTER_SLEEP(fn, level, priority) _TLK_REGISTER_AFTER_SLEEP(fn, level, priority)

void tlk_after_sleep(void);



typedef bool (*tlk_prevent_suspend_func_t)(void);

#define TLK_REGISTER_PREVENT_SUSPEND(fn) \
    static const tlk_prevent_suspend_func_t _tlk__prevent_suspend_##fn \
    __attribute__((section(".prevent_suspend_array"), used)) = fn;

bool tlk_prevent_suspend(void);



typedef bool (*tlk_prevent_sleep_func_t)(void);

#define TLK_REGISTER_PREVENT_SLEEP(fn) \
    static const tlk_prevent_sleep_func_t _tlk__prevent_sleep_##fn \
    __attribute__((section(".prevent_sleep_array"), used)) = fn;

bool tlk_prevent_sleep(void);


#endif
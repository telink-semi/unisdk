#include "tlk_log.h"
#include "common/include/tlk_init.h"
#include "common/include/tlk_utils.h"
#include "core/include/tlk_core.h"
#include "core/include/tlk_mtimer.h"
#include "core/include/tlk_sys.h"
#include "properties/tlk_mtimer.h"
#include "system/task_planner/task_planner.h"
#include <string.h>

#if ((CONFIG_TLK_MEMORY_STACK_SIZE * 1024UL) <                                                     \
     (CONFIG_TLK_DEBUG_LOG_BUFFER_SIZE * CONFIG_TLK_DEBUG_LOG_QUEUE_SIZE))
#error "Stack size too small for configured logging buffers"
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_FROM_ISR)
#include "core/include/tlk_plic.h"
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_UART)
#include "core/include/tlk_uart.h"
static volatile uint8_t tlk_tx_done = 1;
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_COLOR)
#define COL_RESET  "\x1b[0m"
#define COL_RED    "\x1b[31m"
#define COL_GREEN  "\x1b[32m"
#define COL_YELLOW "\x1b[33m"
#define COL_BLUE   "\x1b[34m"
#define COL_MAG    "\x1b[35m"
#define COL_CYAN   "\x1b[36m"
#define COL_GRAY   "\x1b[90m"
#else
#define COL_RESET  ""
#define COL_RED    ""
#define COL_GREEN  ""
#define COL_YELLOW ""
#define COL_BLUE   ""
#define COL_MAG    ""
#define COL_CYAN   ""
#define COL_GRAY   ""
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_LOW) || TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_HIGH)
#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_HIGH)
#include "system/task_planner/task_planner.h"
#endif

typedef struct
{
    uint64_t        ts_us;
    const char*     module;
    tlk_log_level_t level;

    /* If is_literal=true, msg points to a stable string (ISR-safe).
       Else msg_buf contains a preformatted message produced in task context. */
    bool        is_literal;
    const char* msg;
    uint16_t    msg_len;
    char        msg_buf[CONFIG_TLK_DEBUG_LOG_BUFFER_SIZE];
} tlk_log_event_t;

static volatile uint32_t s_wr = 0;
static volatile uint32_t s_rd = 0;
static tlk_log_event_t   s_q[CONFIG_TLK_DEBUG_LOG_QUEUE_SIZE];
static bool              tlk_debug_log_event_pop(tlk_log_event_t* out);

#elif TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_IMMEDIATE)

static char tlk_log_buffer[CONFIG_TLK_DEBUG_LOG_BUFFER_SIZE];

#endif

static inline const char* tlk_debug_log_level_str_get(tlk_log_level_t lvl)
{
    switch (lvl)
    {
    case TLK_LOG_LEVEL_ERROR:
        return COL_RED "ERR" COL_RESET;
    case TLK_LOG_LEVEL_WARN:
        return COL_YELLOW "WARN" COL_RESET;
    case TLK_LOG_LEVEL_INFO:
        return COL_GREEN "INFO" COL_RESET;
    case TLK_LOG_LEVEL_DEBUG:
        return COL_CYAN "DBG" COL_RESET;
    default:
        return "LOG";
    }
}

static uint64_t tlk_debug_log_get_us(void)
{
    return (tlk_mtimer_get_mtime() * 1000000ULL) / UNISDK_MTIMER_CYCLES_PER_SECOND;
}

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_UART)
#define tlk_log_uart_lock()                                                                        \
    do                                                                                             \
    {                                                                                              \
        tlk_tx_done = 0;                                                                           \
    } while (0)

#define tlk_log_uart_unlock()                                                                      \
    do                                                                                             \
    {                                                                                              \
        tlk_tx_done = 1;                                                                           \
    } while (0)

#define tlk_log_interface_is_locked() (!tlk_tx_done)

static void tlk_debug_log_tx_handle(void)
{
    tlk_log_uart_unlock();
#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_HIGH) // apply for timer initiator
    static tlk_log_event_t e;
    if (tlk_debug_log_event_pop(&e))
    {
        tlk_uart_send_bytes(CONFIG_TLK_DEBUG_UART,
                            (const uint8_t*) e.msg_buf,
                            e.msg_len,
                            tlk_debug_log_tx_handle,
                            TLK_SEC_TO_US(5));
    }
#endif
}
#else
#define tlk_log_interface_is_locked() 0

#endif

void tlk_log_hexdump(const tlk_log_t* log, tlk_log_level_t level, const void* data, uint32_t len)
{
    if (!data || len == 0)
        return;

    const uint8_t* bytes = (const uint8_t*) data;
    char           line[100];

    for (uint32_t i = 0; i < len; i += 16)
    {
        int off = snprintf(line, sizeof(line), "%04X: ", (unsigned int) i);

        for (uint8_t j = 0; j < 16; j++)
        {
            if (i + j < len)
            {
                off += snprintf(line + off, sizeof(line) - off, "%02X ", bytes[i + j]);
            }
            else
            {
                off += snprintf(line + off, sizeof(line) - off, "   ");
            }
        }

        off += snprintf(line + off, sizeof(line) - off, " |");

        for (uint8_t j = 0; j < 16 && (i + j) < len; j++)
        {
            uint8_t c   = bytes[i + j];
            line[off++] = (c >= 32 && c <= 126) ? (char) c : '.';
        }

        line[off++] = '|';
        line[off]   = '\0';

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_IMMEDIATE)
        tlk_debug_log_blocking(log, level, "%s", line);
#else
        tlk_log_queue(log, level, "%s", line);
#endif
    }
}

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_IMMEDIATE)

void tlk_debug_log_blocking(const tlk_log_t* log, tlk_log_level_t level, const char* fmt, ...)
{
    uint64_t timestamp = tlk_debug_log_get_us();

    /* 1) Prefix */
    int off = snprintf(tlk_log_buffer,
                       sizeof(tlk_log_buffer),
                       "[%05lu.%03u.%03u]: [%s]: [%s]: ",
                       (unsigned long) ((timestamp / 1000000) % 100000),
                       (uint16_t) ((timestamp / 1000) % 1000),
                       (uint16_t) (timestamp % 1000),
                       log ? log->module : "?",
                       tlk_debug_log_level_str_get(level));

    if (off < 0 || off >= (int) sizeof(tlk_log_buffer))
    {
        return;
    }

    /* 2) Append formatted message */
    va_list ap;
    va_start(ap, fmt);
    int32_t size = vsnprintf(tlk_log_buffer + off, sizeof(tlk_log_buffer) - off, fmt, ap);
    size += off;
    va_end(ap);

    if (size < CONFIG_TLK_DEBUG_LOG_BUFFER_SIZE - 2)
    {
        tlk_log_buffer[size++] = '\r';
        tlk_log_buffer[size++] = '\n';
        tlk_log_buffer[size]   = '\0';
    }
    else
    {
        tlk_log_buffer[CONFIG_TLK_DEBUG_LOG_BUFFER_SIZE - 3] = '\r';
        tlk_log_buffer[CONFIG_TLK_DEBUG_LOG_BUFFER_SIZE - 2] = '\n';
        tlk_log_buffer[CONFIG_TLK_DEBUG_LOG_BUFFER_SIZE - 1] = '\0';
    }

/* 3) Blocking output */
#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_UART)
    if (size > 0)
    {
        tlk_uart_send_bytes(CONFIG_TLK_DEBUG_UART,
                            (uint8_t*) tlk_log_buffer,
                            size,
                            tlk_debug_log_tx_handle,
                            TLK_SEC_TO_US(5));
        while (!tlk_tx_done)
        {
        }
        tlk_tx_done = 0;
    }

#elif TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_USB_CDC)
    extern uint16_t tlk_debug_log_usb_print_buffer(const uint8_t* buf, uint16_t len);
    if (size > 0)
    {
        tlk_debug_log_usb_print_buffer((uint8_t*) tlk_log_buffer, size);
    }
#endif
}

#endif

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_HIGH)

static void tlk_debug_log_timer_shot(void* arg)
{
    (void) arg;
    tlk_debug_log_pump();
}
TLK_SW_TIMER_REGISTER(log_timer, tlk_debug_log_timer_shot, SW_TIMER_MODE_PERIODIC,
                      TASK_MS(CONFIG_TLK_DEBUG_LOG_PERIODIC_PUMP_PERIOD))
#endif

void tlk_debug_log_init(void)
{
#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_HIGH)
    tlk_task_sw_timer_start(&log_timer);
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_USB_CDC)
    extern void tlk_debug_log_usb_cdc_init(void);
    tlk_debug_log_usb_cdc_init();
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_LOW) || TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_HIGH)
    s_wr = 0;
    s_rd = 0;
#endif
}

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_USB_CDC) &&                                      \
    TLK_IS_DISABLED(CONFIG_TLK_TASK_PLANNER)
void tlk_debug_log_interface_loop(void)
{
    extern void tlk_debug_log_usb_run(void);
    tlk_debug_log_usb_run();
}
#endif

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_LOW) || TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_HIGH)

static bool tlk_debug_log_event_push(const tlk_log_event_t* e)
{
    uint32_t cs = tlk_core_mie_disable(TLK_FLD_MIE_MSIE | TLK_FLD_MIE_MTIE | TLK_FLD_MIE_MEIE);

    uint32_t wr = s_wr;
    uint32_t rd = s_rd;

    /* full when next wr equals rd */
    uint32_t next = (wr + 1u) % CONFIG_TLK_DEBUG_LOG_QUEUE_SIZE;
    if (next == rd)
    {
        tlk_core_mie_enable(cs);
        return false; /* drop */
    }

    s_q[wr] = *e;
    s_wr    = next;

    tlk_core_mie_enable(cs);
    return true;
}

static bool tlk_debug_log_event_pop(tlk_log_event_t* out)
{
    uint32_t cs = tlk_core_mie_disable(TLK_FLD_MIE_MSIE | TLK_FLD_MIE_MTIE | TLK_FLD_MIE_MEIE);

    uint32_t rd = s_rd;
    uint32_t wr = s_wr;

    if (rd == wr)
    {
        tlk_core_mie_enable(cs);
        return false;
    }

    *out = s_q[rd];
    s_rd = (rd + 1u) % CONFIG_TLK_DEBUG_LOG_QUEUE_SIZE;

    tlk_core_mie_enable(cs);
    return true;
}

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_FROM_ISR)
void tlk_debug_log_isr(const tlk_log_t* log, tlk_log_level_t level, const char* msg)
{
    tlk_log_event_t e = {0};
    e.ts_us           = tlk_debug_log_get_us();
    e.module          = log ? log->module : "?";
    e.level           = level;
    e.is_literal      = true;
    e.msg             = e.msg_buf;
    e.msg             = msg ? msg : "";
    e.msg_len         = strlen(e.msg);

    (void) tlk_debug_log_event_push(&e); /* drop if full */
}
#endif

/* ---- Task/thread log: format into event buffer, then enqueue ---- */
void tlk_debug_log_queue(const tlk_log_t* log, tlk_log_level_t level, const char* fmt, ...)
{
    /* If someone accidentally calls this from ISR, fall back to literal */
#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_FROM_ISR)
    if (tlk_plic_is_in_isr())
    {
        tlk_debug_log_isr(log, level, "tlk_log called from ISR (use tlk_log_isr)");
        return;
    }
#endif

    tlk_log_event_t e = {0};
    e.ts_us           = tlk_debug_log_get_us();
    e.module          = log ? log->module : "?";
    e.level           = level;
    e.is_literal      = false;

    int16_t off = snprintf(e.msg_buf,
                           sizeof(e.msg_buf),
                           "[%05lu.%03u.%03u]: [%s]: [%s]: ",
                           (uint32_t) ((e.ts_us / 1000000) % 100000),
                           (uint16_t) ((e.ts_us / 1000u) % 1000u),
                           (uint16_t) (e.ts_us % 1000u),
                           e.module ? e.module : "?",
                           tlk_debug_log_level_str_get(e.level));

    if (off < 0)
    {
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    int16_t size = off + vsnprintf(e.msg_buf + off, sizeof(e.msg_buf) - off, fmt, ap);
    va_end(ap);

    if (size < 0)
    {
        return;
    }

    if ((uint16_t) size < sizeof(e.msg_buf) - 2)
    {
        e.msg_buf[size++] = '\r';
        e.msg_buf[size++] = '\n';
        e.msg_buf[size]   = '\0';
    }
    else
    {
        e.msg_buf[sizeof(e.msg_buf) - 3] = '\r';
        e.msg_buf[sizeof(e.msg_buf) - 2] = '\n';
        e.msg_buf[sizeof(e.msg_buf) - 1] = '\0';
    }

    e.msg_len = size;
    e.msg     = e.msg_buf;
    (void) tlk_debug_log_event_push(&e);
}

void tlk_debug_log_pump(void)
{
#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_UART)
    enum tlk_uart_status uart_log_status = TLK_UART_OK;
#else
    extern uint16_t tlk_debug_log_usb_print_buffer(const uint8_t* buf, uint16_t len);
#endif

    tlk_log_event_t e;

    if (!tlk_log_interface_is_locked())
    {
        while (tlk_debug_log_event_pop(&e))
        {
            if (e.is_literal)
            {
                static char tlk_log_buffer[CONFIG_TLK_DEBUG_LOG_BUFFER_SIZE];
                int16_t     off = snprintf(tlk_log_buffer,
                                           CONFIG_TLK_DEBUG_LOG_BUFFER_SIZE,
                                           "[%05lu.%03u.%03u]: [IRQ]: [%s]: [%s]: %s",
                                           (unsigned int) ((e.ts_us / 1000000) % 100000),
                                           (unsigned short) (e.ts_us / 1000u) % 1000u,
                                           (unsigned short) (e.ts_us % 1000u),
                                           e.module ? e.module : "?",
                                           tlk_debug_log_level_str_get(e.level),
                                           e.msg);

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_UART)
                tlk_log_uart_lock();
                uart_log_status = tlk_uart_send_bytes(CONFIG_TLK_DEBUG_UART,
                                                      (const uint8_t*) tlk_log_buffer,
                                                      off,
                                                      tlk_debug_log_tx_handle,
                                                      TLK_USEC(5));
#elif TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_USB_CDC)
                if (off > 0)
                    tlk_debug_log_usb_print_buffer((uint8_t*) tlk_log_buffer, off);
#endif
            }
            else
            {
#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_UART)
                tlk_log_uart_lock();
                uart_log_status = tlk_uart_send_bytes(CONFIG_TLK_DEBUG_UART,
                                                      (const uint8_t*) e.msg_buf,
                                                      e.msg_len,
                                                      tlk_debug_log_tx_handle,
                                                      TLK_USEC(5));
#elif TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_USB_CDC)
                if (e.msg_len > 0)
                    tlk_debug_log_usb_print_buffer((uint8_t*) e.msg_buf, e.msg_len);
#endif
            }

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_UART)
            if (uart_log_status != TLK_UART_OK)
            {
                // restart UART TX
                tlk_log_uart_unlock();
            }
#endif
        }
    }
}

#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_PRIO_LOW)
#if TLK_IS_ENABLED(CONFIG_TLK_TASK_PLANNER_MODE_LOOP)
#include "common/include/tlk_loop.h"
TLK_REGISTER_LOOP(tlk_debug_log_pump)
#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_USB_CDC)
extern void tlk_debug_log_usb_run(void);
TLK_REGISTER_LOOP(tlk_debug_log_usb_run);
#endif
#elif TLK_IS_ENABLED(CONFIG_TLK_TASK_PLANNER_MODE_SCHEDULER)
#include "system/task_planner/task_planner.h"
static void tlk_debug_log_pump_task(tTask* t)
{
    tlk_debug_log_pump();
    TASK_DELAY(t, TASK_MS(CONFIG_TLK_DEBUG_LOG_PERIODIC_PUMP_PERIOD));
}
#if TLK_IS_ENABLED(CONFIG_TLK_DEBUG_LOG_INTERFACE_USB_CDC)
static void tlk_debug_log_interface_task(tTask* t)
{
    extern void tlk_debug_log_usb_run(void);
    tlk_debug_log_usb_run();
    TASK_DELAY(t, TASK_MS(5));
}
TASK_SPAWN(log_interface_task, tlk_debug_log_interface_task)
#endif

TASK_SPAWN(log_task, tlk_debug_log_pump_task)
#endif
#endif
#endif
TLK_REGISTER_PRE_INIT(tlk_debug_log_init, TLK_INIT_LEVEL_APPLICATION, TLK_INIT_PRIORITY_HIGH)

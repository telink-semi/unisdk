menuconfig TLK_PM
    bool "PM"
    select TLK_PLIC
    select TLK_SYSTEM_TIMER_ADVANCED_MODE


if TLK_PM

# Suspend mode
config TLK_PM_SUSPEND
    bool "Suspend"
    default y
    select TLK_SYSTEM_TIMER_ADVANCED_MODE
    select TLK_SYSTEM_TIMER_ADVANCED_USE_32K_TICK_ANALOG_READOUT


# Suspend minimal duration
config TLK_PM_SUSPEND_MIN_DURATION_MS
    int "Suspend minimal duration (ms)"
    depends on TLK_PM_SUSPEND
    default 2

config TLK_PM_SUSPEND_WAKEUP_COST_MS
    int "Suspend wakeup duration (ms)"
    depends on TLK_PM_SUSPEND
    default 1


# Deep sleep mode
config TLK_PM_DEEP_SLEEP
    bool "Deep sleep"


if TLK_PM_DEEP_SLEEP

# Deep sleep minimal duration
config TLK_PM_DEEP_SLEEP_MIN_DURATION_MS
    int "Deep sleep minimal duration (ms)"
    default 3


# Deep sleep force to suspend  if deep sleep is not able to start
config TLK_PM_ALLOW_FALLBACK_TO_SUSPEND
    bool "PM force Suspend mode if deep sleep is not able to start"
    depends on TLK_PM_SUSPEND


# Deep sleep with RAM retention
config TLK_PM_RAM_RETENTION_ENABLE
    bool "Enable RAM retention for deep sleep"


if TLK_PM_RAM_RETENTION_ENABLE

# Deep sleep RAM retention minimal duration
config TLK_PM_DEEP_RETENTION_MIN_DURATION_MS
    int "Deep retention minimal duration (ms)"
    default 20

config TLK_PM_DEEP_RETENTION_WAKEUP_COST_MS
    int "Deep retention wakeup duration (ms)"
    default 3


# Deep sleep RAM retention memory size selection
choice
    prompt "Retention memory size"

    config TLK_PM_RETENTION_MEMORY_SIZE_32K
    bool "32K"
    config TLK_PM_RETENTION_MEMORY_SIZE_64K
    bool "64K"
    config TLK_PM_RETENTION_MEMORY_SIZE_96K
    bool "96K"

endchoice # Retention memory size


# Selected deep sleep RAM retention memory size
config TLK_PM_RETENTION_MEMORY_SIZE
    int
    default 32 if TLK_PM_RETENTION_MEMORY_SIZE_32K
    default 64 if TLK_PM_RETENTION_MEMORY_SIZE_64K
    default 96 if TLK_PM_RETENTION_MEMORY_SIZE_96K

endif # TLK_PM_RAM_RETENTION_ENABLE

endif # TLK_PM_DEEP_SLEEP

endif # TLK_PM
.PHONY: cmake config pinmux build clean cleanbuild pristine pristine-config

# Export TELINK_BASE so it's available as an environment variable for cmake.
# This is used by root CMakeLists.txt to restore Telink_DIR cache variable,
# making it survive CMakeCache deletion/regeneration.
export TELINK_BASE := $(CURDIR)

# Default application to build
APP ?= samples/gpio_demo

# SOC and BOARD parameters (empty by default)
SOC ?=
BOARD ?=

# Extra CMake flags (empty by default)
EXTRA_CMAKE_FLAGS ?=

# Parse Kconfigs for all samples (used e.g. by SDK export script)
PARSE_ALL_SAMPLE_KCONFIGS ?= OFF

# CMake configuration
CMAKE := cmake
BUILD_DIR := build
CMAKE_FLAGS := -B $(BUILD_DIR) -G Ninja -DTelink_DIR='$(CURDIR)/cmake' $(EXTRA_CMAKE_FLAGS)

ifeq ($(PARSE_ALL_SAMPLE_KCONFIGS),ON)
CMAKE_FLAGS += -DPARSE_ALL_SAMPLE_KCONFIGS=ON
endif

# Add SOC and BOARD flags if they are set
ifneq ($(SOC),)
CMAKE_FLAGS += -DSOC=$(SOC)
endif

ifneq ($(BOARD),)
CMAKE_FLAGS += -DBOARD=$(BOARD)
endif

# Python scripts
PYTHON := python

# Ensure cmake environment is properly set up
ENSURE_CMAKE := @test -d "$(BUILD_DIR)" && test -f "$(BUILD_DIR)/CMakeCache.txt" || make cmake

# Detect an application switch: when the cached application differs from the
# requested APP, remove the CMake cache and the derived configuration OUTPUT
# files (.config / autoconf.h / the preprocessed Kconfig tree), while KEEPING
# build.config and chip.config as the merge baseline. On the next configure
# build.config is re-merged with the new application's Kconfig tree
# (kconfig.cmake / kconfig.py): user-set options are preserved, options from
# the previous application (e.g. CONFIG_TLK_GPIO_DEMO_*) are dropped, and the
# new application's options (e.g. CONFIG_TLK_UART_DEMO_*) take effect. This
# preserves the intended .config reuse instead of discarding user config.
APP_CHANGE_CLEAN := @if [ -f "$(BUILD_DIR)/CMakeCache.txt" ]; then cached_app=$$(grep -m1 '^APPLICATION_SOURCE_DIR:' "$(BUILD_DIR)/CMakeCache.txt" | cut -d'=' -f2- | tr -d '\r'); if [ -n "$$cached_app" ] && [ "$$cached_app" != "$(abspath $(APP))" ]; then if [ -t 1 ]; then C_YEL='\033[33m'; C_RST='\033[0m'; else C_YEL=''; C_RST=''; fi; echo "$${C_YEL}Warning: Detected application change: '$$cached_app' -> '$(abspath $(APP))'$${C_RST}"; echo "$${C_YEL}Removing CMake cache and regenerating the configuration for the new application; user-set options in build.config are kept and re-merged.$${C_RST}"; $(CMAKE) -E remove -f "$(BUILD_DIR)/CMakeCache.txt" "$(BUILD_DIR)/.config" "$(BUILD_DIR)/autoconf.h" "$(BUILD_DIR)/"*.pinmux; fi; fi

# Targets
cmake:
	@rm -rf "$(BUILD_DIR)/CMakeCache.txt" "$(BUILD_DIR)/CMakeFiles" 2>/dev/null || true
	$(CMAKE) $(CMAKE_FLAGS) -S $(APP)

reconfigure:
	$(CMAKE) $(CMAKE_FLAGS) -S $(APP)

config:
	$(ENSURE_CMAKE)
	$(CMAKE) --build $(BUILD_DIR) --target config

config_chip:
	$(ENSURE_CMAKE)
	$(CMAKE) --build $(BUILD_DIR) --target config_chip

config_build:
	$(ENSURE_CMAKE)
	$(CMAKE) --build $(BUILD_DIR) --target config_build

pinmux:
	$(ENSURE_CMAKE)
	$(CMAKE) --build $(BUILD_DIR) --target generate_dotpinmux

build:
	$(ENSURE_CMAKE)
	@$(CMAKE) --build $(BUILD_DIR)

# clean: remove compiled artifacts (.o/Telink.elf) and the generated
# configuration files (chip.config, build.config, .config, autoconf.h,
# Kconfig/, registered via ADDITIONAL_CLEAN_FILES), while keeping the CMake
# cache, build.ninja and generated directories. The next 'make build'
# reconfigures and recompiles from the current configuration.
# Use when you want to rebuild everything but keep the build directory.
clean:
	$(ENSURE_CMAKE)
	@$(CMAKE) --build $(BUILD_DIR) --target clean

# cleanbuild: convenience wrapper for 'clean' followed by 'build' - wipe
# artifacts and generated configs, then reconfigure and rebuild from scratch.
cleanbuild: clean build

# pristine: remove the ENTIRE build directory (configuration, CMake cache,
# derived configs, compiled artifacts and *.pinmux files), so the next build
# configures and compiles from scratch. Requires no existing build state.
# Use for a full reset: switching APP, chasing config pollution, or shipping a
# clean workspace. See also 'pristine-config' below for the config-only reset.
pristine:
	@rm -rf $(BUILD_DIR) 2>/dev/null || rd /S /Q "$(BUILD_DIR)" 2>NUL || true
	@echo "Pristine: removed $(BUILD_DIR). Next 'make build' will configure from scratch."

# pristine-config: remove only the generated configuration files
# (chip.config, build.config, .config, autoconf.h, Kconfig/, pinmux.h) via the
# CMake 'pristine-config' target, while keeping the CMake cache, compiled
# artifacts and user *.pinmux files. The next configure regenerates the
# configs quickly and the existing artifacts are reused.
# Use to reset the configuration (e.g. after menuconfig) without recompiling.
# Note: SOC/BOARD changes are already handled automatically at configure time.
pristine-config:
	$(ENSURE_CMAKE)
	$(CMAKE) --build $(BUILD_DIR) --target pristine-config

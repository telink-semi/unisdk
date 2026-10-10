.PHONY: cmake reconfigure config pinmux build clean cleanbuild pristine pristine-config

# Export TELINK_BASE so it's available as an environment variable for cmake.
# This is used by root CMakeLists.txt to restore Telink_DIR cache variable,
# making it survive CMakeCache deletion/regeneration.
export TELINK_BASE := $(CURDIR)

# Default application to build
APP ?= samples/gpio_demo

# SOC_SERIES, SOC and BOARD parameters (empty by default)
SOC_SERIES ?=
SOC ?=
BOARD ?=

# Extra CMake flags (empty by default)
EXTRA_CMAKE_FLAGS ?=

# CMake configuration
CMAKE := cmake

# Build directory. Always the top-level 'build', whichever APP is selected -
# switching APP reuses it (see APP_CHANGE_CLEAN below), so every sample built
# from here shares one build directory and one chip.config, e.g.:
#   make APP=samples/i2c_demo/ds1307 build
# still builds into ./build. A standalone project created by "+ New Project"
# has its own Makefile and builds into its own build/ instead. Override
# BUILD_DIR directly if you want something else.
BUILD_DIR ?= build

CMAKE_FLAGS := -B $(BUILD_DIR) -G Ninja -DTelink_DIR='$(CURDIR)/cmake' $(EXTRA_CMAKE_FLAGS)

# Add SOC_SERIES, SOC and BOARDD flags if they are set
ifneq ($(SOC_SERIES),)
CMAKE_FLAGS += -DSOC_SERIES=$(SOC_SERIES)
endif

ifneq ($(SOC),)
CMAKE_FLAGS += -DSOC=$(SOC)
endif

ifneq ($(BOARD),)
CMAKE_FLAGS += -DBOARD=$(BOARD)
endif

# Point cmake at this checkout's own venv Python explicitly, rather than
# relying on cmake/modules/python.cmake's VIRTUAL_ENV-env-var-based
# discovery (which silently falls back to whatever "python"/"python3" is on
# PATH - e.g. a system Python without kconfiglib installed - in any shell
# that never activated the venv). Keeps `make cmake`/`make build` working
# regardless of terminal activation state.
ifeq ($(OS),Windows_NT)
PYTHON_IN_VENV := $(CURDIR)/.venv/Scripts/python.exe
else
PYTHON_IN_VENV := $(CURDIR)/.venv/bin/python
endif
ifneq ($(wildcard $(PYTHON_IN_VENV)),)
CMAKE_FLAGS += -DPython3_EXECUTABLE='$(PYTHON_IN_VENV)'
endif

# Python scripts
PYTHON := python

# Ensure cmake environment is properly set up
# Uses reconfigure rather than cmake: the cache can be missing because
# APP_CHANGE_CLEAN just removed it (or the Kconfig/Pinmux editors wrote
# build.config/*.pinmux into a fresh build directory), and cmake's target
# would delete exactly the configuration those are meant to keep.
ENSURE_CMAKE := @test -d "$(BUILD_DIR)" && test -f "$(BUILD_DIR)/CMakeCache.txt" || $(MAKE) reconfigure

# Seed build.config when it's missing - a fresh build directory, `make cmake`,
# or an application switch (APP_CHANGE_CLEAN) - from the application's
# prj.conf, non-interactively (kconfig.cmake's config_build_default target),
# then reconfigure so cmake picks it up. Without it build.config only came
# from menuconfig (`make config`/`config_build`) and `make build` failed.
SEED_BUILD_CONFIG := @test -f "$(BUILD_DIR)/build.config" || { $(CMAKE) --build $(BUILD_DIR) --target config_build_default && $(CMAKE) $(CMAKE_FLAGS) -S $(APP) > /dev/null; }

# Detect an application switch: when the cached application differs from the
# requested APP, remove the CMake cache, build.config and the derived
# configuration output (.config / autoconf.h / *.pinmux), while KEEPING
# chip.config (the SoC/board choice is shared by every sample). build.config
# is per-application: dropping it lets the next configure seed it from the
# new application's prj.conf (kconfig.cmake's _parse_prj_conf), the same as
# a fresh build directory would.
APP_CHANGE_CLEAN := @if [ -f "$(BUILD_DIR)/CMakeCache.txt" ]; then cached_app=$$(grep -m1 '^APPLICATION_SOURCE_DIR:' "$(BUILD_DIR)/CMakeCache.txt" | cut -d'=' -f2- | tr -d '\r'); if [ -n "$$cached_app" ] && [ "$$cached_app" != "$(abspath $(APP))" ]; then if [ -t 1 ]; then C_YEL='\033[33m'; C_RST='\033[0m'; else C_YEL=''; C_RST=''; fi; echo "$${C_YEL}Warning: Detected application change: '$$cached_app' -> '$(abspath $(APP))'$${C_RST}"; echo "$${C_YEL}Removing CMake cache and build.config; the configuration is regenerated from the new application's prj.conf (chip.config is kept).$${C_RST}"; $(CMAKE) -E remove -f "$(BUILD_DIR)/CMakeCache.txt" "$(BUILD_DIR)/build.config" "$(BUILD_DIR)/.config" "$(BUILD_DIR)/autoconf.h" "$(BUILD_DIR)/"*.pinmux; fi; fi

# Targets

# cmake: Prepare the build directory for a (possibly new) sample.
# Every sample is treated as an independent project, so drop the CMake cache
# and all per-application generated config output (build.config, autoconf.h,
# pinmux.h, *.pinmux) to avoid stale config from the previous sample.
# TODO: Temporary chip.config and generated/ are kept on purpose for faster iteration.
cmake:
	@rm -rf "$(BUILD_DIR)/CMakeCache.txt" "$(BUILD_DIR)/CMakeFiles" \
		"$(BUILD_DIR)/build.config" \
		"$(BUILD_DIR)/pinmux.h" "$(BUILD_DIR)/"*.pinmux 2>/dev/null || true
	$(CMAKE) $(CMAKE_FLAGS) -S $(APP)
	$(SEED_BUILD_CONFIG)

reconfigure:
	$(CMAKE) $(CMAKE_FLAGS) -S $(APP)

config:
	$(APP_CHANGE_CLEAN)
	$(ENSURE_CMAKE)
	$(CMAKE) --build $(BUILD_DIR) --target config_chip
	$(CMAKE) --build $(BUILD_DIR) --target config_build
	$(CMAKE) --build $(BUILD_DIR) --target generate_autoconf
	$(CMAKE) --build $(BUILD_DIR) --target update_dotpinmux

config_chip:
	$(APP_CHANGE_CLEAN)
	$(ENSURE_CMAKE)
	$(CMAKE) --build $(BUILD_DIR) --target config_chip

config_build:
	$(APP_CHANGE_CLEAN)
	$(ENSURE_CMAKE)
	$(CMAKE) --build $(BUILD_DIR) --target config_build

pinmux:
	$(APP_CHANGE_CLEAN)
	$(ENSURE_CMAKE)
	$(CMAKE) --build $(BUILD_DIR) --target generate_dotpinmux

build:
	$(APP_CHANGE_CLEAN)
	$(ENSURE_CMAKE)
	@$(CMAKE) --build $(BUILD_DIR)

# clean: remove compiled artifacts (.o/Telink.elf) and the generated
# configuration files (chip.config, build.config, .config, autoconf.h,
# Kconfig/, registered via ADDITIONAL_CLEAN_FILES), while keeping the CMake
# cache, build.ninja and generated directories. The next 'make build'
# reconfigures and recompiles from the current configuration.
# Use when you want to rebuild everything but keep the build directory.
clean:
	$(APP_CHANGE_CLEAN)
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
	$(APP_CHANGE_CLEAN)
	$(ENSURE_CMAKE)
	$(CMAKE) --build $(BUILD_DIR) --target pristine-config

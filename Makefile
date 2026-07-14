.PHONY: cmake config pinmux build clean cleanbuild doc doc-pdf doc-serve

# Default application to build
APP ?= samples/gpio_demo

# SOC and BOARD parameters (empty by default)
SOC ?=
BOARD ?=

# Extra CMake flags (empty by default)
EXTRA_CMAKE_FLAGS ?=

# CMake configuration
CMAKE := cmake
BUILD_DIR := build
CMAKE_FLAGS := -B $(BUILD_DIR) -G Ninja -DTelink_DIR='$(CURDIR)/cmake' $(EXTRA_CMAKE_FLAGS)

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

# Targets
cmake:
	@rm -rf $(BUILD_DIR)/CMakeCache.txt 2>/dev/null || rd /S /Q "$(BUILD_DIR)/CMakeCache.txt" 2>NUL || true
	$(CMAKE) $(CMAKE_FLAGS) -S $(APP)

config:
	$(ENSURE_CMAKE)
	$(CMAKE) --build $(BUILD_DIR) --target config

pinmux:
	$(ENSURE_CMAKE)
	$(CMAKE) --build $(BUILD_DIR) --target generate_dotpinmux

build:
	$(ENSURE_CMAKE)
	$(CMAKE) --build $(BUILD_DIR)

clean:
	$(ENSURE_CMAKE)
	$(CMAKE) --build $(BUILD_DIR) --target clean

cleanbuild: clean build

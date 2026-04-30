#!/bin/bash

set -e

# ==============================================================================
# Argument Validation
# Check if all three arguments (SOC, Sample, Config) are provided.
# If any argument is missing, stop the script with an error.
# ==============================================================================
SOC_SERIES=$1
SAMPLE_NAME=$2
CONFIG_NAME=$3

if [ -z "$SOC_SERIES" ] || [ -z "$SAMPLE_NAME" ] || [ -z "$CONFIG_NAME" ]; then
    echo "[ci_prepare.sh] ERROR: Invalid arguments"
    exit 1
fi

# ==============================================================================
# Path Definitions
# ==============================================================================
GEN_CONFIG_SCRIPT="scripts/ci/defconfig_to_dotconfig.py"                        # Script for generating build.config from the .defconfig
KCONFIG_BUILD_PATH="Kconfig.build"                                              # Path to root of the config tree
CHIP_CONFIG_PATH="ci_configs/$SOC_SERIES/chip.config"                           # Path to the general chip.config
DEF_CONFIG_PATH="ci_configs/$SOC_SERIES/$SAMPLE_NAME/$CONFIG_NAME.defconfig"    # Path to the current sample configuration source

DEST_CHIP_CONFIG_PATH="build/chip.config"                                       # Path to the chip.config used by preprocess_kconfig.py
DEST_BUILD_CONFIG_PATH="build/build.config"                                     # Path to the generated build.config
DEST_DOT_CONFIG_PATH="build/.config"                                            # Path to the .config file used for build

CHIP_CACHE_BASE_DIR=".chip_cache"                                               # Path to the chip cache
CHIP_CACHE_DIR="$CHIP_CACHE_BASE_DIR/${SOC_SERIES}"                             # Directory with the chip cache
CACHE_KCONFIG_PATH="$CHIP_CACHE_DIR/Kconfig"                                    # Path to the cached Kconfig
DEST_KCONFIG_PATH="build/Kconfig/"                                              # Path to the Kconfig used for GEN_CONFIG

export KCONFIG_BINARY_DIR="$DEST_KCONFIG_PATH"
export KCONFIG_APPLICATION_DIR="samples/${SAMPLE_NAME}_demo"

# ==============================================================================
# Files preparing
# Copy necessary files for build.
# If the file doesn't exist or copying fails — exit with an error.
# ==============================================================================
mkdir -p build

#======
# chip.config validation and copy
#======
if [ ! -f "$CHIP_CONFIG_PATH" ]; then
    echo "[ci_prepare.sh] ERROR: chip.config not found at $CHIP_CONFIG_PATH"
    exit 1
fi

echo "[ci_prepare.sh] Found chip.config: $CHIP_CONFIG_PATH"

if ! cp "$CHIP_CONFIG_PATH" "$DEST_CHIP_CONFIG_PATH"; then
    echo "[ci_prepare.sh] ERROR: Failed to copy chip.config"
    exit 1
fi

#======
# Check if chip cache exists and is valid
#======
echo "[ci_prepare.sh] Using cache directory: $CHIP_CACHE_DIR"

if [ ! -d "$CACHE_KCONFIG_PATH" ]; then
    echo "[ci_prepare.sh] ERROR: Chip cache files not found"
    exit 1
fi

rm -rf "$DEST_KCONFIG_PATH"
if ! cp -r "$CACHE_KCONFIG_PATH" "$DEST_KCONFIG_PATH"; then
    echo "[ci_prepare.sh] ERROR: Failed to copy Kconfig directory to build directory"
    exit 1
fi

#======
# .config file preparing (defconfig)
#======
if [ ! -f "$DEF_CONFIG_PATH" ]; then
    echo "[ci_prepare.sh] ERROR: $CONFIG_NAME.defconfig not found at $DEF_CONFIG_PATH"
    exit 2
fi

python3 $GEN_CONFIG_SCRIPT "$KCONFIG_BUILD_PATH" "$DEF_CONFIG_PATH" "$DEST_BUILD_CONFIG_PATH"

if [ ! -f "$DEST_BUILD_CONFIG_PATH" ]; then
    echo "[ci_prepare.sh] ERROR: build.config not found at $DEST_BUILD_CONFIG_PATH"
    exit 1
fi

cat "$DEST_CHIP_CONFIG_PATH" "$DEST_BUILD_CONFIG_PATH" > "$DEST_DOT_CONFIG_PATH" 

exit 0
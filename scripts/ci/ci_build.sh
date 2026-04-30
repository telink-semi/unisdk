#!/bin/bash

set -e

# ==============================================================================
# Argument Validation
# Check if all four arguments (SOC, Sample, Config, Board) are provided.
# If any argument is missing, stop the script with an error.
# ==============================================================================
SOC_SERIES=$1
SAMPLE_NAME=$2
CONFIG_NAME=$3
BOARD=$4

if [ -z "$SOC_SERIES" ] || [ -z "$SAMPLE_NAME" ] || [ -z "$CONFIG_NAME" ] || [ -z "$BOARD" ]; then
    echo "[ci_build.sh] ERROR: Invalid arguments"
    exit 1
fi

# ==============================================================================
# Path Definitions
# ==============================================================================
LOG_PATH="ci_logs/$SOC_SERIES/$SAMPLE_NAME/$CONFIG_NAME.log"
SAMPLE_DIR="samples/${SAMPLE_NAME}_demo"

PINMUX_FOR_SOC_PATH="ci_configs/$SOC_SERIES/$BOARD.board.pinmux"                # Path to the default pinmux configuration
DEST_PINMUX_PATH="build/$BOARD.board.pinmux"                                    # Path to the .pinmux file used for build

mkdir -p "$(dirname "$LOG_PATH")"

echo "[ci_build.sh] Log file: $LOG_PATH"

# ==============================================================================
# Build Execution
# Temporarily disable 'set -e' to capture the make exit code.
# Run the build and write all output to the log file.
# ==============================================================================
set +e
make cmake APP=$SAMPLE_DIR > "$LOG_PATH" 2>&1

# .pinmux file preparing
if [ -f "$PINMUX_FOR_SOC_PATH" ]; then
    echo "[ci_prepare.sh] Found .pinmux: $PINMUX_FOR_SOC_PATH"
    
    if ! cp "$PINMUX_FOR_SOC_PATH" "$DEST_PINMUX_PATH"; then
        echo "[ci_prepare.sh] ERROR: Failed to copy .pinmux"
        exit 1
    fi

else
    echo "[ci_prepare.sh] ERROR: .pinmux not found"
    exit 1
fi


cmake --build build --target autogenerate_dotpinmux
make build >> "$LOG_PATH" 2>&1
BUILD_STATUS=$?
set -e

# ==============================================================================
# Status Check
# Check the build result:
# - If exit code is 0 — build succeeded.
# - Otherwise show an error and point to the log file.
# ==============================================================================
if [ $BUILD_STATUS -eq 0 ]; then
    echo "[ci_build.sh] BUILD SUCCESS"
else
    echo "[ci_build.sh] BUILD ERROR"
    echo "[ci_build.sh] Check the detailed in log file: $LOG_PATH"
    exit 1
fi

exit 0
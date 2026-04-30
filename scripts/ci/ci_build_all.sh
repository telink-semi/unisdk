#!/bin/bash

set -e

PREPARE_SCRIPT="./scripts/ci/ci_prepare.sh"                 # Script to prepare the environment for build
BUILD_SCRIPT="./scripts/ci/ci_build.sh"                     # Script for building sample
CHIP_CACHE_BASE_DIR=".chip_cache"                           # Path to the chip cache
PREPROCESS_KCONFIG_SCRIPT="scripts/preprocess_kconfig.py"   # Script to preprocess Kconfigs
LIB_GEN_SCRIPT="./scripts/lib_builder/lib_builder.sh"

SOC_SERIES=("TL321X" "TL721X" "TLSR952X")
BOARDS=("TL3218X_EVK" "TL7218X_EVK" "TLSR9528A_EVK")
SAMPLE_NAMES=("gpio" "pm" "uart" "irq_nesting" "tinyusb_demos/usb_cdc") 
CONFIG_NAMES=("min" "suspend" "retention" "max" "max_all")

echo "START ALL BUILDS"
echo "========================================="

mkdir -p "$CHIP_CACHE_BASE_DIR"

for i in "${!SOC_SERIES[@]}"; do
    soc_series="${SOC_SERIES[i]}"
    board="${BOARDS[i]}"

    echo
    echo "[ci_build_all.sh] Generating chip cache for $soc_series..."
    
    CHIP_CACHE_DIR="$CHIP_CACHE_BASE_DIR/$soc_series"
    CHIP_CONFIG_PATH="ci_configs/$soc_series/chip.config"
    CHIP_CONFIG_CACHE_PATH="$CHIP_CACHE_DIR/chip.config"

    mkdir -p "$CHIP_CACHE_DIR"

    if ! cp "$CHIP_CONFIG_PATH" "$CHIP_CONFIG_CACHE_PATH"; then
        echo "[ci_build_all.sh] ERROR: Failed to copy chip.config to cache"
        exit 1
    fi
        
    if ! python3 $PREPROCESS_KCONFIG_SCRIPT \
        --root-dir "$(pwd)" \
        --build-dir  "$(pwd)/$CHIP_CACHE_DIR"; then
        echo "[ci_build_all.sh] ERROR: Failed to generate chip cache"
        exit 1
    fi

    for sample in "${SAMPLE_NAMES[@]}"; do
        for config in "${CONFIG_NAMES[@]}"; do
            echo
            echo " === $soc_series $sample $config === "

            set +e
            $PREPARE_SCRIPT "$soc_series" "$sample" "$config"
            ret=$?
            set -e

            if [ $ret -eq 1 ]; then
                echo "[ci_build_all.sh] Error: preparing failed (fatal)"
                exit 1
            elif [ $ret -eq 2 ]; then
                echo "[ci_build_all.sh] Warning: preparing failed (skip, continue)"
                continue
            fi

            $BUILD_SCRIPT "$soc_series" "$sample" "$config" "$board" 
        done
    done
done

if [ -f "$LIB_GEN_SCRIPT" ]; then
    for soc_series in "${SOC_SERIES[@]}"; do
        echo
        echo "[ci_build_all.sh] Generating library for $soc_series..."
        $LIB_GEN_SCRIPT "$soc_series"
    done
fi

echo
echo "========================================="
echo "ALL BUILDS COMPLETED"

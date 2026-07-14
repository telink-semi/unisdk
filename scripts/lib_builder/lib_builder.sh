#!/bin/bash

set -e

PREPROCESS_KCONFIG_SCRIPT="scripts/preprocess_kconfig.py"
GEN_DOT_CONFIG_SCRIPT="scripts/ci/defconfig_to_dotconfig.py"
LIB_DEF_CONFIG_PATH="scripts/lib_builder/configs/library.defconfig"
KCONFIG_BUILD_PATH="Kconfig.build"
BUILD_DIR="build"

if python3 -c "import sys" &>/dev/null 2>&1; then
    PYTHON=python3
elif python -c "import sys" &>/dev/null 2>&1; then
    PYTHON=python
else
    echo "[lib_builder.sh] Python not found"
    exit 1
fi

# Parse arguments
TOOLCHAIN_TYPE="andes"
SOC_SERIES=""
while [[ $# -gt 0 ]]; do
    case "$1" in
        -t|--toolchain-type)
            TOOLCHAIN_TYPE="$2"
            shift 2
            ;;
        *)
            SOC_SERIES="$1"
            shift
            ;;
    esac
done

if [ -z "$SOC_SERIES" ]; then
    echo "[lib_builder.sh] Usage: $0 [-t andes|zephyr] <SOC_SERIES>"
    exit 1
fi

LIB_CHIP_CONFIG_PATH="scripts/lib_builder/configs/${SOC_SERIES}/chip.config"

if [ ! -d "soc" ]; then
    echo "[lib_builder.sh] Run script from SDK root"
    exit 1
fi

if [ ! -d "soc/${SOC_SERIES}" ]; then
    echo "[lib_builder.sh] Invalid SOC series: ${SOC_SERIES}"
    exit 1
fi

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

cp "$LIB_CHIP_CONFIG_PATH" "$BUILD_DIR/chip.config"

$PYTHON "$PREPROCESS_KCONFIG_SCRIPT" \
    --root-dir "$(pwd)" \
    --build-dir "$(pwd)/$BUILD_DIR"

KCONFIG_BINARY_DIR="$BUILD_DIR/Kconfig/" \
$PYTHON "$GEN_DOT_CONFIG_SCRIPT" "$KCONFIG_BUILD_PATH" "$LIB_DEF_CONFIG_PATH" "$BUILD_DIR/build.config"

echo "# Auto-generated .config file" > "$BUILD_DIR/.config"
cat "$BUILD_DIR/chip.config" >> "$BUILD_DIR/.config"
echo "" >> "$BUILD_DIR/.config"
cat "$BUILD_DIR/build.config" >> "$BUILD_DIR/.config"
echo "" >> "$BUILD_DIR/.config"
echo "CONFIG_TLK_TOOLCHAIN_TYPE=\"${TOOLCHAIN_TYPE}\"" >> "$BUILD_DIR/.config"

make cmake BUILD_DIR="$BUILD_DIR"
cmake --build "$BUILD_DIR" --target build_soc_libraries

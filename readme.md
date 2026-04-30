# Unified SDK

## Overview

UniSDK adopts a modular and extensible architecture design that integrates multiple tools such as Make, CMake, Ninja, West, and Kconfig to form a complete compilation and build workflow. The system supports multiple compilation methods, including West commands, direct CMake calls, and Make commands, providing developers with flexible build options.

## Core Components

- **CMake**: The main build system responsible for project configuration, dependency management, and build process definition
- **Ninja**: A high-performance build tool that executes actual compilation tasks as CMake's backend
- **West**: Zephyr project management tool extended in UniSDK to manage projects and simplify build workflows
- **Kconfig**: Configuration management system for handling compile-time configuration options
- **Make**: Provides a traditional make command interface as a frontend to CMake
- **BDT**: Telink Burning and Debugging Tool for flash operations on Telink devices, integrated via `west tl-bdt` command

## Directory Structure

Build system-related files are mainly distributed in the following directories:

```
unisdk/
├── CMakeLists.txt         # Main CMake configuration file
├── Makefile               # Make command interface
├── west.yml               # West workspace configuration
├── Kconfig.chip           # Chip-related Kconfig configuration entry
├── Kconfig.build          # Build-related Kconfig configuration entry
├── cmake/                 # CMake modules and configurations
├── scripts/               # Build-related scripts
├── core/                  # Core code
├── soc/                   # Chip support code
├── boards/                # Development board support
└── docs/                  # Documentation
```

## Getting Started

### Install Dependencies

#### Ubuntu
```bash
sudo apt install --no-install-recommends git cmake ninja-build python3-dev python3-venv python3-tk make
```

#### macOS
```bash
brew install cmake ninja python3 python-tk
```

#### Windows

**Using winget:**
```powershell
winget install Kitware.CMake Ninja-build.Ninja Python Git.Git
```

**Manual installation:**
- [CMake](https://cmake.org/download/) (Make sure to check "Add CMake to the PATH environment variable")
- [Ninja](https://github.com/ninja-build/ninja/releases) (Manually add to PATH environment variable)
- [Python](https://www.python.org/downloads/) (Make sure to check "Add Python.exe to PATH")
- [Git](https://git-scm.com/downloads)

### Python Virtual Environment

```bash
# Create virtual environment
python -m venv .venv  # Windows
python3 -m venv .venv  # Linux/MacOS

# Activate virtual environment
source .venv/bin/activate  # Linux/Mac
.venv\Scripts\activate.bat     # Windows Batchfile
.venv\Scripts\activate.ps1     # Windows PowerShell

# Install dependencies
pip install -r requirements.txt
```

**Windows PowerShell script execution permission:**
If you encounter a script execution error when activating the virtual environment, run the following command:
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### West Tool Setup

```bash
# Verify west installation
west --version

# Initialize west workspace
west init -l
```

**Potential issue:** If you encounter the error `already initialized in {where}, aborting.` when running `west init -l`, this may be because the `ZEPHYR_BASE` environment variable points to a Zephyr workspace with an existing `.west` directory.

**Solution:** Temporarily unset the `ZEPHYR_BASE` environment variable before running `west init -l`:

1. **Windows (PowerShell):**
   ```powershell
   Remove-Item Env:ZEPHYR_BASE; west init -l
   ```

2. **Linux/macOS:**
   ```bash
   unset ZEPHYR_BASE; west init -l
   ```

### Toolchain Configuration

Ensure you have downloaded the toolchain and extracted it to a directory, then set the `TELINK_TOOLCHAIN_PATH` environment variable:

```bash
# Windows
set TELINK_TOOLCHAIN_PATH=path/to/toolchain  # Command Prompt
$env:TELINK_TOOLCHAIN_PATH=path/to/toolchain  # PowerShell

# Linux/macOS
export TELINK_TOOLCHAIN_PATH=path/to/toolchain
```

### BDT Tool Configuration

For flash operations using `west tl-bdt` commands, you may need to configure the BDT tool path:

```bash
# Windows
set BDT_PATH=path/to/BDT  # Command Prompt
$env:BDT_PATH=path/to/BDT  # PowerShell

# Linux/macOS
export BDT_PATH=path/to/BDT
```

The BDT tool path can also be specified per-command using the `--bdt-path` option:

```bash
west tl-bdt download --chip B92 --bdt-path path/to/BDT -i build/Telink.bin
```

## Build Process

### Build Commands

#### Basic Build Command
```bash
west tl-build
```

#### Common Build Examples

**Specify build directory:**
```bash
west tl-build -d build_tl3218
```

**Specify source directory:**
```bash
west tl-build path/to/application
```

**Configure and build:**
```bash
west tl-build -k
```

**Clean build directory:**
```bash
# Clean build directory and rebuild
west tl-build --clean

# Use a pristine build directory
west tl-build --pristine=always
```

### Configuration Commands

#### Basic Configuration Command
```bash
west tl-config
```

#### Common Configuration Examples

**Specify build directory:**
```bash
west tl-config -d build_custom
# Or specify both source and build directories
west tl-config path/to/application -d build_custom
```

**Use current directory:**
```bash
west tl-config
```

**Use different source directory:**
```bash
west tl-config path/to/application
```
### BDT Commands

The `west tl-bdt` command provides flash operations for Telink devices:

#### Download Firmware
```bash
# Basic download
west tl-bdt download

# Download with specific chip and firmware path
west tl-bdt download --chip B92 -i build/Telink.bin
```

#### Flash Operations
```bash
# Read flash
west tl-bdt read --chip B92 -a 0x00 -s 16

# Write flash
west tl-bdt write --chip B92 -a 0x1000 -i data.bin

# Erase flash
west tl-bdt erase --chip B92 -a 0x00 -s 4k

# Lock flash region
west tl-bdt lock --chip B92 -a 0x00 -s 512k

# Unlock flash
west tl-bdt unlock --chip B92

# Reset device
west tl-bdt reset --chip B92
```

## Building Without West

### Using Make Commands

UniSDK provides a Makefile interface as a frontend to CMake, simplifying build commands.

#### Makefile Targets

```bash
# Run CMake configuration
make cmake

# Configure project
make config

# Run pinmux configuration tool
make pinmux

# Build project
make build

# Clean build directory
make clean

# Clean and rebuild
make cleanbuild
```

#### Makefile Variables

You can control the behavior of the Makefile through environment variables or command-line parameters:

```bash
# Specify application to build
make cmake build APP=samples/gpio_demo

# Specify SOC and BOARD
make cmake build SOC=TL3218X BOARD=TL3218X_EVK

# Specify application, SOC, and BOARD simultaneously
make cmake build APP=samples/gpio_demo SOC=TL3218X BOARD=TL3218X_EVK
```

**Available variables:**
- `APP`: Specify application path to build (default: samples/gpio_demo)
- `SOC`: Specify target SOC
- `BOARD`: Specify target development board
- `BUILD_DIR`: Specify build directory (default: build)

### CMake Configuration Process
```bash
cmake -B path/to/build -G Ninja -DTelink_DIR='path/to/telink-package' -S path/to/source
```

**Parameter description:**
- `-B path/to/build`: Specify build directory location
- `-G Ninja`: Specify Ninja as the generator
- `-DTelink_DIR='path/to/telink-package'`: Set Telink_DIR variable pointing to telink-package directory
- `-S path/to/source`: Specify source code directory location

### CMake Build Process
```bash
# Build project
cmake --build path/to/build

# Or use Ninja directly
ninja -C path/to/build
```

**Clean build directory:**
```bash
# Clean build directory
cmake --build path/to/build --target clean
```
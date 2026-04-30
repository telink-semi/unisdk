# Build Flow Documentation

## Overview
This document details the complete build process flow from configuration to firmware generation.

## Getting Started Guide

### Install dependencies

You’ll install some host dependencies using your package manager.

#### Ubuntu
1. Use apt to install the required dependencies:
```
  sudo apt install --no-install-recommends git cmake ninja-build python3-dev python3-venv python3-tk make
```
2. Verify the versions of the main dependencies installed on your system by entering:
```
cmake --version
python3 --version
git --version
ninja --version
```
If the error message "command not found" is displayed, it means that the corresponding tool is not installed. You need to install it first.

#### MacOS
1. Use Homebrew to install the required dependencies:
```
  brew install cmake ninja python3 python-tk
```
2. Verify the versions of the main dependencies installed on your system by entering:
```
cmake --version
python3 --version
git --version
ninja --version
```
If the error message "command not found" is displayed, it means that the corresponding tool is not installed. You need to install it first.

#### Windows
1. You can install using the following two methods:
1.1 For systems that support winget, install using winget:
```
winget install Kitware.CMake Ninja-build.Ninja Python Git.Git
```
If your system doesn't support winget but you still want to install using it, you can refer to the following method to install winget: [winget](https://github.com/microsoft/winget-cli/releases)，download the latest version of 'Microsoft.DesktopAppInstaller_8wekyb3d8bbwe.msixbundle' and install it.


1.2 For systems that don't support winget, download and install from official websites:
- [CMake](https://cmake.org/download/) (Make sure to check Add CMake to the PATH environment variable)
- [Ninja](https://github.com/ninja-build/ninja/releases) (You need to manually add it to the PATH environment variable)
- [Python](https://www.python.org/downloads/) (Make sure to check Add Python.exe to PATH)
- [Git](https://git-scm.com/downloads)

2. After installation, verify that the installation was successful:
```
cmake --version
python --version
git --version
ninja --version
```
If the error message "command not found" is displayed, it means that the corresponding tool is not installed or not added to the PATH environment variable. You need to install it or add it to the PATH environment variable first.

### install Python dependencies
Before starting the build process, You’ll also install additional Python dependencies in a Python virtual environment.

#### Python Virtual Environment
```bash
# Create and activate virtual environment
python -m venv .venv # Windows
python3 -m venv .venv # Linux/MacOS
```
```
# Activate virtual environment
source .venv/bin/activate  # Linux/Mac
.venv\Scripts\activate.bat     # Windows Batchfile
.venv\Scripts\activate.ps1     # Windows PowerShell
```
When using `.venv\Scripts\activate.ps1` to activate the virtual environment in Windows PowerShell, you might encounter the following error:
```
Activate.ps1 cannot be loaded because running scripts is disabled on this system.
```
This is because Windows PowerShell doesn't allow script execution by default. You can resolve this issue by following command:

```
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```
Then, when you have activated the virtual environment, you can install the required Python dependencies using the following command:
```
# Install dependencies
pip install -r requirements.txt
```

#### West Tool Setup
```bash
# Verify west installation
west --version

# Initialize west workspace
west init -l
```

**Potential Issue with west init -l:**
If you encounter the error `already initialized in {where}, aborting.` when running `west init -l`, this is likely due to the `ZEPHYR_BASE` environment variable being set to a Zephyr workspace with an existing `.west` directory. West incorrectly detects this as the current workspace.

**Solution:**
Temporarily unset the `ZEPHYR_BASE` environment variable before running `west init -l`:

1. **Windows (PowerShell):**
   ```powershell
   Remove-Item Env:ZEPHYR_BASE; west init -l
   ```

2. **Linux/macOS:**
   ```bash
   unset ZEPHYR_BASE; west init -l
   ```

After running these commands, the UniSDK workspace will be properly initialized.

#### Toolchain Configuration

Before building, ensure you have downloaded the toolchain and extracted it to a directory.
if you don't have the toolchain, you can download it from the following link (Only available for Telink):
[Toolchain_V5.4.1](https://drive.weixin.qq.com/s?k=AKwA0AfNAA8Ox1r0ab)

- Set `TELINK_TOOLCHAIN_PATH` environment variable
- Ensure toolchain binaries are accessible in PATH

## Build Process Stages

### 1. Environment Validation Phase
- **Purpose**: Verify all required tools and dependencies are available
- **Checks**:
  - Python virtual environment activation
  - West tool availability and version
  - Toolchain path configuration
  - Required Python packages installation

### 2. Build Phase

The build phase transforms application source code and configuration into executable binary files. Unisdk uses CMake and Ninja as its build system, providing flexible and powerful build capabilities.

### 2.1 Configuration Commands

Before building, you may need to configure the application settings. The `tl-config` command provides a unified interface for Telink application configuration.

#### Basic Configuration Command

The simplest configuration command is:

```bash
west tl-config
```
This command configures the application in the current directory using default settings.

- **Tool**: `scripts/west_commands/config.py`
- **Process**:
  1. Sets up environment variables (TELINK_BASE)
  2. Creates build directory if needed
  3. Runs initial CMake configuration if required
  4. Executes the unified `config` target which:
     - Generates `chip.config` from `Kconfig.chip`
     - Generates `capabilities.h` from chip configuration
     - Generates `build.config` from `Kconfig.build`
     - Merges configurations into final `.config`
     - Generates pinmux configuration files

#### Common Configuration Command Examples

##### Specifying Build Directory
```bash
west tl-config -d build_custom
# Or specifying source directory and build directory
west tl-config path/to/application -d build_custom
```

##### Using Current Directory
```bash
west tl-config
```

##### Using Different Source Directory
```bash
west tl-config path/to/application
```

#### Configuration Integration with Build

The configuration functionality is also integrated into the build command through the `-k/--kconfig` option:

```bash
# Configure and build in one command
west tl-build -k
```

This option uses the same underlying configuration logic as `west tl-config`, ensuring consistent behavior between standalone configuration and build-time configuration.

### 2.2 Basic Build Commands

The simplest build command is as follows:

```bash
west tl-build
```
This command builds the application in the current directory. If it's the first build, it will automatically configure and build.

- **Tool**: `scripts/west_commands/build.py`
- **Process**:
  1. Checks for existing `.config`, `build.config`, `chip.config`, `pinmux.h`, and `capabilities.h` files
  2. Invokes CMake to generate build system
  3. Compiles source code using selected toolchain
  4. Generates firmware binaries in `build/` directory(`build/` is the default build directory)

### 2.3 Common Build Command Examples

#### Specifying Build Directory
```bash
west tl-build -d build_tl3218
```

#### Specifying Source Directory
```bash
west tl-build path/to/application
```

#### Running CMake After Configuration
```bash
west tl-build -c
```

#### Building and Flashing Firmware(Todo)
```bash
west tl-build --flash
```

### 2.4 Cleaning Build Directory

You can clean the build directory using the `--clean` parameter or `--pristine` parameter:

```bash
# Clean the build directory and rebuild
west tl-build --clean

# Control whether to use a pristine build directory
west tl-build --pristine=always
```

## 3. Configuration File Generation Logic and Dependencies

### 3.1 Configuration File Hierarchy

Unisdk uses a hierarchical configuration mechanism to manage various configuration options. Configuration files are loaded in the following order of priority from lowest to highest:

1. **chip.config**: Contains default chip-related configurations provided by the chip vendor
2. **build.config**: Contains build-specific configuration options that can be customized by users
3. **.config**: The final merged configuration file, which can be automatically generated through command-line parameters or edited through menuconfig

### 3.2 Configuration File Generation Flow

#### Configuration via `west tl-config` Command

When executing the `west tl-config` command, the configuration generation flow is as follows:

1. **Parameter Processing Phase**:
   - Parses the `source_dir` positional argument (defaults to current directory if not provided)
   - Parses the `-d/--build-dir` option (defaults to `source_dir/build` if not provided)

2. **CMake Configuration Phase** (if needed):
   - Checks for existing `CMakeCache.txt` to determine if initial CMake configuration is required
   - Runs CMake configuration with Ninja generator and Telink package path
   - Uses the specified source and build directories

3. **Configuration Target Execution**:
   - Executes `cmake --build <build_dir> --target config`
   - This triggers the complete configuration sequence described in section 3.3

#### Configuration via `west tl-build` Command

When executing the `west tl-build` command (especially with `-k/--kconfig` option), the configuration generation flow is as follows:

1. **Parameter Processing Phase**:
   - Checks for `--board` or `--soc` in command-line parameters
   - If parameters are specified, the `_update_kconfig()` method creates or updates the `.config` file
   - If no parameters are specified, the system checks if a `.config` file already exists

2. **Default Configuration Generation Phase** (when no parameters are specified and no `.config` file exists):
   - The `kconfig.cmake` script is called
   - The system runs `scripts/kconfig.py` to generate `chip.config` and `build.config`
   - These configuration files are merged into the final `.config` file
   - For pinmux configuration, if `.pinmux` file is missing, it will first generate the default `.pinmux` file, then generate `pinmux.h` file from the `.pinmux` file.

3. **CMake Configuration Phase**:
   - The `_configure_cmake()` method passes the `BOARD` and `SOC` parameters to CMake
   - The CMake build system configures the project using these parameters and configuration files

Both approaches ultimately use the same unified `config` target, ensuring consistent configuration generation across different command interfaces.

### 3.3 Configuration Target

A new unified `config` target has been created to replace multiple individual custom commands:

```bash
cmake --build build --target config
```

This target executes the entire configuration flow in sequence:
1. Clean existing configuration files
2. Generate `chip.config`
3. Generate `capabilities.h`
4. Generate `build.config`
5. Merge into `.config`
6. Generate pinmux configuration

This ensures a consistent and controlled configuration process.

### 3.4 Configuration File Dependencies

- The `.config` file depends on `chip.config` and `build.config` (when automatically generated)
- The CMake build system depends on the `.config` file to determine compilation options
- CMake monitors changes to the `.config` file and reconfigures the project when the file is modified

Through this dependency management, the system ensures that configuration changes are correctly propagated to the build process, allowing developers to flexibly adjust and customize build options.

## 4. Troubleshooting

When using the Unisdk build system, you may encounter the following common issues. Here are the corresponding solutions:

### 4.1 Environment-Related Issues

#### Problem: Error indicating that TELINK_TOOLCHAIN_PATH environment variable cannot be found

**Solutions:**
```bash
# Windows
set TELINK_TOOLCHAIN_PATH=path/to/toolchain # Windows Command Prompt
$env:TELINK_TOOLCHAIN_PATH=path/to/toolchain # PowerShell
# Linux/macOS
export TELINK_TOOLCHAIN_PATH=path/to/toolchain
```

#### Problem: CMake configuration fails

**Solutions:**
- Ensure all necessary dependencies are installed
- Check if the CMake version is compatible
- Try again after clearing the old build cache with the `--clean` parameter

### 4.2 Build-Related Issues

#### Problem: Compilation errors occur during the build process

**Solutions:**
- Check if the application code has syntax errors
- Ensure correct board and chip configurations are used
- Try a clean build using the `--pristine=always` parameter

### 4.3 menuconfig Issues

#### Problem: menuconfig arrow icons not working properly when using powershell in VSCode

**Solutions:**
- Roll back to an earlier version of VSCode; or use the test version of ConPty library provided by VSCode
- Use Windows 11 system
- You can use letter keys instead of arrow icons for operation
- You can use command prompt instead of PowerShell to execute related operations

### 4.4 Environment Variable Issues

#### Problem: Environment variables not taking effect in terminal after setting

**Solutions:**
- Restart the terminal, VS Code or the system
- If the previous step doesn't solve the problem, manually check if the PATH contains the correct path and add it to the user's PATH

### 4.5 Other Issues

If you encounter other issues, try the following general solutions:

1. Ensure you're using the latest version of Unisdk
2. Clear the build directory and rebuild:
   ```bash
   west tl-build --clean
   ```
3. Check system logs for more detailed error information

## 5. Building Without West

This chapter explains how to build Unisdk projects without using the west tool, providing an alternative approach for environments where west might not be available or preferred.

### 5.1 CMake Configuration Process

Without west, you'll need to manually create the cmake files:
```
cmake -B path/to/build -G Ninja -DTelink_DIR='path/to/telink-package' -S path/to/source
```

This command is a key step for manually configuring CMake projects without the west tool. Here's an explanation of each parameter:

- **`-B path/to/build`**: Specifies the location of the build directory. This creates or uses the specified directory to store all build output files, keeping the source directory clean.

- **`-G Ninja`**: Specifies Ninja as the generator, which is a high-performance build system.

- **`-DTelink_DIR='path/to/telink-package'`**: Sets the Telink_DIR variable pointing to the telink-package directory, telling CMake where to find the TelinkConfig.cmake file, which contains all configuration and module information for the SDK. If have set TELINK_BASE environment variable, this parameter is not needed.

- **`-S path/to/source`**: Specifies the location of the source code directory, typically pointing to the project root directory or example project directory that contains the main CMakeLists.txt.

### 5.2 CMake Build Process

Once CMake is configured, build the project:

```bash
# Build the project
cmake --build path/to/build

# Or use Ninja directly if available
ninja -C path/to/build
```

To clean the build directory manually:

```bash
# Clean the build directory
cmake --build path/to/build --target clean

# Or completely remove and recreate the build directory
```


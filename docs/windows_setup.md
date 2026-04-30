# Unisdk Windows Automated Deployment Script User Guide

## Introduction

This script is designed to simplify the environment configuration process for Unisdk on Windows systems. By running a single script, it automatically completes the following tasks:

- Check and set PowerShell execution policy
- Automatically install or update required tools (CMake, Ninja, Python, Git)
- Create Python virtual environment
- Install all Python dependencies
- Set Telink toolchain environment variables
- Initialize west workspace
- Provide detailed configuration logs

## System Requirements

- Windows 10 or Windows 11 operating system
- User account with administrator privileges (recommended)
- Stable network connection (for downloading tools and dependencies)

## Usage

### Method 1: Run PowerShell Script Directly (Recommended)

1. Ensure Git is installed and you have cloned the Unisdk repository
2. Open PowerShell as administrator (important)
3. Navigate to the script directory:
   ```powershell
   cd <path_to_unisdk_repository>\scripts
   ```
   Replace `<path_to_unisdk_repository>` with the actual path where you cloned the Unisdk repository
4. Run the script:
   ```powershell
   .\windows_setup.ps1
   ```

### Method 2: Run via File Explorer

1. Locate the `windows_setup.ps1` file
2. Right-click on the file and select "Run as administrator"

## Script Features in Detail

### 1. PowerShell Execution Policy Configuration

The script automatically checks and sets the PowerShell execution policy to `RemoteSigned` to allow running local scripts.

### 2. Tool Installation Methods

The script first detects if `winget` (Windows Package Manager) is installed:

- **If winget is available**: Automatically downloads and installs all required tools
- **If winget is not available**: Provides official download links to guide users through manual installation

### 3. Environment Variable Settings

The script automatically sets the following environment variables:

- `TELINK_TOOLCHAIN_PATH`: Points to the toolchain directory
- Adds CMake and Ninja paths to the system PATH

### 4. Python Environment Configuration

- Creates `.venv` virtual environment in the Unisdk root directory
- Upgrades pip to the latest version
- Installs all Python dependencies listed in `requirements.txt`
- Ensures west tool is properly installed

### 5. Telink Toolchain Setup

The script creates the toolchain directory and sets environment variables, but due to special permissions required for toolchain download, users need to manually download and extract:

- Creates `toolchain` directory
- Sets `TELINK_TOOLCHAIN_PATH` environment variable
- Provides download link and subsequent operation guidelines

### 6. West Workspace Initialization

Initializes the west workspace in the Unisdk root directory by executing the `west init -l` command.

## Logging

The script automatically generates a `setup_log.txt` file in the script directory, recording detailed logs of the entire configuration process for troubleshooting purposes.

## Notes

1. **Administrator Privileges**: While not strictly necessary, it's recommended to run the script as administrator to ensure proper environment variable settings

2. **Manual Toolchain Download**: Telink toolchain needs to be manually downloaded and extracted to the `<path_to_unisdk_repository>\toolchain` directory
   Replace `<path_to_unisdk_repository>` with the actual path where you cloned the Unisdk repository
   - Download link: https://drive.weixin.qq.com/s?k=AKwA0AfNAA8Ox1r0ab

3. **Environment Variables Effectiveness**: After script completion, it's recommended to open a new PowerShell window to apply all environment variable changes

4. **Virtual Environment Activation**: Use the following command to activate the virtual environment:
   ```powershell
   <path_to_unisdk_repository>\.venv\Scripts\Activate.ps1
   ```
   Replace `<path_to_unisdk_repository>` with the actual path where you cloned the Unisdk repository

5. **First Run Issues**: If you encounter the error "Cannot load file ... because running scripts is disabled on this system", manually set the execution policy:
   ```powershell
   Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser -Force
   ```

## Troubleshooting

### Common Issues and Solutions

1. **winget Installation Failure**
   - Check Windows version (requires Windows 10 1709 or higher)
   - Try installing App Installer from Microsoft Store

2. **Python Dependencies Installation Failure**
   - Ensure network connection is working
   - Proxy server settings might be needed, which can be modified in the script

3. **Environment Variables Not Taking Effect**
   - Close and reopen PowerShell window
   - Check system environment variable settings for correctness

4. **CMake or Ninja Not Found**
   - Manually check installation paths
   - The script will attempt to automatically find common installation locations, but may require manual addition to PATH

5. **Environment variables not taking effect in terminal after setting**
   - 1. Restart the terminal, VS Code or the system
   - 2. If the previous step doesn't solve the problem, manually check if the PATH contains the correct path and add it to the user's PATH

### Getting Help

If you encounter issues that cannot be resolved by the script, please check the `setup_log.txt` log file or contact Unisdk technical support.

## Next Steps

After script completion, you can start development with Unisdk:

1. Activate the Python virtual environment
2. Ensure Telink toolchain is properly installed and configured
3. Use `west tl-build` command to build projects

## Changelog

### v1.0
- Initial release
- Support for automatic installation of CMake, Ninja, Python, and Git
- Creation of Python virtual environment and dependency installation
- Setting of Telink toolchain environment variables
- West workspace initialization

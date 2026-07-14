# Unisdk Windows Automated Deployment Script
# This script will automatically configure all necessary environments and tools for Windows systems

# Display title
Write-Host "=========================================" -ForegroundColor Cyan
Write-Host "      Unisdk Windows Automated Script      " -ForegroundColor Cyan
Write-Host "=========================================" -ForegroundColor Cyan

# Set error handling
$ErrorActionPreference = "Stop"

# Create log file
$logFile = "$PSScriptRoot\setup_log.txt"
Start-Transcript -Path $logFile -Append

# Configuration options
$SkipToolUpdates = $true  # Set to $true to skip updating already installed tools to the latest version

# Function: Check and set PowerShell execution policy
function Set-ExecutionPolicyIfNeeded {
    Write-Host "Checking PowerShell execution policy..." -ForegroundColor Yellow
    $currentPolicy = Get-ExecutionPolicy -Scope CurrentUser

    if ($currentPolicy -ne "RemoteSigned" -and $currentPolicy -ne "Unrestricted") {
        Write-Host "Need to modify PowerShell execution policy to allow script execution" -ForegroundColor Yellow
        try {
            Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser -Force
            Write-Host "PowerShell execution policy has been set to RemoteSigned" -ForegroundColor Green
        } catch {
            Write-Host "Failed to set PowerShell execution policy, please run the script as administrator" -ForegroundColor Red
            throw $_
        }
    } else {
        Write-Host "PowerShell execution policy is already correctly configured: $currentPolicy" -ForegroundColor Green
    }
}

# Function: Check if winget is available
function Test-WingetAvailable {
    try {
        winget --version | Out-Null
        # Initialize winget and accept source agreements
        winget source update --accept-source-agreements 2>$null
        return $true
    } catch {
        return $false
    }
}

# Function: Install or update application (using winget)
function Install-WithWinget {
    param (
        [string]$packageId,
        [string]$appName,
        [bool]$skipUpdate = $false  # New parameter to control whether to skip updates
    )

    Write-Host "Checking $appName..." -ForegroundColor Yellow

    # First check if the tool is already available in PATH
    if (Test-ToolInstalled -toolName $appName.ToLower()) {
        Write-Host "$appName is already installed and available in PATH. Skipping installation." -ForegroundColor Green
        return $true
    }

    try {
        # Use packageId for more reliable checking
        $installed = winget list --id $packageId --exact 2>$null

        # Check if the package is installed by looking for the package ID in the output
        if ($installed -match $packageId) {
            if ($skipUpdate) {
                Write-Host "$appName is already installed. Skipping update as requested." -ForegroundColor Yellow
            } else {
                Write-Host "$appName is already installed, checking for updates..." -ForegroundColor Yellow
                winget upgrade --id $packageId --exact --silent --accept-source-agreements --accept-package-agreements 2>$null
                Write-Host "$appName has been updated to the latest version" -ForegroundColor Green
            }
        } else {
            Write-Host "Installing $appName..." -ForegroundColor Yellow
            winget install --id $packageId --exact --silent --accept-source-agreements --accept-package-agreements
            Write-Host "$appName installation completed" -ForegroundColor Green
        }
        return $true
    } catch {
        Write-Host "Failed to install $appName using winget: $_" -ForegroundColor Red
        return $false
    }
}

# Function: Download file
function Invoke-DownloadFile {
    param (
        [string]$url,
        [string]$outputPath
    )

    Write-Host "Downloading $outputPath..." -ForegroundColor Yellow

    try {
        Invoke-WebRequest -Uri $url -OutFile $outputPath -UseBasicParsing
        Write-Host "Download completed: $outputPath" -ForegroundColor Green
        return $true
    } catch {
        Write-Host "Download failed: $_" -ForegroundColor Red
        return $false
    }
}

# Function: Extract zip file
function Expand-ZipFile {
    param (
        [string]$zipPath,
        [string]$destinationPath
    )

    Write-Host "Extracting $zipPath to $destinationPath..." -ForegroundColor Yellow

    try {
        # Create destination directory
        if (-not (Test-Path $destinationPath)) {
            New-Item -ItemType Directory -Path $destinationPath -Force | Out-Null
        }

        # Use .NET for extraction
        Add-Type -AssemblyName System.IO.Compression.FileSystem
        [System.IO.Compression.ZipFile]::ExtractToDirectory($zipPath, $destinationPath)
        Write-Host "Extraction completed" -ForegroundColor Green
        return $true
    } catch {
        Write-Host "Extraction failed: $_" -ForegroundColor Red
        return $false
    }
}

# Function: Set environment variable
function Set-EnvironmentVariable {
    param (
        [string]$name,
        [string]$value,
        [string]$scope = "User"
    )

    Write-Host "Setting environment variable: $name = $value" -ForegroundColor Yellow

    try {
        [System.Environment]::SetEnvironmentVariable($name, $value, $scope)
        # Also set environment variable for current process to make it immediately effective for subsequent commands in the script
        [System.Environment]::SetEnvironmentVariable($name, $value, "Process")
        Write-Host "Environment variable has been set: $name = $value" -ForegroundColor Green
        return $true
    } catch {
        Write-Host "Failed to set environment variable: $_" -ForegroundColor Red
        return $false
    }
}

# Function: Add directory to PATH environment variable
function Add-ToPath {
    param (
        [string]$pathToAdd,
        [string]$scope = "User"
    )

    Write-Host "Adding $pathToAdd to PATH environment variable..." -ForegroundColor Yellow

    try {
        # Get current PATH
        $currentPath = [System.Environment]::GetEnvironmentVariable("PATH", $scope)

        # Check if path already exists
        if (-not $currentPath.Contains($pathToAdd)) {
            # Add path
            $newPath = "$currentPath;$pathToAdd"
            [System.Environment]::SetEnvironmentVariable("PATH", $newPath, $scope)
            [System.Environment]::SetEnvironmentVariable("PATH", $newPath, "Process")
            Write-Host "Successfully added $pathToAdd to PATH environment variable" -ForegroundColor Green
        } else {
            Write-Host "$pathToAdd is already in PATH environment variable" -ForegroundColor Green
        }
        return $true
    } catch {
        Write-Host "Failed to add to PATH environment variable: $_" -ForegroundColor Red
        return $false
    }
}

# Function: Check if tool is installed
# Function: Check common installation paths for a tool
function Find-ToolPath {
    param (
        [string]$toolName
    )

    Write-Host "Attempting to find $toolName installation path..." -ForegroundColor Yellow

    # Define common paths based on tool name - check both C and D drives
    $possiblePaths = @(
        "C:\Program Files\$toolName\bin",
        "C:\Program Files (x86)\$toolName\bin",
        "D:\Program Files\$toolName\bin",
        "D:\Program Files (x86)\$toolName\bin",
        "$env:USERPROFILE\AppData\Local\Microsoft\WinGet\Links",
        "$env:LOCALAPPDATA\Microsoft\WindowsApps"
    )

    # Add tool-specific paths
    if ($toolName -eq "cmake") {
        $possiblePaths += "C:\Program Files\CMake\bin",
                         "C:\Program Files (x86)\CMake\bin",
                         "D:\Program Files\CMake\bin",
                         "D:\Program Files (x86)\CMake\bin"
    } elseif ($toolName -eq "ninja") {
        $possiblePaths += "C:\Program Files\Ninja",
                         "C:\Program Files (x86)\Ninja",
                         "D:\Program Files\Ninja",
                         "D:\Program Files (x86)\Ninja"
    }

    # Remove duplicates
    $possiblePaths = $possiblePaths | Select-Object -Unique

    foreach ($path in $possiblePaths) {
        Write-Host "Checking path: $path" -ForegroundColor Yellow

        if (Test-Path $path) {
            # Look for executable files matching the tool name
            $executablePattern = "$toolName*.exe"
            $executables = Get-ChildItem -Path $path -Name $executablePattern -ErrorAction SilentlyContinue

            if ($executables.Count -gt 0) {
                Write-Host "Found $toolName executables: $executables" -ForegroundColor Yellow

                # Add to current process PATH
                $env:PATH += ";$path"
                Write-Host "Added $path to current process PATH" -ForegroundColor Green

                # Also add to user PATH for future sessions
                Add-ToPath -pathToAdd $path -scope "User"

                # Test again with the updated PATH
                try {
                    Write-Host "Testing $toolName again..." -ForegroundColor Yellow
                    $output = & $toolName --version 2>&1
                    Write-Host "$toolName is now available: $output" -ForegroundColor Green
                    return $true
                } catch {
                    Write-Host "Direct call still fails, trying with full path..." -ForegroundColor Yellow

                    # Try with full path to each executable
                    foreach ($exe in $executables) {
                        $fullPath = Join-Path $path $exe
                        try {
                            $output = & $fullPath --version 2>&1
                            Write-Host "$toolName is available at full path: $output" -ForegroundColor Green

                            # Set an alias for convenience
                            Set-Alias -Name $toolName -Value $fullPath -Scope Global
                            Write-Host "Created alias '$toolName' for $fullPath" -ForegroundColor Green
                            return $true
                        } catch {
                            Write-Host "Failed with $fullPath" -ForegroundColor Yellow
                            continue
                        }
                    }
                }
            } else {
                Write-Host "No $toolName executables found in $path" -ForegroundColor Yellow
            }
        } else {
            Write-Host "Path does not exist: $path" -ForegroundColor Yellow
        }
    }

    return $false
}

# Function: Test if a tool is installed and available in PATH
function Test-ToolInstalled {
    param (
        [string]$toolName
    )

    try {
        # Try running tool version command
        $output = & $toolName --version 2>&1
        Write-Host "$toolName is installed: $output" -ForegroundColor Green
        return $true
    } catch {
        # Try to find the tool in common installation paths
        return Find-ToolPath -toolName $toolName
    }
}

# Function: Create Python virtual environment
function New-PythonVirtualEnvironment {
    param (
        [string]$envPath
    )

    Write-Host "Creating Python virtual environment: $envPath" -ForegroundColor Yellow

    try {
        # Check if virtual environment already exists
        if (Test-Path $envPath) {
            Write-Host "Virtual environment already exists, skipping creation" -ForegroundColor Green
        } else {
            python -m venv $envPath
            Write-Host "Python virtual environment created successfully: $envPath" -ForegroundColor Green
        }
        return $true
    } catch {
        Write-Host "Failed to create Python virtual environment: $_" -ForegroundColor Red
        return $false
    }
}

# Function: Activate Python virtual environment and install dependencies
function Install-PythonDependencies {
    param (
        [string]$envPath,
        [string]$requirementsPath
    )

    Write-Host "Activating Python virtual environment and installing dependencies..." -ForegroundColor Yellow

    try {
        # Get activation script path
        $activateScript = Join-Path $envPath "Scripts\Activate.ps1"

        # Activate virtual environment
        . $activateScript

        # Upgrade pip
        Write-Host "Upgrading pip..." -ForegroundColor Yellow
        python -m pip install --upgrade pip

        # Install dependencies
        if (Test-Path $requirementsPath) {
            Write-Host "Installing dependencies: $requirementsPath" -ForegroundColor Yellow
            pip install -r $requirementsPath
            Write-Host "Python dependencies installation completed" -ForegroundColor Green
        } else {
            Write-Host "Warning: Could not find requirements.txt file: $requirementsPath" -ForegroundColor Yellow
        }

        # Verify west installation
        try {
            $westVersion = west --version 2>&1
            Write-Host "West is installed: $westVersion" -ForegroundColor Green
        } catch {
            Write-Host "West is not installed, installing..." -ForegroundColor Yellow
            pip install west
        }

        return $true
    } catch {
        Write-Host "Failed to install Python dependencies: $_" -ForegroundColor Red
        return $false
    }
}

# Function: Initialize west workspace
function Initialize-WestWorkspace {
    param (
        [string]$workspacePath
    )

    Write-Host "Initializing west workspace: $workspacePath" -ForegroundColor Yellow

    try {
        # Switch to workspace directory
        Set-Location $workspacePath

        # Initialize west workspace
        west init -l
        Write-Host "West workspace initialization completed" -ForegroundColor Green
        return $true
    } catch {
        Write-Host "Failed to initialize west workspace: $_" -ForegroundColor Red
        Write-Host "Note: This may need to be done manually, or the current directory may not be the correct west workspace" -ForegroundColor Yellow
        return $false
    }
}

# Function: Download and set up Telink toolchain
function Set-TelinkToolchain {
    param (
        [string]$toolchainDir
    )

    Write-Host "Setting up Telink toolchain..." -ForegroundColor Yellow

    # Create toolchain directory
    if (-not (Test-Path $toolchainDir)) {
        New-Item -ItemType Directory -Path $toolchainDir -Force | Out-Null
    }

    # Note: Due to potential permission requirements for the toolchain download link, we only create the directory and set environment variables here
    Write-Host "Toolchain directory created: $toolchainDir" -ForegroundColor Yellow
    Write-Host "Please manually download the toolchain and extract it to $toolchainDir" -ForegroundColor Yellow
    Write-Host "Toolchain download link: https://drive.weixin.qq.com/s?k=AKwA0AfNAA8Ox1r0ab" -ForegroundColor Cyan

    # Set TELINK_TOOLCHAIN_PATH environment variable
    Set-EnvironmentVariable -name "TELINK_TOOLCHAIN_PATH" -value $toolchainDir -scope "User"

    # Add toolchain bin directory to PATH
    $toolchainBinPath = Join-Path $toolchainDir "bin"
    if (Test-Path $toolchainBinPath) {
        Add-ToPath -pathToAdd $toolchainBinPath -scope "User"
    } else {
        Write-Host "Warning: Toolchain bin directory does not exist, it will be automatically added to PATH after extracting the toolchain" -ForegroundColor Yellow
    }

    return $true
}

# Main function
function Main {
    try {
        # Get the directory where the script is located
        $unisdkRoot = Split-Path -Parent $PSScriptRoot
        Write-Host "Unisdk root directory: $unisdkRoot" -ForegroundColor Blue

        # Set PowerShell execution policy
        Set-ExecutionPolicyIfNeeded

        # Call set_telink_base.ps1 to set TELINK_BASE environment variable
        Set-EnvironmentVariable -name "TELINK_BASE" -value $unisdkRoot -scope "User"

        # Check if winget is available
        $wingetAvailable = Test-WingetAvailable

        if ($wingetAvailable) {
            Write-Host "winget is available, using winget to install tools" -ForegroundColor Green

            # Install necessary tools
            Install-WithWinget -packageId "Kitware.CMake" -appName "CMake" -skipUpdate $SkipToolUpdates
            Install-WithWinget -packageId "Ninja-build.Ninja" -appName "Ninja" -skipUpdate $SkipToolUpdates
            Install-WithWinget -packageId "Python.Python.3.12" -appName "Python" -skipUpdate $SkipToolUpdates
            Install-WithWinget -packageId "Git.Git" -appName "Git" -skipUpdate $SkipToolUpdates
        } else {
            Write-Host "Warning: winget is not available, will prompt for manual tool installation" -ForegroundColor Yellow
            Write-Host "Please download and install winget from the following link:" -ForegroundColor Yellow
            Write-Host "https://github.com/microsoft/winget-cli/releases" -ForegroundColor Cyan
            Write-Host "or download and install required tools from the following links:" -ForegroundColor Yellow
            Write-Host "1. CMake: https://cmake.org/download/" -ForegroundColor Cyan
            Write-Host "2. Ninja: https://github.com/ninja-build/ninja/releases" -ForegroundColor Cyan
            Write-Host "3. Python: https://www.python.org/downloads/" -ForegroundColor Cyan
            Write-Host "4. Git: https://git-scm.com/downloads" -ForegroundColor Cyan

            Read-Host "Press Enter to continue after installation..."
        }

        # Verify tool installation success
        $cmakeInstalled = Test-ToolInstalled -toolName "cmake"
        $ninjaInstalled = Test-ToolInstalled -toolName "ninja"
        $pythonInstalled = Test-ToolInstalled -toolName "python"
        $gitInstalled = Test-ToolInstalled -toolName "git"

        # Create Python virtual environment
        $venvPath = Join-Path $unisdkRoot ".venv"
        New-PythonVirtualEnvironment -envPath $venvPath

        # Install Python dependencies
        $requirementsPath = Join-Path $unisdkRoot "requirements.txt"
        Install-PythonDependencies -envPath $venvPath -requirementsPath $requirementsPath

        # Set up Telink toolchain
        $toolchainDir = Join-Path $unisdkRoot "toolchain"
        Set-TelinkToolchain -toolchainDir $toolchainDir

        # Initialize west workspace
        Initialize-WestWorkspace -workspacePath $unisdkRoot

        # Display completion message
        Write-Host "=========================================" -ForegroundColor Green
        Write-Host "      Unisdk Environment Setup Complete!  " -ForegroundColor Green
        Write-Host "=========================================" -ForegroundColor Green
        Write-Host "The following configurations have been completed:"
        Write-Host "- CMake: $cmakeInstalled" -ForegroundColor $(if($cmakeInstalled){"Green"}else{"Red"})
        Write-Host "- Ninja: $ninjaInstalled" -ForegroundColor $(if($ninjaInstalled){"Green"}else{"Red"})
        Write-Host "- Python: $pythonInstalled" -ForegroundColor $(if($pythonInstalled){"Green"}else{"Red"})
        Write-Host "- Git: $gitInstalled" -ForegroundColor $(if($gitInstalled){"Green"}else{"Red"})
        Write-Host "- Python virtual environment: $venvPath"
        Write-Host "- Telink toolchain path: $toolchainDir (please download and extract manually)"
        Write-Host "- West workspace initialized"
        Write-Host ""
        Write-Host "Next steps:"
        Write-Host "1. Download and extract Telink toolchain to $toolchainDir" -ForegroundColor Yellow
        Write-Host "2. Open a new PowerShell window to apply environment variable changes" -ForegroundColor Yellow
        Write-Host "3. Activate virtual environment using: $venvPath\Scripts\Activate.ps1" -ForegroundColor Yellow
        Write-Host "4. Run west tl-build to start building projects" -ForegroundColor Yellow

        return $true
    } catch {
        Write-Host "Error during configuration: $_" -ForegroundColor Red
        return $false
    } finally {
        Stop-Transcript
        Write-Host "Log has been saved to: $logFile" -ForegroundColor Cyan
    }
}

# Run main function
Main

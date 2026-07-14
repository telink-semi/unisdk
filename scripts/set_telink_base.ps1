# This script sets the TELINK_BASE environment variable permanently
# Run in Windows PowerShell

# Get the directory where the script is located
$unisdkRoot = Split-Path -Parent $PSScriptRoot
Write-Host "Unisdk root directory: $unisdkRoot" -ForegroundColor Blue

# Set TELINK_BASE environment variable
Write-Host "Setting TELINK_BASE environment variable..." -ForegroundColor Yellow

try {
    [System.Environment]::SetEnvironmentVariable('TELINK_BASE', $unisdkRoot, 'User')
    # Also set environment variable for current process to make it immediately effective
    [System.Environment]::SetEnvironmentVariable('TELINK_BASE', $unisdkRoot, 'Process')
    Write-Host "TELINK_BASE environment variable has been permanently set: $unisdkRoot" -ForegroundColor Green
    Write-Host "The environment variable is immediately effective in this PowerShell session." -ForegroundColor Cyan
} catch {
    Write-Host "Error: Failed to set TELINK_BASE environment variable." -ForegroundColor Red
    Write-Host "Error details: $_" -ForegroundColor Red
    Write-Host "Please ensure you have sufficient permissions." -ForegroundColor Yellow
}

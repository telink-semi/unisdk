@echo off
REM This script sets the TELINK_BASE environment variable permanently
REM Run in Windows Command Prompt with administrator privileges

REM Get the parent directory of the script location as TELINK_BASE
set "SCRIPT_DIR=%~dp0"
set "TELINK_BASE=%SCRIPT_DIR%.."

REM Normalize path (remove trailing backslash)
for %%i in ("%TELINK_BASE%") do set "TELINK_BASE=%%~fi"

REM Set environment variable temporarily for current session
set "TELINK_BASE=%TELINK_BASE%"

REM Set environment variable permanently for the user
setx TELINK_BASE "%TELINK_BASE%"
if %ERRORLEVEL% equ 0 (
    echo TELINK_BASE environment variable has been permanently set to: %TELINK_BASE%
    echo You may need to restart your command prompt for the changes to take effect.
) else (
    echo Error: Failed to set TELINK_BASE environment variable permanently.
    echo Please try running this script with administrator privileges.
)

REM Keep the window open to view messages
pause

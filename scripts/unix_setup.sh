#!/usr/bin/env bash
# Unisdk Linux/macOS Automated Deployment Script
# Counterpart of windows_setup.ps1: installs Make, CMake, Ninja, Python 3 (with
# venv) and Git through the system package manager (Homebrew on macOS; apt, dnf,
# yum, pacman or zypper on Linux), sets TELINK_BASE, creates <sdk>/.venv,
# installs requirements.txt into it and initializes the west workspace.
#
# Usage: bash scripts/unix_setup.sh
# Tools that are already on PATH are left alone (same as windows_setup.ps1's
# $SkipToolUpdates = $true). Package installs may prompt for your sudo password.

set -u

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
UNISDK_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
VENV_PATH="$UNISDK_ROOT/.venv"
REQUIREMENTS_PATH="$UNISDK_ROOT/requirements.txt"
LOG_FILE="$SCRIPT_DIR/setup_log.txt"

# Mirror everything into the log file, like windows_setup.ps1's Start-Transcript
exec > >(tee -a "$LOG_FILE") 2>&1

if [ -t 1 ] || [ -n "${TERM:-}" ]; then
    C_CYAN=$'\033[36m'; C_YELLOW=$'\033[33m'; C_GREEN=$'\033[32m'; C_RED=$'\033[31m'; C_BLUE=$'\033[34m'; C_RESET=$'\033[0m'
else
    C_CYAN=""; C_YELLOW=""; C_GREEN=""; C_RED=""; C_BLUE=""; C_RESET=""
fi
info() { echo "${C_YELLOW}$*${C_RESET}"; }
ok()   { echo "${C_GREEN}$*${C_RESET}"; }
err()  { echo "${C_RED}$*${C_RESET}"; }

echo "${C_CYAN}=========================================${C_RESET}"
echo "${C_CYAN}   Unisdk Linux/macOS Automated Script   ${C_RESET}"
echo "${C_CYAN}=========================================${C_RESET}"
echo "${C_BLUE}Unisdk root directory: $UNISDK_ROOT${C_RESET}"

OS="$(uname -s)"

# Run a command as root (directly when already root, via sudo otherwise)
as_root() {
    if [ "$(id -u)" -eq 0 ]; then
        "$@"
    elif command -v sudo >/dev/null 2>&1; then
        sudo "$@"
    else
        err "Need root to run: $* (sudo is not available)"
        return 1
    fi
}

# Function: Set TELINK_BASE permanently in the user's shell startup files
set_telink_base() {
    export TELINK_BASE="$UNISDK_ROOT"
    info "Setting environment variable: TELINK_BASE = $UNISDK_ROOT"
    local line="export TELINK_BASE=\"$UNISDK_ROOT\""
    local files=()
    case "$(basename "${SHELL:-bash}")" in
        zsh)  files=("$HOME/.zshrc") ;;
        bash) if [ "$OS" = "Darwin" ]; then files=("$HOME/.bash_profile"); else files=("$HOME/.bashrc"); fi ;;
        *)    files=("$HOME/.profile") ;;
    esac
    for f in "${files[@]}"; do
        touch "$f"
        # Replace an older TELINK_BASE line (e.g. a moved SDK) instead of stacking them
        if grep -q '^export TELINK_BASE=' "$f"; then
            if grep -qxF "$line" "$f"; then
                ok "TELINK_BASE is already set in $f"
                continue
            fi
            local tmp
            tmp="$(mktemp)"
            grep -v '^export TELINK_BASE=' "$f" > "$tmp" && cat "$tmp" > "$f"
            rm -f "$tmp"
        fi
        echo "$line" >> "$f"
        ok "Added TELINK_BASE to $f"
    done
}

# Function: Make Homebrew available on macOS (installing it if needed)
ensure_homebrew() {
    if ! command -v brew >/dev/null 2>&1; then
        for b in /opt/homebrew/bin/brew /usr/local/bin/brew; do
            [ -x "$b" ] && eval "$("$b" shellenv)" && break
        done
    fi
    if command -v brew >/dev/null 2>&1; then
        return 0
    fi
    info "Homebrew is not installed, attempting to install it..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)" || return 1
    for b in /opt/homebrew/bin/brew /usr/local/bin/brew; do
        [ -x "$b" ] && eval "$("$b" shellenv)" && break
    done
    command -v brew >/dev/null 2>&1
}

# Function: Detect the Linux package manager
detect_package_manager() {
    for pm in apt-get dnf yum pacman zypper; do
        if command -v "$pm" >/dev/null 2>&1; then
            echo "$pm"
            return 0
        fi
    done
    echo ""
}

# Function: Package name for a tool under a given package manager
package_for() {
    local pm="$1" tool="$2"
    case "$pm:$tool" in
        brew:python)    echo "python@3.12" ;;
        brew:*)         echo "$tool" ;;
        apt-get:ninja)  echo "ninja-build" ;;
        apt-get:python) echo "python3 python3-venv python3-pip" ;;
        dnf:ninja|yum:ninja) echo "ninja-build" ;;
        dnf:python|yum:python) echo "python3 python3-pip" ;;
        pacman:python)  echo "python python-pip" ;;
        zypper:python)  echo "python3 python3-pip" ;;
        *)              echo "$tool" ;;
    esac
}

# Function: Pick the Python interpreter to build the venv with
find_python() {
    for p in python3.12 python3 python; do
        if command -v "$p" >/dev/null 2>&1 &&
           "$p" -c 'import sys; sys.exit(0 if sys.version_info >= (3, 8) else 1)' 2>/dev/null; then
            command -v "$p"
            return 0
        fi
    done
    return 1
}

# Python counts as installed only if it can also create a venv (Debian/Ubuntu
# ship python3 without ensurepip until python3-venv is installed)
python_ready() {
    local py
    py="$(find_python)" || return 1
    "$py" -c 'import venv, ensurepip' >/dev/null 2>&1
}

tool_installed() {
    case "$1" in
        python) python_ready ;;
        *)      command -v "$1" >/dev/null 2>&1 ;;
    esac
}

# Function: Install whatever of make/cmake/ninja/python/git is missing
install_tools() {
    local missing=()
    for tool in make cmake ninja python git; do
        if tool_installed "$tool"; then
            ok "$tool is already installed. Skipping installation."
        else
            missing+=("$tool")
        fi
    done
    [ "${#missing[@]}" -eq 0 ] && return 0

    local pm
    if [ "$OS" = "Darwin" ]; then
        if ! ensure_homebrew; then
            err "Failed to install Homebrew automatically (https://brew.sh)"
            return 1
        fi
        pm="brew"
        # macOS's make comes with the Xcode Command Line Tools, not Homebrew
        # (brew's "make" formula only installs it as gmake)
        local rest=()
        for tool in "${missing[@]}"; do
            if [ "$tool" = "make" ]; then
                info "Installing the Xcode Command Line Tools (provides make) - confirm the macOS dialog..."
                xcode-select --install || true
            else
                rest+=("$tool")
            fi
        done
        [ "${#rest[@]}" -eq 0 ] && return 0
        missing=("${rest[@]}")
    else
        pm="$(detect_package_manager)"
    fi
    if [ -z "$pm" ]; then
        err "No supported package manager found (apt-get, dnf, yum, pacman, zypper)."
        info "Please install the following tools manually: ${missing[*]}"
        info "1. CMake: https://cmake.org/download/"
        info "2. Ninja: https://github.com/ninja-build/ninja/releases"
        info "3. Python: https://www.python.org/downloads/"
        info "4. Git: https://git-scm.com/downloads"
        return 1
    fi

    local packages=()
    for tool in "${missing[@]}"; do
        # shellcheck disable=SC2207
        packages+=($(package_for "$pm" "$tool"))
    done
    info "Installing ${packages[*]} with $pm..."
    case "$pm" in
        brew)    brew install "${packages[@]}" ;;
        apt-get) as_root apt-get update && as_root apt-get install -y "${packages[@]}" ;;
        dnf)     as_root dnf install -y "${packages[@]}" ;;
        yum)     as_root yum install -y "${packages[@]}" ;;
        pacman)  as_root pacman -Sy --needed --noconfirm "${packages[@]}" ;;
        zypper)  as_root zypper --non-interactive install "${packages[@]}" ;;
    esac
    local rc=$?
    if [ $rc -eq 0 ]; then ok "Tool installation completed"; else err "Tool installation failed (exit code $rc)"; fi
    return $rc
}

# Function: Create Python virtual environment
create_venv() {
    info "Creating Python virtual environment: $VENV_PATH"
    if [ -x "$VENV_PATH/bin/python" ]; then
        ok "Virtual environment already exists, skipping creation"
        return 0
    fi
    local py
    if ! py="$(find_python)"; then
        err "Python 3.8+ was not found, cannot create the virtual environment"
        return 1
    fi
    if "$py" -m venv "$VENV_PATH" && [ -x "$VENV_PATH/bin/python" ]; then
        ok "Python virtual environment created successfully: $VENV_PATH ($("$VENV_PATH/bin/python" --version))"
        return 0
    fi
    err "Failed to create Python virtual environment with $py"
    return 1
}

# Function: Install requirements.txt into the virtual environment
install_python_dependencies() {
    info "Installing dependencies into the virtual environment..."
    local vpy="$VENV_PATH/bin/python"
    info "Upgrading pip..."
    "$vpy" -m pip install --upgrade pip || return 1
    if [ -f "$REQUIREMENTS_PATH" ]; then
        info "Installing dependencies: $REQUIREMENTS_PATH"
        "$vpy" -m pip install -r "$REQUIREMENTS_PATH" || return 1
        ok "Python dependencies installation completed"
    else
        info "Warning: Could not find requirements.txt file: $REQUIREMENTS_PATH"
    fi
    if "$VENV_PATH/bin/west" --version >/dev/null 2>&1; then
        ok "West is installed: $("$VENV_PATH/bin/west" --version)"
    else
        info "West is not installed, installing..."
        "$vpy" -m pip install west || return 1
    fi
}

# Function: Initialize west workspace
init_west_workspace() {
    info "Initializing west workspace: $UNISDK_ROOT"
    if (cd "$UNISDK_ROOT" && "$VENV_PATH/bin/west" init -l); then
        ok "West workspace initialization completed"
    else
        info "Note: west init failed - the workspace may already be initialized, or may need to be set up manually"
    fi
}

main() {
    local tools_ok=true venv_ok=false deps_ok=false

    set_telink_base
    install_tools || tools_ok=false

    create_venv && venv_ok=true
    if $venv_ok; then
        install_python_dependencies && deps_ok=true
        $deps_ok && init_west_workspace
    fi

    status() { if tool_installed "$1"; then echo "${C_GREEN}true${C_RESET}"; else echo "${C_RED}false${C_RESET}"; fi; }
    echo "${C_GREEN}=========================================${C_RESET}"
    echo "${C_GREEN}      Unisdk Environment Setup Complete!  ${C_RESET}"
    echo "${C_GREEN}=========================================${C_RESET}"
    echo "The following configurations have been completed:"
    echo "- Make: $(status make)"
    echo "- CMake: $(status cmake)"
    echo "- Ninja: $(status ninja)"
    echo "- Python: $(status python)"
    echo "- Git: $(status git)"
    echo "- Python virtual environment: $VENV_PATH ($venv_ok)"
    echo "- Python dependencies: $deps_ok"
    echo ""
    echo "Next steps:"
    info "1. Open a new terminal to apply environment variable changes"
    info "2. Activate virtual environment using: source $VENV_PATH/bin/activate"
    info "3. Run west tl-build to start building projects (toolchain will be checked and downloaded automatically)"
    echo "${C_CYAN}Log has been saved to: $LOG_FILE${C_RESET}"

    $tools_ok && $deps_ok
}

main

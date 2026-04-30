#!/bin/bash
# This script sets the TELINK_BASE environment variable permanently
# Run in Linux/macOS terminal

# Get the parent directory of the script location as TELINK_BASE
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
TELINK_BASE="$( cd "$SCRIPT_DIR/.." && pwd )"

# Set environment variable temporarily for current session
export TELINK_BASE="$TELINK_BASE"
echo "Setting TELINK_BASE environment variable to: $TELINK_BASE"

# Function to add export line to shell configuration file
add_to_shell_config() {
    local config_file=$1
    if [ -f "$config_file" ]; then
        # Check if the line already exists
        if ! grep -q "^export TELINK_BASE=$TELINK_BASE" "$config_file"; then
            echo "export TELINK_BASE=$TELINK_BASE" >> "$config_file"
            echo "Added TELINK_BASE to $config_file"
            return 0
        else
            echo "TELINK_BASE is already set in $config_file"
            return 0
        fi
    fi
    return 1
}

# Try to add to appropriate shell configuration file
echo "\nSetting TELINK_BASE permanently:"
# Check for zsh
if [ -n "$ZSH_VERSION" ]; then
    add_to_shell_config "$HOME/.zshrc"
# Check for bash
elif [ -n "$BASH_VERSION" ]; then
    # Try .bash_profile first (macOS), then .bashrc (Linux)
    if ! add_to_shell_config "$HOME/.bash_profile"; then
        add_to_shell_config "$HOME/.bashrc"
    fi
else
    # Try both common files as fallback
    add_to_shell_config "$HOME/.bashrc"
    add_to_shell_config "$HOME/.bash_profile"
    add_to_shell_config "$HOME/.zshrc"
fi

echo "\nTELINK_BASE environment variable has been set permanently."
echo "To apply the changes immediately, run: source ~/.bashrc  # or your shell's config file"
echo "Otherwise, the changes will take effect in new terminal sessions."

# Keep the window open to view messages
echo "\nPress Enter to continue..."
read
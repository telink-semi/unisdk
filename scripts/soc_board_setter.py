#!/usr/bin/env python3
"""
SOC/BOARD Configuration Setter
Handles SOC and BOARD configuration with proper dependency resolution
"""

import sys
import argparse
from pathlib import Path

# Add scripts directory to path to import kconfig_parser
script_dir = Path(__file__).parent
sys.path.insert(0, str(script_dir))

try:
    from kconfig_parser import KconfigParser
    import kconfiglib
except ImportError as e:
    print(f"Error importing required modules: {e}")
    print("Please ensure kconfiglib is installed: pip install kconfiglib")
    sys.exit(1)


class SocBoardConfigSetter:
    def __init__(self, kconfig_file, telink_base=None, config_file=None):
        """
        Initialize SOC/BOARD configuration setter
        
        Args:
            kconfig_file: Main Kconfig file path
            telink_base: Telink SDK base directory
            config_file: Path to .config file (defaults to telink_base/.config)
        """
        self.kconfig_file = Path(kconfig_file)
        self.telink_base = Path(telink_base) if telink_base else self.kconfig_file.parent
        self.config_file = Path(config_file) if config_file else self.telink_base / '.config'
        # Load the Kconfig
        self.kconf = kconfiglib.Kconfig(str(self.kconfig_file), suppress_traceback=True)

        # Initialize Kconfig parser
        try:
            self.parser = KconfigParser(kconfig_file, telink_base)
        except Exception as e:
            print(f"Failed to initialize Kconfig parser: {e}")
            raise
    
    def _get_dependencies_for_soc(self, soc_name):
        """Get all dependencies that need to be set before SOC"""
        soc_config_name = f"SOC_{soc_name}"
        if soc_config_name not in self.parser.socs:
            return []
        
        soc_info = self.parser.socs[soc_config_name]
        return soc_info['depends_on']
    
    def _get_dependencies_for_board(self, board_name):
        """Get all dependencies that need to be set before BOARD"""
        board_config_name = f"BOARD_{board_name}"
        if board_config_name not in self.parser.boards:
            return []
        
        board_info = self.parser.boards[board_config_name]
        return board_info['depends_on']
    
    def set_soc(self, soc_name):
        """Set SOC configuration with proper dependencies"""
        # print(f"Setting SOC: {soc_name}")
        
        # Validate SOC exists
        soc_config_name = f"SOC_{soc_name}"
        if soc_config_name not in self.parser.socs:
            raise ValueError(f"SOC not found: {soc_name}")
        
        # Get and set dependencies first
        dependencies = self._get_dependencies_for_soc(soc_name)
        if dependencies:
            # print(f"Setting SOC dependencies: {', '.join(dependencies)}")
            for dep in dependencies:
                if dep.startswith('SOC_SERIES_'):
                    # Set TLK_SOC_SERIES dependency
                    self._set_config_value(dep, 'y')
        
        # Set the SOC itself
        self._set_config_value(soc_config_name, 'y')
        
        # Unset other SOCs in the same series
        # self._unset_other_socs(soc_name)
        
        print(f"Successfully set SOC: {soc_name}")
    
    def set_board(self, board_name):
        """Set BOARD configuration with proper dependencies"""
        # print(f"Setting BOARD: {board_name}")
        
        # Validate BOARD exists
        board_config_name = f"BOARD_{board_name}"
        if board_config_name not in self.parser.boards:
            raise ValueError(f"BOARD not found: {board_name}")
        
        # Get and set dependencies first
        dependencies = self._get_dependencies_for_board(board_name)
        if dependencies:
            print(f"Setting BOARD dependencies: {', '.join(dependencies)}")
            for dep in dependencies:
                if dep.startswith('SOC_'):
                    # Set SOC dependency if not already set
                    soc_name = dep.replace('SOC_', '')
                    self.set_soc(soc_name)
        
        # Set the BOARD itself
        self._set_config_value(board_config_name, 'y')
        
        # Set BOARD string configuration
        board_string_value = board_name
        self._set_config_value('BOARD', f'"{board_string_value}"')
        
        # Set TLK_HAS_BOARD_SELECTED
        self._set_config_value('TLK_HAS_BOARD_SELECTED', 'y')
        
        # Unset other boards
        # self._unset_other_boards(board_name)
        
        print(f"Successfully set BOARD: {board_name}")
    
    def set_soc_board(self, soc_name, board_name):
        """Set both SOC and BOARD with proper validation and dependency resolution"""
        # print(f"Setting SOC/BOARD combination: {soc_name}/{board_name}")
        
        # Validate combination first
        is_valid, error_msg = self.parser.validate_soc_board_combination(soc_name, board_name)
        if not is_valid:
            raise ValueError(f"Invalid SOC/BOARD combination: {error_msg}")
        
        # Set SOC first (it may be a dependency for BOARD)
        self.set_soc(soc_name)
        
        # Then set BOARD
        self.set_board(board_name)
        
        print(f"Successfully set SOC/BOARD combination: {soc_name}/{board_name}")
    
    def _set_config_value(self, key, value):
        """Set a configuration value using kconfiglib"""
        try:
            # Always use direct file manipulation for critical values
            if key == 'TLK_HAS_BOARD_SELECTED' or key == 'BOARD' or key.startswith('TLK_BOARD_'):
                self._set_config_direct(key, value)
                return
            
            # Load existing config if it exists
            if self.config_file.exists():
                self.kconf.load_config(str(self.config_file))
            else:
                self.kconf.load_config()  # Load default config
            
            # Set the value
            if key in self.kconf.syms:
                sym = self.kconf.syms[key]
                ret = sym.set_value(value)
                if not ret:
                    print(f"Warning: Failed to set {key}={value} (symbol may not be visible due to dependencies)")
                    # Fallback to direct file manipulation
                    self._set_config_direct(key, value)
                    return
            else:
                print(f"Warning: Symbol {key} not found in Kconfig")
                # Debug: print available symbols containing 'BOARD'
                board_symbols = [name for name in self.kconf.syms.keys() if 'BOARD' in name]
                if board_symbols:
                    print(f"Available BOARD symbols: {board_symbols}")
                # Fallback to direct file manipulation
                self._set_config_direct(key, value)
                return
            
            # Write the config
            self.kconf.write_config(str(self.config_file))
            
        except Exception as e:
            print(f"Warning: Failed to set {key}={value}: {e}")
            # Fallback to direct file manipulation
            self._set_config_direct(key, value)
    
    def _set_config_direct(self, key, value):
        """Direct file manipulation as fallback"""
        if not self.config_file.exists():
            self.config_file.touch()
        
        content = self.config_file.read_text()
        
        # Remove existing setting
        lines = [line for line in content.split('\n') 
                if not line.startswith(f'CONFIG_TLK_{key}=') and not line.startswith(f'# CONFIG_TLK_{key} is not set')]
        
        # Add new setting with CONFIG_TLK_ prefix
        config_key = f"CONFIG_TLK_{key}"
        if value == 'y':
            lines.append(f'{config_key}={value}')
        elif value == 'n':
            lines.append(f'# {config_key} is not set')
        else:
            lines.append(f'{config_key}={value}')
        
        self.config_file.write_text('\n'.join(lines))
        print(f"  Directly set {config_key}={value} in config file")
    
    def _unset_other_socs(self, current_soc):
        """Unset other SOCs to maintain single selection"""
        for soc_name in self.parser.socs:
            if not soc_name.startswith('SOC_'):
                continue
            
            clean_soc_name = soc_name.replace('SOC_', '')
            if clean_soc_name != current_soc:
                self._set_config_value(soc_name, 'n')
    
    def _unset_other_boards(self, current_board):
        """Unset other boards to maintain single selection"""
        for board_name in self.parser.boards:
            if not board_name.startswith('BOARD_'):
                continue
            
            clean_board_name = board_name.replace('BOARD_', '')
            if clean_board_name != current_board:
                self._set_config_value(board_name, 'n')


def main():
    parser = argparse.ArgumentParser(description='SOC/BOARD configuration setter')
    parser.add_argument('--kconfig', required=True, help='Kconfig file path')
    parser.add_argument('--telink-base', help='Telink SDK base directory')
    parser.add_argument('--config-file', help='Path to .config file (defaults to telink_base/.config)')
    parser.add_argument('--soc', help='Set SOC configuration')
    parser.add_argument('--board', help='Set BOARD configuration')
    parser.add_argument('--validate-only', action='store_true', 
                       help='Only validate SOC/BOARD combination without setting')
    
    args = parser.parse_args()
    
    if not args.soc and not args.board:
        print("Error: At least --soc or --board must be specified")
        return 1
    
    try:
        setter = SocBoardConfigSetter(args.kconfig, args.telink_base, args.config_file)
        
        if args.soc and args.board:
            # Validate and set both
            if args.validate_only:
                is_valid, error_msg = setter.parser.validate_soc_board_combination(args.soc, args.board)
                if is_valid:
                    print(f"SOC {args.soc} and BOARD {args.board} combination is valid")
                    return 0
                else:
                    print(f"SOC {args.soc} and BOARD {args.board} combination is invalid: {error_msg}")
                    return 1
            else:
                setter.set_soc_board(args.soc, args.board)
        
        elif args.soc:
            # Set only SOC
            setter.set_soc(args.soc)
        
        elif args.board:
            # Set only BOARD (will set SOC dependencies automatically)
            setter.set_board(args.board)
        
        return 0
        
    except Exception as e:
        print(f"Error: {e}")
        return 1


if __name__ == '__main__':
    sys.exit(main())
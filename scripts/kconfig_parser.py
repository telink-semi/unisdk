#!/usr/bin/env python3
"""
Kconfig Parser Tool - Using kconfiglib professional library
For parsing Telink SDK Kconfig files to get SOC and BOARD dependencies
"""

import os
import sys
import argparse
from pathlib import Path

try:
    from kconfiglib import Kconfig, Symbol, BOOL
except ImportError:
    print("Error: kconfiglib library is required")
    print("Please run: pip install kconfiglib")
    sys.exit(1)


class KconfigParser:
    def __init__(self, kconfig_file, telink_base=None):
        """
        Initialize Kconfig parser
        
        Args:
            kconfig_file: Kconfig file path
            telink_base: Telink SDK base directory
        """
        self.kconfig_file = Path(kconfig_file)
        self.telink_base = Path(telink_base) if telink_base else self.kconfig_file.parent
        
        # Set environment variables to ensure kconfiglib can find all files
        os.environ['srctree'] = str(self.telink_base)
        os.environ['KCONFIG_CONFIG'] = str(self.telink_base / '.config')
        
        # Use kconfiglib to parse Kconfig
        try:
            self.kconf = Kconfig(str(self.kconfig_file), warn=False)
        except Exception as e:
            print(f"Failed to parse Kconfig: {e}")
            raise
        
        # Parse SOC and BOARD information
        self.socs = {}
        self.boards = {}
        self._parse_symbols()
    
    def _parse_symbols(self):
        """Parse all symbols to extract SOC and BOARD information"""
        for name, symbol in self.kconf.syms.items():
            if not isinstance(symbol, Symbol):
                continue
                
            # Skip auto-generated symbols
            if name.startswith('_') or name.startswith('MODULE_'):
                continue
            
            # Extract SOC configuration
            if self._is_soc_config(name, symbol):
                self.socs[name] = {
                    'name': name,
                    'prompt': self._get_prompt(symbol),
                    'help': self._get_help(symbol),
                    'depends_on': self._get_dependencies(symbol),
                    'type': symbol.type,
                    'visibility': symbol.visibility
                }
            
            # Extract BOARD configuration
            elif self._is_board_config(name, symbol):
                self.boards[name] = {
                    'name': name,
                    'prompt': self._get_prompt(symbol),
                    'help': self._get_help(symbol),
                    'depends_on': self._get_dependencies(symbol),
                    'type': symbol.type,
                    'visibility': symbol.visibility
                }
    
    def _is_soc_config(self, name, symbol):
        """Determine if this is a SOC configuration"""
        # SOC configurations usually start with TLK_SOC_, but not TLK_SOC_SERIES_
        return (name.startswith('TLK_SOC_') and 
                not name.startswith('TLK_SOC_SERIES_') and
                symbol.type == BOOL)
    
    def _is_board_config(self, name, symbol):
        """Determine if this is a BOARD configuration"""
        # BOARD configurations now start with TLK_BOARD_ prefix
        if not name.startswith('TLK_BOARD_') or symbol.type != BOOL or not self._get_prompt(symbol):
            return False
        
        # BOARD configurations should depend on SOC configurations
        deps = self._get_dependencies(symbol)
        has_soc_dep = any(dep.startswith('TLK_SOC_') for dep in deps)
        
        # BOARD configurations usually have TLK_HAS_BOARD_SELECTED in select
        has_board_select = any('TLK_HAS_BOARD_SELECTED' in str(item) for item in getattr(symbol, 'selects', []))
        
        return has_soc_dep or has_board_select
    
    def _get_prompt(self, symbol):
        """Get symbol prompt information"""
        if symbol.nodes:
            for node in symbol.nodes:
                if hasattr(node, 'prompt') and node.prompt:
                    return node.prompt[0]  # prompt is (text, condition) tuple
        return ""
    
    def _get_help(self, symbol):
        """Get symbol help information"""
        help_text = []
        if symbol.nodes:
            for node in symbol.nodes:
                if hasattr(node, 'help') and node.help:
                    help_text.append(node.help)
        return '\n'.join(help_text).strip()
    
    def _get_dependencies(self, symbol):
        """Get symbol dependencies"""
        deps = []
        
        # Get direct dependencies (depends on)
        if symbol.direct_dep:
            deps.extend(self._extract_symbols_from_expr(symbol.direct_dep))
        
        # Get selected dependencies (select)
        if hasattr(symbol, 'selects') and symbol.selects:
            for select_item in symbol.selects:
                # select might be tuple (symbol, condition)
                if isinstance(select_item, tuple) and len(select_item) >= 1:
                    select_symbol = select_item[0]
                else:
                    select_symbol = select_item
                
                if hasattr(select_symbol, 'name'):
                    deps.append(select_symbol.name)
        
        # For SOC configurations, also check if they're conditionally included
        if hasattr(symbol, 'name') and self._is_soc_config(symbol.name, symbol):
            # Check if this SOC is conditionally included via TLK_SOC_SERIES
            soc_series_dep = self._get_soc_series_dependency(symbol.name)
            if soc_series_dep:
                deps.append(soc_series_dep)
        
        # Filter out None values and empty strings, and TLK_HAS_BOARD_SELECTED (this is select not depends on)
        deps = [dep for dep in deps if dep and isinstance(dep, str) and dep != 'TLK_HAS_BOARD_SELECTED']
        
        return deps
    
    def _get_soc_series_dependency(self, soc_name):
        """Get the TLK_SOC_SERIES dependency for a SOC configuration using dynamic parsing"""
        # Find the SOC symbol in the Kconfig tree
        soc_symbol = self.kconf.syms.get(soc_name)
        if not soc_symbol:
            return None
        
        # Traverse up the node tree to find conditional inclusion (if statements)
        node = soc_symbol.nodes[0] if soc_symbol.nodes else None
        while node and node.parent:
            # Check if this node is conditionally included by an 'if' statement
            if hasattr(node.parent, 'if') and getattr(node.parent, 'if'):
                # Get the condition expression
                condition_expr = self._expr_to_str(getattr(node.parent, 'if'))
                if condition_expr and condition_expr.startswith('TLK_SOC_SERIES_'):
                    return condition_expr
            node = node.parent
        
        # If no conditional inclusion found, try to infer from file path
        # SOC files are typically in directories like soc/TL321X/Kconfig
        if soc_symbol.nodes:
            soc_file = soc_symbol.nodes[0].filename
            if 'soc' in soc_file:
                # Extract directory name from path like soc/TL321X/Kconfig
                parts = soc_file.replace('\\', '/').split('/')
                for i, part in enumerate(parts):
                    if part == 'soc' and i + 1 < len(parts):
                        series_dir = parts[i + 1]
                        # Map directory name to TLK_SOC_SERIES symbol
                        if series_dir.startswith('TL'):
                            return f'TLK_SOC_SERIES_{series_dir}'
                        elif series_dir.startswith('TLSR'):
                            return f'TLK_SOC_SERIES_{series_dir}'
        
        return None
    
    def _extract_symbols_from_expr(self, expr):
        """Extract all symbols from expression"""
        symbols = []
        
        if expr is None:
            return symbols
        
        # If it's a tuple, the second element is the actual expression
        if isinstance(expr, tuple) and len(expr) >= 2:
            expr = expr[1]
        
        if hasattr(expr, 'name'):
            # This is a symbol
            symbols.append(expr.name)
        elif hasattr(expr, 'left') and hasattr(expr, 'right'):
            # This is a binary expression
            symbols.extend(self._extract_symbols_from_expr(expr.left))
            symbols.extend(self._extract_symbols_from_expr(expr.right))
        elif hasattr(expr, 'items'):
            # This is a list (like AND or OR expression)
            for item in expr.items:
                symbols.extend(self._extract_symbols_from_expr(item))
        
        return symbols
    
    def get_all_socs(self):
        """Get all available SOC options"""
        return list(self.socs.values())
    
    def get_all_boards(self):
        """Get all available BOARD options"""
        return list(self.boards.values())
    
    def validate_soc_board_combination(self, soc_name, board_name):
        """Validate if SOC and BOARD combination is valid"""
        # Build SOC and BOARD configuration names
        if soc_name:
            soc_config_name = f"TLK_SOC_{soc_name}"
        else:
            soc_config_name = None
        if board_name:
            board_config_name = f"TLK_BOARD_{board_name}"
        else:
            board_config_name = None
        
        # Check if SOC exists
        if soc_config_name and soc_config_name not in self.socs:
            return False, f"SOC not found: {soc_name}"
        
        # Check if BOARD exists
        if board_config_name and board_config_name not in self.boards:
            return False, f"BOARD not found: {board_name}"
        
        if board_config_name and soc_config_name:
            # Check if BOARD depends on specified SOC
            board_deps = self.boards[board_config_name]['depends_on']
            if soc_config_name not in board_deps:
                # Get all SOCs supported by BOARD
                supported_socs = [dep.replace('TLK_SOC_', '') for dep in board_deps if dep.startswith('TLK_SOC_')]
                if supported_socs:
                    return False, f"BOARD {board_name} does not support SOC {soc_name}, supported SOCs: {', '.join(supported_socs)}"
                else:
                    return False, f"BOARD {board_name} does not depend on any SOC configuration"
            
        return True, ""
    
    def get_supported_boards_for_soc(self, soc_name):
        """Get all boards supported by specified SOC"""
        soc_config_name = f"TLK_SOC_{soc_name}"
        supported_boards = []
        
        for board_name, board_info in self.boards.items():
            if soc_config_name in board_info['depends_on']:
                # Remove TLK_BOARD_ prefix for display
                display_name = board_name.replace('TLK_BOARD_', '')
                supported_boards.append({
                    'name': display_name,
                    'prompt': board_info['prompt'],
                    'type': board_info['type'],
                    'depends_on': board_info['depends_on'],
                    'help': board_info['help']
                })
        
        return supported_boards
    
    def get_supported_socs_for_board(self, board_name):
        """Get all SOCs supported by specified BOARD"""
        board_config_name = f"TLK_BOARD_{board_name}"
        if board_config_name not in self.boards:
            return []
        
        board_deps = self.boards[board_config_name]['depends_on']
        supported_socs = []
        
        for dep in board_deps:
            if dep.startswith('TLK_SOC_') and dep in self.socs:
                supported_socs.append({
                    'name': dep.replace('TLK_SOC_', ''),
                    'prompt': self.socs[dep]['prompt']
                })
        
        return supported_socs


def main():
    parser = argparse.ArgumentParser(description='Kconfig parser tool')
    parser.add_argument('--kconfig', required=True, help='Kconfig file path')
    parser.add_argument('--list-socs', action='store_true', help='List all available SOCs')
    parser.add_argument('--list-boards', action='store_true', help='List all available BOARDS')
    parser.add_argument('--validate', nargs=2, metavar=('SOC', 'BOARD'), help='Validate SOC and BOARD combination')
    parser.add_argument('--supported-boards', metavar='SOC', help='Get boards supported by specified SOC')
    parser.add_argument('--supported-socs', metavar='BOARD', help='Get SOCs supported by specified BOARD')
    
    args = parser.parse_args()
    
    try:
        kconfig_parser = KconfigParser(args.kconfig)
    except Exception as e:
        print(f"Failed to initialize Kconfig parser: {e}")
        return 1
    
    if args.list_socs:
        socs = kconfig_parser.get_all_socs()
        print("=== Available SOCs ===")
        for soc in socs:
            soc_name = soc['name'].replace('TLK_SOC_', '')
            deps = ', '.join(soc['depends_on']) if soc['depends_on'] else 'No dependencies'
            print(f"{soc_name}: {soc['prompt']} (dependencies: {deps})")
        return 0
    
    if args.list_boards:
        boards = kconfig_parser.get_all_boards()
        print("=== Available BOARDS ===")
        for board in boards:
            deps = ', '.join(board['depends_on']) if board['depends_on'] else 'No dependencies'
            print(f"{board['name']}: {board['prompt']} (dependencies: {deps})")
        return 0
    
    if args.validate:
        soc_name, board_name = args.validate
        is_valid, error_msg = kconfig_parser.validate_soc_board_combination(soc_name, board_name)
        if is_valid:
            print(f"SOC {soc_name} and BOARD {board_name} combination is valid")
            return 0
        else:
            print(f"SOC {soc_name} and BOARD {board_name} combination is invalid: {error_msg}")
            return 1
    
    if args.supported_boards:
        soc_name = args.supported_boards
        boards = kconfig_parser.get_supported_boards_for_soc(soc_name)
        print(f"=== BOARDS supported by SOC {soc_name} ===")
        if boards:
            for board in boards:
                print(f"{board['name']}: {board['prompt']}")
        else:
            print(f"No boards found supporting SOC {soc_name}")
        return 0
    
    if args.supported_socs:
        board_name = args.supported_socs
        socs = kconfig_parser.get_supported_socs_for_board(board_name)
        print(f"=== SOCs supported by BOARD {board_name} ===")
        if socs:
            for soc in socs:
                print(f"{soc['name']}: {soc['prompt']}")
        else:
            print(f"No SOCs found supporting BOARD {board_name}")
        return 0
    
    return 0


if __name__ == '__main__':
    sys.exit(main())
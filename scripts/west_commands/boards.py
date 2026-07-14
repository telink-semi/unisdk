#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0

import os
import sys
from west.commands import WestCommand

# Add scripts directory to path to import kconfig_parser
script_dir = os.path.join(os.path.dirname(__file__), '..')
sys.path.insert(0, script_dir)



class Boards(WestCommand):
    """Command to display information about supported boards."""

    def __init__(self):
        super().__init__('tl-boards',
                         'display information about supported Telink boards',
                         'Usage: west tl-boards')

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(self.name, help=self.help)
        parser.add_argument('-v', '--verbose', action='store_true',
                           help='Show detailed information about each board')
        parser.add_argument('--soc', metavar='SOC', 
                           help='Filter boards by supported SOC (e.g., TL3218X)')
        return parser

    def do_run(self, args, unknown_args):
        # Import KconfigParser inside the method to use self.err()
        try:
            from kconfig_parser import KconfigParser
        except ImportError as e:
            self.err(f"Failed to import kconfig_parser: {e}")
            self.err("Please ensure kconfig_parser.py is in the scripts directory")
            return 1

        # Get the repository root directory
        repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
        kconfig_file = os.path.join(repo_root, 'Kconfig.chip')

        if not os.path.isfile(kconfig_file):
            self.err(f"Kconfig.chip file not found: {kconfig_file}")
            return 1

        # Initialize Kconfig parser
        try:
            parser = KconfigParser(kconfig_file, repo_root)
        except Exception as e:
            self.err(f"Failed to initialize Kconfig parser: {e}")
            return 1

        # Get all boards or filter by SOC
        if args.soc:
            boards = parser.get_supported_boards_for_soc(args.soc)
            if not boards:
                self.inf(f"No boards found supporting SOC {args.soc}")
                return 0
            self.inf(f"Boards supporting SOC {args.soc} ({len(boards)}):")
        else:
            boards = parser.get_all_boards()
            if not boards:
                self.inf("No supported boards found")
                return 0
            self.inf(f"Supported boards ({len(boards)}):")
        
        self.inf("=" * 60)

        for board in sorted(boards, key=lambda x: x['name']):
            board_name = board['name'].replace('TLK_BOARD_', '')
            
            if args.verbose:
                self.inf(f"\nBoard: {board_name}")
                self.inf("-" * 40)
                self.inf(f"  Full name: {board['name']}")
                self.inf(f"  Prompt: {board['prompt']}")
                self.inf(f"  Type: {board['type']}")
                self.inf(f"  Dependencies: {', '.join(board['depends_on']) if board['depends_on'] else 'None'}")
                
                # Show supported SOCs for this board
                supported_socs = parser.get_supported_socs_for_board(board_name)
                if supported_socs:
                    self.inf(f"  Supported SOCs:")
                    for soc in supported_socs:
                        self.inf(f"    - {soc['name']}: {soc['prompt']}")
                else:
                    self.inf(f"  Supported SOCs: None")
                
                if board['help']:
                    self.inf("  Help:")
                    for line in board['help'].split('\n'):
                        if line.strip():
                            self.inf(f"    {line.strip()}")
            else:
                deps = f" (depends on: {', '.join(board['depends_on'])})" if board['depends_on'] else ""
                self.inf(f"- {board_name}: {board['prompt']}{deps}")

        return 0
#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0

import os
import sys
from west.commands import WestCommand

# Add scripts directory to path to import kconfig_parser
script_dir = os.path.join(os.path.dirname(__file__), '..')
sys.path.insert(0, script_dir)

class Socs(WestCommand):
    """Command to display information about supported SoCs."""

    def __init__(self):
        super().__init__('tl-socs',
                         'display information about supported Telink socs',
                         'Usage: west tl-socs')

    def do_add_parser(self, parser_adder):
        parser = parser_adder.add_parser(self.name, help=self.help)
        parser.add_argument('-v', '--verbose', action='store_true',
                           help='Show detailed information about each SoC')
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

        # Get all SOCs
        socs = parser.get_all_socs()
        
        if not socs:
            self.inf("No supported SoCs found")
            return 0

        self.inf(f"Supported SoCs ({len(socs)}):")
        self.inf("=" * 60)

        for soc in sorted(socs, key=lambda x: x['name']):
            soc_name = soc['name'].replace('TLK_SOC_', '')
            
            if args.verbose:
                self.inf(f"\nSoC: {soc_name}")
                self.inf("-" * 40)
                self.inf(f"  Full name: {soc['name']}")
                self.inf(f"  Prompt: {soc['prompt']}")
                self.inf(f"  Type: {soc['type']}")
                self.inf(f"  Dependencies: {', '.join(soc['depends_on']) if soc['depends_on'] else 'None'}")
                
                if soc['help']:
                    self.inf("  Help:")
                    for line in soc['help'].split('\n'):
                        if line.strip():
                            self.inf(f"    {line.strip()}")
            else:
                deps = f" (depends on: {', '.join(soc['depends_on'])})" if soc['depends_on'] else ""
                self.inf(f"- {soc_name}: {soc['prompt']}{deps}")

        return 0
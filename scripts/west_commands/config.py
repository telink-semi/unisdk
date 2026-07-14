import argparse
import os
import pathlib
import subprocess
import sys
from pathlib import Path
from west.commands import WestCommand
from west.configuration import config
from validation_utils import validate_soc_board
# Add scripts directory to path to import kconfig_parser
script_dir = pathlib.Path(__file__).parent.parent
sys.path.insert(0, str(script_dir))

# Add descriptive help text
CONFIG_USAGE = '''
west tl-config [-h] [-d BUILD_DIR]
           [--list-socs] [--list-boards] [--validate SOC BOARD]
           [-c] [source_dir]''' 

CONFIG_DESCRIPTION = '''
Configure a Telink application using interactive menuconfig.

The configuration will be saved to the .config file in the build directory.
If no build directory is specified, it will default to a 'build' directory
in the source directory.''' 

class Config(WestCommand):
    """West command for configuring Telink applications"""
    
    def __init__(self):
        super().__init__(
            'tl-config',
            'configure a Telink application',
            CONFIG_DESCRIPTION,
            accepts_unknown_args=True
        )
        self.source_dir = None
        self.build_dir = None
    
    def do_add_parser(self, parser_adder):
        """Add command line arguments"""
        parser = parser_adder.add_parser(
            self.name,
            help=self.help,
            description=self.description,
            usage=CONFIG_USAGE,
            formatter_class=argparse.RawDescriptionHelpFormatter
        )
        
        # Add configuration options
        parser.add_argument("source_dir", nargs='?', default=os.getcwd(),
                          help="Application source directory (default: current directory)")
        parser.add_argument("-d", "--build-dir",
                          help="Build directory")
        parser.add_argument("--list-socs", action="store_true",
                          help="List all available SOCs")
        parser.add_argument("--list-boards", action="store_true", 
                          help="List all available boards")
        parser.add_argument("--validate", nargs=2, metavar=('SOC', 'BOARD'),
                          help="Validate SOC and BOARD combination")
        parser.add_argument("-c", "--cmake-only", action="store_true",
                          help="Only run CMake configuration, skip interactive config GUI")
        
        return parser
            
    def _get_kconfig_parser(self):
        """Get Kconfig parser"""
        try:
            from kconfig_parser import KconfigParser
        except ImportError as e:
            self.die(f"Error: Failed to import kconfig_parser: {e}")
            return None
        
        try:
            # Get Kconfig file path
            telink_base = pathlib.Path(__file__).resolve().parent.parent.parent
            kconfig_file = telink_base / 'Kconfig.chip'
            
            if not kconfig_file.exists():
                self.die(f"Kconfig file not found: {kconfig_file}")
                return None
            
            # Create parser
            return KconfigParser(str(kconfig_file), str(telink_base))
            
        except Exception as e:
            self.die(f"Error creating Kconfig parser: {e}")
            return None
    
    def do_run(self, args, unknown_args):
        """Execute the config command"""
        
        # Handle list operations
        if args.list_socs:
            parser = self._get_kconfig_parser()
            if not parser:
                self.die("Error: Kconfig parser not available")
                return 1
            
            socs = parser.get_all_socs()
            self.banner("Available SOCs")
            for soc in socs:
                deps = ', '.join(soc['depends_on']) if soc['depends_on'] else "No dependencies"
                self.inf(f"{soc['name'].replace('TLK_SOC_', '')}: {soc['prompt']} (dependencies: {deps})")
            return 0
        
        if args.list_boards:
            parser = self._get_kconfig_parser()
            if not parser:
                self.die("Error: Kconfig parser not available")
                return 1
            
            boards = parser.get_all_boards()
            self.banner("Available BOARDS")
            for board in boards:
                deps = ', '.join(board['depends_on']) if board['depends_on'] else "No dependencies"
                self.inf(f"{board['name']}: {board['prompt']} (dependencies: {deps})")
            return 0
        
        if args.validate:
            soc_name, board_name = args.validate
            is_valid, error_msg = validate_soc_board(soc_name, board_name, warn_func=self.wrn)
            if is_valid:
                self.inf(f"SOC {soc_name} and BOARD {board_name} combination is valid")
                return 0
            else:
                self.die(f"SOC {soc_name} and BOARD {board_name} combination is invalid: {error_msg}")
                return 1

        self.source_dir = pathlib.Path(args.source_dir).resolve()
        
        # Set build directory
        if args.build_dir:
            self.build_dir = pathlib.Path(args.build_dir).resolve()
        else:
            self.build_dir = self.source_dir / "build"

        """Run menuconfig for interactive configuration by calling west tl-build"""
        try:
            # Run west tl-build command directly with full terminal interaction
            import subprocess
            import os
            
            # Build the command
            cmd = [
                'west', 'tl-build',
                str(self.source_dir),
                '--build-dir', str(self.build_dir),
                '-k',
            ]
            
            # Add --cmake-only if requested
            if args.cmake_only:
                cmd.append('--cmake-only')

            self.inf(f"Running command: {' '.join(cmd)}")
            # Run the command with full terminal interaction
            # This allows the menuconfig interface to be displayed
            result = subprocess.run(cmd, 
                                   cwd=os.environ.get('TELINK_BASE', os.getcwd()),
                                   stdout=None,  # Direct to terminal
                                   stderr=None,  # Direct to terminal
                                   stdin=None,   # Direct to terminal
                                   shell=False)
            
            return result.returncode == 0

        except Exception as e:
            self.die(f"Error running configuration: {e}")
            return False
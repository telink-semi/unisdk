import argparse
import os
import pathlib
import subprocess
import sys
from pathlib import Path
from west.commands import WestCommand
from west.configuration import config

# Add scripts directory to path
script_dir = pathlib.Path(__file__).parent.parent
sys.path.insert(0, str(script_dir))


BDT_DESCRIPTION = '''
BDT tool operations for Telink applications.


BDT tool path can be set via:
1. --bdt-path option
2. BDT_PATH environment variable
3. Default: looks for bdt in system PATH'''

class Bdt(WestCommand):
    """West command for BDT operations using BDT tool"""
    
    def __init__(self):
        super().__init__(
            'tl-bdt',
            'BDT operations for Telink applications',
            BDT_DESCRIPTION,
            accepts_unknown_args=True
        )
        self.build_dir = None
        self.bdt_path = None
    
    def do_add_parser(self, parser_adder):
        """Add command line arguments"""
        parser = parser_adder.add_parser(
            self.name,
            help=self.help,
            description=self.description,
            formatter_class=argparse.RawDescriptionHelpFormatter
        )
        
        # Global flash options
        parser.add_argument("-d", "--build-dir",
                          help="Build directory (default: 'build' in current directory)")
        parser.add_argument("-c", "--core", action="store_true",
                          help="Operate on core instead of flash")
        parser.add_argument("--usb", action="store_true",
                          help="Use USB mode (default: EVK mode)")
        parser.add_argument("--bdt-path",
                          help="Path to BDT tool executable")
        parser.add_argument("--chip",
                          help="Chip type (e.g., B92, TL721X, TL321X)")
        parser.add_argument("--sws", nargs=4,
                          metavar=('EVK_CLK', 'EVK_RATE', 'TARGET_CLK', 'TARGET_RATE'),
                          help="Set SWire clock values")
        parser.add_argument("--bus", type=int,
                          help="USB bus ID (required when multiple USB devices exist)")
        parser.add_argument("--devid", type=int,
                          help="USB device ID (required when multiple USB devices exist)")
        
        # Subcommands
        subparsers = parser.add_subparsers(dest='operation', required=True,
                                         help='Flash operation to perform')
        
        # Download subcommand
        download_parser = subparsers.add_parser('download',
                                               help='Download firmware using BDT tool')
        download_parser.add_argument("-i", "--input",
                                    help="Path to firmware file (default: build/telink/Telink.bin)")
        download_parser.add_argument("-a", "--addr", default="0x00",
                                    help="Flash address to download to (default: 0x00)")
        
        # Read subcommand
        read_parser = subparsers.add_parser('read',
                                          help='Read data from flash using BDT tool')
        read_parser.add_argument("-a", "--addr", default="0x00",
                               help="Flash address to read from (default: 0x00)")
        read_parser.add_argument("-s", "--size", required=True,
                               help="Number of bytes to read (e.g., 16, 1k)")
        read_parser.add_argument("-o", "--output",
                               help="Output file path (default: save<timestamp>.bin)")
        
        # Write subcommand
        write_parser = subparsers.add_parser('write',
                                           help='Write data to flash')
        write_parser.add_argument("-a", "--addr", default="0x00",
                                help="Flash address to write to (default: 0x00)")
        write_parser.add_argument("-s", "--size",
                                help="Number of bytes to write (required if writing raw data)")
        write_parser.add_argument("-i", "--input",
                                help="Path to input file (required if not writing raw data)")
        write_parser.add_argument("data", nargs="*",
                                help="Raw data bytes to write (e.g., 01 02 03 04)")
        write_parser.add_argument("-e", "--erase", action="store_true",
                                help="Erase before writing")
        
        # Erase subcommand
        erase_parser = subparsers.add_parser('erase',
                                           help='Erase flash')
        erase_parser.add_argument("-a", "--addr", default="0x00",
                                help="Flash address to erase from (default: 0x00)")
        erase_parser.add_argument("-s", "--size", required=True,
                                help="Number of bytes to erase (e.g., 1k, 4k)")
        
        # Lock subcommand
        lock_parser = subparsers.add_parser('lock',
                                          help='Lock flash region')
        lock_parser.add_argument("-a", "--addr", default="0", help="Flash address to lock (default: 0)")
        lock_parser.add_argument("-s", "--size", default="0", help="Size of region to lock (default: 0, all regions)")
        
        # Unlock subcommand
        unlock_parser = subparsers.add_parser('unlock',
                                            help='Unlock flash')
        
        # Reset subcommand
        reset_parser = subparsers.add_parser('reset',
                                           help='Reset device')
        reset_parser.add_argument("-c", "--core", action="store_true",
                                help="Reset from core instead of flash")
        
        return parser
        
    def _find_bdt_path(self):
        """Find BDT tool path"""
        self.inf("Looking for BDT tool...")
        
        # Check if provided via command line with directory
        if self.bdt_path:
            self.inf(f"Checking command line path: {self.bdt_path}")
            
            # If it's already an executable file, use it directly
            if os.path.isfile(self.bdt_path):
                if os.access(self.bdt_path, os.X_OK):
                    self.inf(f"   Found executable at command line path: {self.bdt_path}")
                    return self.bdt_path
                else:
                    self.inf(f"   File exists but not executable: {self.bdt_path}")
            
            # If it's a directory, look for BDT executable inside
            if os.path.isdir(self.bdt_path):
                self.inf(f"   Command line path is a directory, searching for BDT executable...")
                bdt_filename = 'Cmd_download_tool.exe' if os.name == 'nt' else 'bdt'
                bdt_path = os.path.join(self.bdt_path, bdt_filename)
                if os.path.exists(bdt_path):
                    if os.access(bdt_path, os.X_OK):
                        self.inf(f"   Found executable in directory: {bdt_path}")
                        return bdt_path
                    else:
                        self.inf(f"   Found file but not executable: {bdt_path}")
            
            # Try different BDT executable names
            if os.name == 'nt':
                for bdt_filename in ['Cmd_download_tool.exe']:
                    bdt_path = os.path.join(self.bdt_path, bdt_filename)
                    if os.path.exists(bdt_path):
                        if os.access(bdt_path, os.X_OK):
                            self.inf(f"   Found alternative executable: {bdt_path}")
                            return bdt_path
            
        # Check BDT_PATH environment variable
        if 'BDT_PATH' in os.environ:
            self.inf(f"Checking BDT_PATH environment variable: {os.environ['BDT_PATH']}")
            bdt_path = os.environ['BDT_PATH']
            if os.path.isfile(bdt_path):
                if os.access(bdt_path, os.X_OK):
                    self.inf(f"   Found executable in BDT_PATH: {bdt_path}")
                    return bdt_path
                else:
                    self.inf(f"   BDT_PATH file exists but not executable: {bdt_path}")
            elif os.path.isdir(bdt_path):
                bdt_filename = 'Cmd_download_tool.exe' if os.name == 'nt' else 'bdt'
                bdt_path_full = os.path.join(bdt_path, bdt_filename)
                if os.path.exists(bdt_path_full) and os.access(bdt_path_full, os.X_OK):
                    self.inf(f"   Found executable in BDT_PATH directory: {bdt_path_full}")
                    return bdt_path_full
        
        # Look in system PATH
        self.inf(f"Searching for BDT in system PATH...")
        for path in os.environ['PATH'].split(os.pathsep):
            if not path:
                continue
            
            # Try all possible BDT executable names
            if os.name == 'nt':
                bdt_path = os.path.join(path, 'Cmd_download_tool.exe')
                if os.path.exists(bdt_path):
                    if os.access(bdt_path, os.X_OK):
                        self.inf(f"   Found executable in PATH: {bdt_path}")
                        return bdt_path
            else:
                bdt_path = os.path.join(path, 'bdt')
                if os.path.exists(bdt_path):
                    if os.access(bdt_path, os.X_OK):
                        self.inf(f"   Found executable in PATH: {bdt_path}")
                        return bdt_path
        
        # Check Telink base directory
        if 'TELINK_BASE' in os.environ:
            self.inf(f"Checking TELINK_BASE directory: {os.environ['TELINK_BASE']}")
            tools_dir = os.path.join(os.environ['TELINK_BASE'], 'tools')
            if os.path.exists(tools_dir):
                if os.name == 'nt':
                    bdt_path = os.path.join(tools_dir, 'Cmd_download_tool.exe')
                    if os.path.exists(bdt_path):
                        if os.access(bdt_path, os.X_OK):
                            self.inf(f"   Found executable in TELINK_BASE/tools: {bdt_path}")
                            return bdt_path
                else:
                    bdt_path = os.path.join(tools_dir, 'bdt')
                    if os.path.exists(bdt_path) and os.access(bdt_path, os.X_OK):
                        self.inf(f"   Found executable in TELINK_BASE/tools: {bdt_path}")
                        return bdt_path
        
        self.inf("BDT tool not found in any location")
        return None
        
    def _build_bdt_command(self, args, operation_args):
        """Build BDT command line"""
        self.inf("Building BDT command...")
        
        bdt_path = self._find_bdt_path()
        if not bdt_path:
            self.die("BDT tool not found. Please set BDT_PATH environment variable or use --bdt-path option.")
            
        self.inf(f"   BDT tool path: {bdt_path}")
        bdt_cmd = [bdt_path]
        
        self.inf(f"   Platform: {'Windows' if os.name == 'nt' else 'Linux'}")
        self.inf(f"   BDT type: {'Windows BDT (Cmd_download_tool.exe)' if os.name == 'nt' else 'Linux BDT'}")
        
        # Windows BDT requires device ID first
        if os.name == 'nt':
            self.inf(f"   Adding device ID: 1 (default)")
            bdt_cmd.append('1')  # Default to device ID 1
            
        # Add chip type
        if args.chip:
            chip_type = args.chip
            self.inf(f"   Chip type from command line: {chip_type}")
        else:
            # Try to get chip from build configuration
            self.inf(f"   Looking for chip type from build configuration...")
            chip = self._get_chip_from_build()
            if chip:
                chip_type = chip
            else:
                self.die("Chip type not specified. Please use --chip option.")
        
        self.inf(f"   Final chip type: {chip_type}")
        bdt_cmd.append(chip_type)
        
        # Map operations to command codes
        # Note: Windows BDT uses 'lf' for both lock and unlock
        # Linux BDT uses 'lf' for lock and 'ulf' for unlock
        op_map = {
            'download': 'wf' if not args.core else 'wc',
            'read': 'rf' if not args.core else 'rc',
            'write': 'wf' if not args.core else 'wc',
            'erase': 'wf' if not args.core else 'wc',
            'lock': 'lf',
            'unlock': 'lf' if os.name == 'nt' else 'ulf',  # Use 'lf' on Windows, 'ulf' on Linux
            'reset': 'rst' if os.name == 'nt' else 'reset'
        }
        
        # Add operation specific command
        if args.operation in op_map:
            op_code = op_map[args.operation]
            self.inf(f"   Operation: {args.operation} -> Command code: {op_code}")
            bdt_cmd.append(op_code)
        else:
            self.die(f"Unsupported operation: {args.operation}")
        
        # Add operation specific arguments
        self.inf(f"   Adding operation specific arguments...")
        
        if args.operation == 'download':
            self.inf(f"   Address: {args.addr}")
            bdt_cmd.append(args.addr)
            firmware_path = args.input or str(self.build_dir / 'telink' / 'Telink.bin')
            # Convert path for Windows if needed
            if os.name == 'nt':
                original_path = firmware_path
                firmware_path = firmware_path.replace('/', '\\')
                self.inf(f"   Firmware path (converted): {original_path} -> {firmware_path}")
            else:
                self.inf(f"   Firmware path: {firmware_path}")
            bdt_cmd.extend(['-i', firmware_path])
        
        elif args.operation == 'read':
            self.inf(f"   Address: {args.addr}")
            bdt_cmd.append(args.addr)
            self.inf(f"   Size: {args.size}")
            bdt_cmd.extend(['-s', args.size])
            if args.output:
                output_path = args.output
                if os.name == 'nt':
                    original_path = output_path
                    output_path = output_path.replace('/', '\\')
                    self.inf(f"   Output path (converted): {original_path} -> {output_path}")
                else:
                    self.inf(f"   Output path: {output_path}")
                bdt_cmd.extend(['-o', output_path])
        
        elif args.operation == 'write':
            self.inf(f"   Address: {args.addr}")
            bdt_cmd.append(args.addr)
            if args.input:
                input_path = args.input
                if os.name == 'nt':
                    original_path = input_path
                    input_path = input_path.replace('/', '\\')
                    self.inf(f"   Input path (converted): {original_path} -> {input_path}")
                else:
                    self.inf(f"   Input path: {input_path}")
                bdt_cmd.extend(['-i', input_path])
            elif args.data:
                self.inf(f"   Raw data: {' '.join(args.data)}")
                bdt_cmd.extend(args.data)
                self.inf(f"   Size: {args.size}")
                bdt_cmd.extend(['-s', args.size])
            else:
                self.die("Either --input or data bytes must be specified for write operation.")
            if args.erase:
                self.inf(f"   Erase before write: yes")
                bdt_cmd.append('-e')
            else:
                self.inf(f"   Erase before write: no")
        
        elif args.operation == 'erase':
            self.inf(f"   Address: {args.addr}")
            bdt_cmd.append(args.addr)
            self.inf(f"   Size: {args.size}")
            bdt_cmd.extend(['-s', args.size, '-e'])
        
        elif args.operation == 'lock':
            addr = getattr(args, 'addr', '0')
            size = getattr(args, 'size', '0')
            self.inf(f"   Address: {addr}")
            self.inf(f"   Size: {size}")
            bdt_cmd.extend([addr, size])

        elif args.operation == 'unlock':
            self.inf(f"   Unlock all regions")
            # For Linux BDT, unlock doesn't need address and size
            if os.name == 'nt':
                bdt_cmd.extend(['0', '0'])
        
        elif args.operation == 'reset':
            if args.core:
                self.inf(f"   Reset type: core reset")
                bdt_cmd.append('-c' if os.name == 'nt' else '-c')
            else:
                self.inf(f"   Reset type: {'flash reset (-f)' if os.name == 'nt' else 'normal reset'}")
                bdt_cmd.append('-f' if os.name == 'nt' else '')
        
        # Add USB mode
        if args.usb:
            self.inf(f"   USB mode: enabled")
            bdt_cmd.append('-u')
            if args.bus and args.devid:
                self.inf(f"   USB bus: {args.bus}, USB devid: {args.devid}")
                bdt_cmd.extend(['-b', str(args.bus), '-d', str(args.devid)])
        else:
            self.inf(f"   USB mode: disabled (using EVK mode)")
        
        # Add SWS settings
        if args.sws:
            self.inf(f"   SWire settings: {' '.join(args.sws)}")
            bdt_cmd.extend(['sws'] + args.sws)
        
        # Add additional unknown arguments
        if operation_args:
            self.inf(f"   Additional arguments: {' '.join(operation_args)}")
            bdt_cmd.extend(operation_args)
        
        # Remove empty arguments
        original_cmd = bdt_cmd.copy()
        bdt_cmd = [arg for arg in bdt_cmd if arg]
        
        if original_cmd != bdt_cmd:
            self.inf(f"   Removed empty arguments")
        
        full_cmd = ' '.join(bdt_cmd)
        self.inf(f"   Final BDT command: {full_cmd}")
        
        return bdt_cmd
        
    def _get_chip_from_build(self):
        """Get chip type from build configuration"""
        self.inf(f"Looking for chip type in build configuration...")
        
        if not self.build_dir:
            self.inf(f"   Build directory not set")
            return None
            
        self.inf(f"   Build directory: {self.build_dir}")
        
        # Try to read from .config
        config_file = self.build_dir / '.config'
        if config_file.exists():
            self.inf(f"   Checking .config...")
            with open(config_file, 'r') as f:
                for line in f:
                    if line.startswith('CONFIG_TLK_SOC_SERIES='):
                        chip_type = line.split('=')[1].strip()
                        self.inf(f"   Found chip type in .config: {chip_type}")
                        return chip_type
            self.inf(f"   CONFIG_TLK_SOC_SERIES not found in .config")
        else:
            self.inf(f"   .config not found in build directory")

        return None
        
    def do_run(self, args, unknown_args):
        """Run flash operation"""
        self.inf("Starting BDT operation...")
        
        # Print environment information
        self.inf("Environment information:")
        self.inf(f"   OS: {'Windows' if os.name == 'nt' else 'Linux'}")
        self.inf(f"   Python version: {sys.version}")
        self.inf(f"   Current directory: {os.getcwd()}")
        
        # Check relevant environment variables
        self.inf("Relevant environment variables:")
        bdt_env_vars = ['BDT_PATH', 'TELINK_BASE']
        for var in bdt_env_vars:
            if var in os.environ:
                self.inf(f"   {var}: {os.environ[var]}")
            else:
                self.inf(f"   {var}: Not set")
        
        # Set build directory
        if args.build_dir:
            self.build_dir = pathlib.Path(args.build_dir)
            self.inf(f"Build directory from command line: {self.build_dir}")
        else:
            self.build_dir = pathlib.Path.cwd() / 'build'
            self.inf(f"Using default build directory: {self.build_dir}")
            
        # Set BDT path
        self.bdt_path = args.bdt_path
        self.inf(f"BDT path from command line args: {'None' if not self.bdt_path else self.bdt_path}")
        
        # Parse unknown arguments to catch global parameters that might be after subcommand
        if unknown_args:
            self.inf(f"Processing unknown arguments: {' '.join(unknown_args)}")
            i = 0
            while i < len(unknown_args):
                arg = unknown_args[i]
                
                # Handle arguments with equals sign
                if '=' in arg:
                    arg_name, arg_value = arg.split('=', 1)
                    if arg_name == '--bdt-path':
                        self.inf(f"Found --bdt-path in unknown args (with =): {arg_value}")
                        self.bdt_path = arg_value
                        unknown_args.pop(i)
                        continue
                    elif arg_name == '--chip':
                        self.inf(f"Found --chip in unknown args (with =): {arg_value}")
                        args.chip = arg_value
                        unknown_args.pop(i)
                        continue
                # Handle arguments with space separator
                elif arg == '--bdt-path' and i + 1 < len(unknown_args):
                    self.inf(f"Found --bdt-path in unknown args (with space): {unknown_args[i + 1]}")
                    self.bdt_path = unknown_args[i + 1]
                    # Remove the argument from unknown_args so it's not passed to BDT
                    unknown_args.pop(i)
                    unknown_args.pop(i)
                    continue
                elif arg == '--chip' and i + 1 < len(unknown_args):
                    self.inf(f"Found --chip in unknown args (with space): {unknown_args[i + 1]}")
                    args.chip = unknown_args[i + 1]
                    unknown_args.pop(i)
                    unknown_args.pop(i)
                    continue
                i += 1
        
        if self.bdt_path:
            self.inf(f"Final BDT path from command line: {self.bdt_path}")
        else:
            self.inf("No BDT path provided in command line, searching for BDT...")
        
        # Build BDT command
        bdt_cmd = self._build_bdt_command(args, unknown_args)
        
        self.inf(f"Running BDT command: {' '.join(bdt_cmd)}")
        
        # Execute BDT command
        try:
            self.inf("Executing BDT command...")
            result = subprocess.run(bdt_cmd, check=True, capture_output=True, text=True)
            self.inf(f"BDT output:\n{result.stdout}")
            if result.stderr:
                self.inf(f"BDT stderr:\n{result.stderr}")
            self.inf("Flash operation completed successfully")
        except subprocess.CalledProcessError as e:
            self.die(f"BDT command failed with exit code {e.returncode}:\n{e.stdout}\n{e.stderr}")
        except FileNotFoundError:
            self.die(f"BDT tool not found at path: {bdt_cmd[0]}. Please check the path and permissions.")
        except PermissionError:
            self.die(f"Permission denied when trying to execute BDT tool: {bdt_cmd[0]}")
        except Exception as e:
            self.die(f"Error executing BDT command: {str(e)}")

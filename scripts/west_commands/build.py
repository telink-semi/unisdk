import argparse
import os
import pathlib
import sys
import subprocess
from west.commands import WestCommand, Verbosity
from zcmake import CMakeCache
from validation_utils import validate_soc_board

# Add scripts directory to path to import kconfig_parser
script_dir = pathlib.Path(__file__).parent.parent
sys.path.insert(0, str(script_dir))

# Add more descriptive help text
BUILD_USAGE = '''
west tl-build [-h] [-d BUILD_DIR]
           [-t TARGET] [-p {auto, always, never}] [-c] [--cmake-only]
           [-o BUILD_OPT] [--kconfig] [--clean] [--flash] [-b BOARD] [--soc SOC]
           [source_dir] -- [cmake_opt [cmake_opt ...]]'''

BUILD_DESCRIPTION = '''\nConvenience wrapper for building Telink applications.

The build directory can be specified with -d/--build-dir.
If not given, the default is a directory named 'build' in the current
working directory.

positional arguments:
  source_dir            application source directory
  cmake_opt             extra options to pass to cmake; implies -c
                        (these must come after "--" as shown above)'''

class Build(WestCommand):
    """West command for building Telink applications"""
    
    def __init__(self):
        super().__init__(
            'tl-build',
            'compile a Telink application',
            BUILD_DESCRIPTION,
            accepts_unknown_args=True
        )
        self.source_dir = None
        self.build_dir = None
        self.created_build_dir = False
        self.run_cmake = False
        self.cmake_cache = None
    
    def do_add_parser(self, parser_adder):
        """Add command line arguments"""
        parser = parser_adder.add_parser(
            self.name,
            help=self.help,
            description=self.description,
            usage=BUILD_USAGE,
            formatter_class=argparse.RawDescriptionHelpFormatter
        )
        
        # Add standard build options
        parser.add_argument("source_dir", nargs='?', default=os.getcwd(),
                          help="Application source directory (default: current directory)")
        parser.add_argument("-d", "--build-dir",
                          help="Build directory")
        parser.add_argument("-c", "--cmake-only", action="store_true",
                          help="Just run CMake, don't build")
        parser.add_argument("-k", "--kconfig", action="store_true", 
                        help="Run configuration before building")
        parser.add_argument("-t", "--target",
                          help="Build target (e.g. flash, debug, etc.)")
        parser.add_argument("-p", "--pristine", choices=['auto', 'always', 'never'],
                          const='always', nargs='?',
                          help="Control whether to use a pristine build directory")
        parser.add_argument("--clean", action="store_true",
                          help="Clean build directory before building")
        parser.add_argument("--flash", action="store_true",
                          help="Flash firmware after building")
        parser.add_argument("-o", "--build-option", dest="build_options", action="append",
                          help="Extra options to pass to the build system")
        parser.add_argument("--soc", 
                          help="Specify SOC for configuration")
        parser.add_argument("-b", "--board", 
                          help="Specify BOARD for configuration")
        
        return parser
        
    def _run_menuconfig(self):
        """Run CMake config target for interactive configuration"""
        try:
            # Base build arguments
            build_args = [
                'cmake', '--build', str(self.build_dir),
                '--target', 'config'
            ]
            
            # Run CMake config target
            result = subprocess.run(build_args, cwd=self.source_dir)
            if result.returncode:
                self.die(f"Error running cmake config with return code {result.returncode}")
            return result.returncode
        except Exception as e:
            self.die(f"Error running cmake config: {e}")
            return 1
    
    def _validate_soc_board(self, soc, board):
        """Validate if SOC and BOARD combination is valid"""
        return validate_soc_board(soc, board, warn_func=self.wrn)

    def _sanity_check_source_dir(self):
        if self.source_dir == self.build_dir:
            # There's no forcing this.
            self.die(f'source and build directory {self.source_dir} cannot be the same; '
                    f'use --build-dir {self.build_dir} to specify a build directory')

        srcrel = os.path.relpath(self.source_dir)
        if not os.path.isdir(self.source_dir):
            self.die(f'source directory "{srcrel}" does not exist')
        if not 'CMakeLists.txt' in os.listdir(self.source_dir):
            self.die(f'source directory "{srcrel}" does not contain '
            'a CMakeLists.txt; is this really what you '
            'want to build? (Use -s SOURCE_DIR to specify '
            'the application source directory)')
        
    def _sanity_check(self):
        # Sanity check the build configuration.
        # Side effect: may update cmake_cache attribute.
        self.dbg('sanity checking the build', level=Verbosity.DBG_EXTREME)
        self._sanity_check_source_dir()

        if not self.cmake_cache:
            self.run_cmake = True
            return          # That's all we can check without a cache.

        if "CMAKE_PROJECT_NAME" not in self.cmake_cache:
            # This happens sometimes when a build system is not
            # completely generated due to an error during the
            # CMake configuration phase.
            self.run_cmake = True

        cached_proj = self.cmake_cache.get('APPLICATION_SOURCE_DIR')
        cached_app = cached_proj

        self.dbg('APP_DIR:', cached_app, level=Verbosity.DBG_EXTREME)
        source_abs = (os.path.abspath(self.args.source_dir)
                      if self.args.source_dir else None)
        cached_abs = os.path.abspath(cached_app) if cached_app else None

        # If the build directory specifies a source app, make sure it's
        # consistent with --source-dir.
        apps_mismatched = (source_abs and cached_abs and
            pathlib.Path(source_abs).resolve() != pathlib.Path(cached_abs).resolve())

        if apps_mismatched and not self.auto_pristine and not self.args.clean:
            self.die(f'Build directory "{self.build_dir}" is for application "{cached_abs}", but source '
            f'directory "{source_abs}" was specified; please clean it, use --pristine, '
            'or use --build-dir to set another build directory')

        if apps_mismatched:
            self.run_cmake = True

        cached_board = self.cmake_cache.get('BOARD')
        self.dbg('Cached BOARD:', cached_board, level=Verbosity.DBG_EXTREME)
        cached_soc = self.cmake_cache.get('SOC')
        self.dbg('Cached SOC:', cached_soc, level=Verbosity.DBG_EXTREME)

        # Check consistency between cached board and --board.
        boards_mismatched = (self.args.board and cached_board and
                             self.args.board != cached_board)
        socs_mismatched = (self.args.soc and cached_soc and
                             self.args.soc != cached_soc)
        if (boards_mismatched or socs_mismatched) and not self.auto_pristine and not self.args.clean:
            self.die(f'Build directory {self.build_dir} targets {"board" if boards_mismatched else "SOC"} {cached_board if boards_mismatched else cached_soc}, '
            f'but {"SOC" if socs_mismatched else "board"} {self.args.soc if socs_mismatched else self.args.board} was specified. '
            '(Clean the directory, use --pristine, or use --build-dir to '
            'specify a different one.)')

        if (self.auto_pristine or self.args.clean) and (apps_mismatched or boards_mismatched or socs_mismatched):
            self._clean_build()
            self.cmake_cache = None
            self.dbg('run_cmake:', True, level=Verbosity.DBG_EXTREME)
            self.run_cmake = True

    def do_run(self, args, unknown_args):
        self.args = args
        """Execute the build command"""
        # Validate SOC and BOARD combination if both are provided
        if self.args.board or self.args.soc:
            is_valid, error_msg = self._validate_soc_board(self.args.soc, self.args.board)
            if not is_valid:
                self.die(f"Error: {error_msg}")
                return 1

        # Set up paths
        self.source_dir = pathlib.Path(self.args.source_dir).resolve()
        self.inf(f"Build source directory {self.source_dir}")

        # Set build directory
        if self.args.build_dir:
            self.build_dir = pathlib.Path(self.args.build_dir).resolve()
        else:
            self.build_dir = self.source_dir / "build"
        self.inf(f"Build build directory {self.build_dir}")
    
        # Set TELINK_BASE environment variable
        telink_base = pathlib.Path(__file__).resolve().parent.parent.parent
        os.environ['TELINK_BASE'] = str(telink_base)
        self.inf(f"TELINK_BASE environment variable set to {telink_base}")
        
        # Handle pristine build option
        if args.pristine is not None:
            pristine = args.pristine
        else:
            pristine = 'never'

        self.auto_pristine = pristine == 'auto'
        if pristine == 'always':
            self.inf(f"Cleaning build directory {self.build_dir} for pristine build...")
            self._clean_build()
            self.run_cmake = True
        
        # Create build directory if it doesn't exist
        if not self.build_dir.exists():
            self.inf(f"Creating build directory {self.build_dir}")
            self.build_dir.mkdir(parents=True)
            self.created_build_dir = True

        # Only configure CMake if needed
        try:
            self.cmake_cache = CMakeCache.from_build_dir(self.build_dir)
        except FileNotFoundError:
            pass

        self._sanity_check()

        # Configure CMake
        self.inf(f"Configuring build in {self.build_dir}...")
        if self.run_cmake:
            if not self.args.board and not self.args.soc and not self.args.kconfig:
                self.wrn("Neither board nor soc specified, using default values. " \
                "If cmake was already configured, the cached values will be used.")
            if self._configure_cmake(self.args, unknown_args) != 0:
                return 1
        
        # Run menuconfig if requested
        if self.args.kconfig:
            self.inf(f"Running menuconfig for interactive configuration...")
            if self._run_menuconfig() != 0:
                self.die(f"Menuconfig failed or was cancelled in {self.build_dir}")
                return 1

        # Clean build artifacts if requested (weak clean: preserves CMake cache and config)
        if self.args.clean and (self.build_dir / "CMakeCache.txt").exists():
            if self._run_cmake_clean() != 0:
                return 1

        # Skip build if --cmake-only
        if self.args.cmake_only:
            self.inf(f"CMake configuration completed in {self.build_dir}. Skipping build as requested.")
            return 0
        
        # Build project
        self.inf(f"Building project in {self.build_dir}...")
        if self._build_project() != 0:
            return 1
                
        self.inf(f"Build completed successfully in {self.build_dir}!")
        return 0
    
    def _config_exists(self):
        """Check if configuration file exists"""
        return (self.build_dir / ".config").exists()
    
    def _clean_build(self):
        """Clean build directory (strong clean: removes everything including config)"""
        if self.build_dir.exists():
            import shutil
            try:
                shutil.rmtree(self.build_dir)
                self.inf(f"Removed build directory: {self.build_dir}")
            except Exception as e:
                self.die(f"Error cleaning build directory: {e}")
                return 1
        return 0

    def _run_cmake_clean(self):
        """Run cmake clean target (weak clean: preserves CMake cache and config)"""
        try:
            clean_args = ['cmake', '--build', str(self.build_dir), '--target', 'clean']
            result = subprocess.run(clean_args)
            if result.returncode:
                self.die(f"CMake clean failed with return code {result.returncode}")
                return 1
            self.inf(f"Cleaned build artifacts in {self.build_dir}")
            return 0
        except Exception as e:
            self.die(f"CMake clean failed: {e}")
            return 1
    
    def _configure_cmake(self, args, unknown_args):
        """Configure CMake"""
        
        # Base CMake arguments
        cmake_args = [
            'cmake',
            '-B', str(self.build_dir),
            '-S', str(self.source_dir),
            "-G", "Ninja",
        ]
        
        # Add Telink package path
        telink_package_dir = pathlib.Path(os.environ['TELINK_BASE']) / 'cmake'
        cmake_args.extend(['-DTelink_DIR=' + str(telink_package_dir)])
        
        # Add SOC and BOARD parameters if provided
        if hasattr(args, 'soc') and args.soc:
            cmake_args.extend(['-DSOC=' + args.soc])
            self.inf(f"Using SOC: {args.soc}")
            
        if hasattr(args, 'board') and args.board:
            cmake_args.extend(['-DBOARD=' + args.board])
            self.inf(f"Using BOARD: {args.board}")
        
        # Add any unknown arguments (cmake options)
        if unknown_args:
            cmake_args.extend(unknown_args)
        
        # Run CMake
        try:
            result = subprocess.run(cmake_args)
            if result.returncode:
                self.die(f"CMake configuration failed with return code {result.returncode}")
            return result.returncode
        except Exception as e:
            self.die(f"CMake configuration failed: {e}")
            return 1
    
    def _build_project(self):
        """Build the project"""
        # Base build arguments
        build_args = [
            'cmake', '--build', str(self.build_dir)
        ]
        
        # Add target if specified
        if self.args.target:
            build_args.extend(['--target', self.args.target])
            self.inf(f"Building target: {self.args.target}")
        
        # Add build options
        build_env = os.environ.copy()
        if self.args.build_options:
            for option in self.args.build_options:
                if '=' in option:
                    # Handle environment variables like VERBOSE=1
                    key, value = option.split('=', 1)
                    build_env[key] = value
                else:
                    # Handle other build options
                    build_args.extend(['--', option])
        
        # Run build
        try:
            result = subprocess.run(build_args, env=build_env)
            if result.returncode:
                self.die(f"Build failed with return code {result.returncode}")
                return 1
        except Exception as e:
            self.die(f"Build failed: {e}")
            return 1
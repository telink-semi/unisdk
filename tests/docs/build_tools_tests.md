# Build Tools Tests Documentation

## Overview

The build tools test module is used to verify the availability and functionality of various build tools, including CMake, Make, and West.

## Test Modules

### 1. CMake Test (`test_cmake.py`)

#### Functionality
Verifies the availability and basic functionality of the CMake build tool.

#### Test Items
- **CMake Availability Check**: Verifies that CMake is available and the version meets requirements
- **CMake Project Generation Test**: Tests the ability to use CMake to generate and build projects
- **CMake Toolchain File Check**: Verifies that CMake toolchain files exist

#### Configuration
Configure in the `build_tools.cmake` section of `test_config.json`:

```json
{
  "build_tools": {
    "cmake": {
      "min_version": "3.13.0",
      "toolchain_files": [
        "cmake/toolchain-riscv32.cmake",
        "cmake/toolchain-arm.cmake"
      ]
    }
  }
}
```

#### Usage
```python
from tests.build_tools.test_cmake import TestCMake
from tests.config.test_config import load_config

config = load_config("config/test_config.json")
cmake_test = TestCMake(config)
results = cmake_test.run_all_tests()
```

#### Detailed Description

**CMake Availability Check**
- Checks if CMake is installed in the system
- Verifies that the CMake version meets the minimum requirement (default 3.13.0+)
- Uses the `cmake --version` command to get version information

**CMake Project Generation Test**
- Creates a simple CMake test project
- Tests the CMake configuration process
- Tests the CMake build process
- Verifies the generated executable file

**CMake Toolchain File Check**
- Checks if configured toolchain files exist
- Verifies that toolchain file paths are correct
- Supports relative and absolute paths

### 2. Make Test (`test_make.py`)

#### Functionality
Verifies the availability and basic functionality of the Make build tool.

#### Test Items
- **Make Availability Check**: Verifies that Make is available
- **Makefile Syntax Check**: Verifies that the Makefile syntax in the project is correct
- **Simple Makefile Build Test**: Tests the ability to build simple projects using Make

#### Configuration
Configure in the `build_tools.make` section of `test_config.json`:

```json
{
  "build_tools": {
    "make": {
      "makefiles": [
        "Makefile",
        "samples/Makefile",
        "boards/Makefile"
      ]
    }
  }
}
```

#### Usage
```python
from tests.build_tools.test_make import TestMake
from tests.config.test_config import load_config

config = load_config("config/test_config.json")
make_test = TestMake(config)
results = make_test.run_all_tests()
```

#### Detailed Description

**Make Availability Check**
- Checks if Make is installed in the system
- Uses the `make --version` command to get version information

**Makefile Syntax Check**
- Checks if configured Makefiles exist
- Uses the `make -n` command to verify Makefile syntax
- Supports checking multiple Makefiles

**Simple Makefile Build Test**
- Creates a simple Make test project
- Tests the Make build process
- Verifies the generated executable file

### 3. West Test (`test_west.py`)

#### Functionality
Verifies the availability and basic functionality of the West build tool, which is the meta-build tool for the Zephyr project.

#### Test Items
- **West Availability Check**: Verifies that West is available and the version meets requirements
- **West Workspace Check**: Verifies that the West workspace is correctly configured
- **West SOC Support Check**: Verifies that West supports the required SOCs
- **West Board Support Check**: Verifies that West supports the required boards
- **West Config Functionality Test**: Tests the ability to configure projects using West
- **West Build Functionality Test**: Tests the ability to build projects using West
- **West Build Parameter Combinations Test**: Tests build functionality with various parameter combinations

#### Configuration
Configure in the `build_tools.west` section of `test_config.json`:

```json
{
  "build_tools": {
    "west": {
      "min_version": "0.14.0",
      "default_board": "TL7218X_EVK",
      "default_soc": "TL7218X",
      "sample_projects": [
        "tests/compilation/sample_test"
      ],
      "required_boards": [
        "TL7218X_EVK",
      ],
      "required_socs": [
        "TL7218X",
      ]
    }
  }
}
```

#### Usage
```python
from tests.build_tools.test_west import TestWest
from tests.config.test_config import load_config

config = load_config("config/test_config.json")
west_test = TestWest(config)
results = west_test.run_all_tests()
```

#### Detailed Description

**West Availability Check**
- Checks if West is installed in the system
- Verifies that the West version meets the minimum requirement (default 0.14.0+)
- Uses the `west --version` command to get version information

**West Workspace Check**
- Checks if the `.west` directory exists
- Verifies that West configuration files exist
- Uses the `west list` command to check workspace status

**West SOC Support Check**
- Uses the `west tl-socs` command to get all supported SOCs
- Verifies that required SOCs are in the support list
- Supports verbose output with `-v` parameter
- Supports filtering boards by SOC with `--soc` parameter

**West Board Support Check**
- Uses the `west tl-boards` command to get all supported boards
- Verifies that required boards are in the support list
- Supports verbose output with `-v` parameter
- Supports filtering boards by SOC with `--soc` parameter

**West Config Functionality Test**
- Tests `west tl-config` command with various parameter combinations
- Tests listing available SOCs with `--list-socs` parameter
- Tests listing available boards with `--list-boards` parameter
- Tests validating SOC and board combinations with `--validate` parameter
- Tests configuring projects with SOC or board
- Uses `--cmake-only` parameter to skip interactive GUI
- Verifies both exit codes and output content for success/failure

**West Build Functionality Test**
- Selects a sample project for building
- Builds using the specified board
- Uses the `west tl-build` command to build the project
- Verifies that the build process is successful

**West Build Parameter Combinations Test**
- Tests 9 different parameter combinations for `west tl-build`:
  1. Build with board
  2. Build with SOC
  3. Build with both SOC and board
  4. CMake configuration only (--cmake-only)
  5. Pristine build (--pristine)
  6. Clean build (--clean)
  7. Build with specific target
  8. Build with custom options (--build-option)
  9. Build with custom build directory (-d)
- Uses temporary directories for build artifacts
- Verifies both exit codes and output content for success/failure

## Test Results

Each build tool test returns a dictionary containing the following fields:

```json
{
  "name": "Test name",
  "success": true/false,
  "message": "Test result message",
  "details": {
    "Additional information": "..."
  }
}
```

## Troubleshooting

### Common Issues

1. **CMake Version Too Low**
   - Ensure that a CMake version meeting the minimum requirement is installed
   - Can check the current version with `cmake --version`

2. **Make Unavailable**
   - Ensure that the Make tool is installed in the system
   - On Windows, you may need to install MinGW or use WSL

3. **West Unavailable**
   - Install West using pip: `pip install west`
   - Ensure that West is added to the PATH environment variable

4. **West Workspace Not Initialized**
   - Run `west init` in the project root directory to initialize the workspace
   - Ensure that the `.west` directory and configuration files exist

5. **Toolchain Files Do Not Exist**
   - Check if configured toolchain file paths are correct
   - Ensure that toolchain files actually exist at the specified locations

### Debugging Tips

1. View detailed log output
2. Check if configuration files are correct
3. Manually verify failed test items
4. Ensure that build tools are correctly installed and configured

## Extension

If you need to add new build tool tests, you can:

1. Create a new test class inheriting from `BaseTest`
2. Implement the `run_all_tests` method
3. Register the new test class in the test runner
4. Update the configuration file to support new test items

## Best Practices

1. Regularly update build tools to the latest stable versions
2. Maintain consistency of build tool versions
3. Use CI/CD systems to automatically run build tool tests
4. Maintain different configuration files for different development environments
5. Regularly back up build configurations and toolchain files
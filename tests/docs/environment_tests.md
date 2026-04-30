# Environment Tests Documentation

## Overview

The environment test module is used to verify the basic configuration and availability of the development environment, including Python environment, toolchain, and environment variables.

## Test Modules

### 1. Python Environment Test (`test_python.py`)

#### Functionality
Verifies that the Python environment and required Python packages are correctly installed and configured.

#### Test Items
- **Python Version Check**: Verifies that the Python version meets the minimum requirement (default 3.8.0+)
- **Required Packages Check**: Verifies that all required Python packages are installed
- **pip Availability Check**: Verifies that pip is available and functioning properly

#### Configuration
Configure in the `python` section of `test_config.json`:

```json
{
  "python": {
    "min_version": "3.8.0",
    "requirements_file": "requirements.txt"
  }
}
```

#### Usage
```python
from tests.environment.test_python import TestPython
from tests.config.test_config import load_config

config = load_config("config/test_config.json")
python_test = TestPython(config)
results = python_test.run_all_tests()
```

#### Detailed Description

**Python Version Check**
- Verifies that the Python version meets the minimum requirement (default 3.8.0+)
- Uses sys.version_info to get the current version
- Compares versions using semantic versioning rules

**Required Packages Check**
- Loads required packages from requirements.txt file
- Supports platform-specific package filtering
- Extracts package names from version specifiers and extras
- Checks if required packages are installed in the Python environment

### 2. Toolchain Test (`test_toolchain.py`)

#### Functionality
Verifies that the RISC-V toolchain is correctly installed and configured.

#### Test Items
- **GCC Availability Check**: Verifies that the RISC-V GCC compiler is available
- **Required Tools Check**: Verifies that all required toolchain tools are available
- **Simple Compilation Test**: Tests basic C code compilation functionality

#### Configuration
Configure in the `toolchain` section of `test_config.json`:

```json
{
  "toolchain": {
    "riscv_gcc_prefix": "riscv32-elf-",
    "required_tools": [
      "gcc",
      "g++",
      "ar",
      "objcopy",
      "objdump",
      "size"
    ]
  }
}
```

#### Usage
```python
from tests.environment.test_toolchain import TestToolchain
from tests.config.test_config import load_config

config = load_config("config/test_config.json")
toolchain_test = TestToolchain(config)
results = toolchain_test.run_all_tests()
```

#### Detailed Description

**GCC Availability Check**
- Verifies that the RISC-V GCC toolchain is available
- Prioritizes tools from TELINK_TOOLCHAIN_PATH environment variable if set
- Otherwise uses the configured prefix from test_config.json
- Handles platform-specific executable extensions (.exe on Windows)
- Uses --version command to verify tool functionality

**Required Tools Check**
- Verifies that all required toolchain tools are available
- Checks tools like gcc, g++, ar, objcopy, objdump, size
- Handles platform-specific executable extensions
- Uses the same prefix and path logic as GCC availability check

### 3. Environment Variables Test (`test_environment_vars.py`)

#### Functionality
Verifies that required environment variables are correctly set.

#### Test Items
- **Required Environment Variables Check**: Verifies that all required environment variables are set
- **Optional Environment Variables Check**: Verifies optional environment variables and provides defaults
- **Workspace Environment Variables Check**: Verifies workspace-related environment variables

#### Configuration
Configure in the `environment` section of `test_config.json`:

```json
{
  "environment": {
    "required_vars": [
      "TELINK_BASE",
      "TELINK_TOOLCHAIN_PATH"
    ],
    "optional_vars": []
  }
}
```

#### Usage
```python
from tests.environment.test_environment_vars import TestEnvironmentVars
from tests.config.test_config import load_config

config = load_config("config/test_config.json")
env_vars_test = TestEnvironmentVars(config)
results = env_vars_test.run_all_tests()
```

#### Detailed Description

**Required Environment Variables Check**
- Verifies that all required environment variables are set
- Checks variables like TELINK_BASE, TELINK_TOOLCHAIN_PATH
- Reports missing variables with their names
- Logs values of correctly set variables

**Optional Environment Variables Check**
- Verifies optional environment variables
- Reports which optional variables are set and their values
- Does not report errors for missing optional variables

## Base Test Class (`base_test.py`)

All environment test classes inherit from the `BaseTest` base class, which provides the following functionality:

- Logging (with configurable log levels)
- Version comparison (semantic versioning support)
- Test execution and result collection
- Temporary file management (cleanup after tests)
- Test configuration loading

## Test Results

Each test returns a dictionary containing the following fields:

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

1. **Python Version Too Low**
   - Ensure that a Python version meeting the minimum requirement is installed
   - Can check the current version with `python --version`

2. **Missing Python Packages**
   - Install missing packages with pip: `pip install <package_name>`
   - Ensure using the correct Python environment

3. **Toolchain Unavailable**
   - Ensure that the RISC-V toolchain is correctly installed
   - Check if the toolchain path is added to the PATH environment variable

4. **Environment Variables Not Set**
   - Ensure that all required environment variables are correctly set
   - Can set in shell configuration files (such as `.bashrc` or `.zshrc`)

### Debugging Tips

1. View detailed log output
2. Check if the configuration file is correct
3. Manually verify failed test items
4. Ensure using the correct Python environment and toolchain

## Extension

If you need to add new environment tests, you can:

1. Create a new test class inheriting from `BaseTest`
2. Implement the `run_all_tests` method
3. Register the new test class in the test runner
4. Update the configuration file to support new test items
# Compilation Tests Documentation

## Overview

The compilation test module is used to verify the actual compilation process of the project, including basic builds, sample builds, and multi-configuration builds.

## Test Modules

### 1. Basic Build Test (`test_basic_build.py`)

#### Functionality
Verifies the basic build capability of the project, ensuring that the project structure and build system are correctly configured.

#### Test Items
- **Project Structure Check**: Verifies that the basic directory structure of the project is complete
- **Build System Files Check**: Verifies that build system files exist
- **Basic CMake Build Test**: Tests the ability to build the project using CMake
- **Basic Make Build Test**: Tests the ability to build the project using Make
- **Build Artifacts Check**: Verifies that build artifacts are correctly generated

#### Configuration
Configure in the `compilation.basic_build` section of `test_config.json`:

```json
{
  "compilation": {
    "basic_build": {
      "required_dirs": [
        "api",
        "boards",
        "cmake",
        "common",
        "core",
        "docs",
        "samples",
        "scripts",
        "soc",
        "system"
      ],
      "required_files": [
        "CMakeLists.txt",
        "Kconfig.build",
        "Kconfig.chip",
        "Makefile",
        "west.yml"
      ],
      "expected_artifacts": [
        ".config",
        ".pinmux",
        "pinmux.h",
        "autoconf.h",
        "build.config",
        "capabilities.h",
        "chip.config",
        "CMakeCache.txt",
        "flash_boot.ld",
        "merge_config_files.cmake",
        "Kconfig/Kconfig.dma",
        "Kconfig/Kconfig.gpio",
        "Kconfig/Kconfig.plic",
        "Kconfig/Kconfig.pm",
        "Kconfig/Kconfig.rf",
        "Kconfig/Kconfig.spi",
        "Kconfig/Kconfig.uart",
        "Kconfig/Kconfig.wdt",
        "Telink.elf",
        "Telink.bin",
        "Telink.lst",
        "Telink.map",
        "compile_commands.json"
      ],
      "test_sample_dir": "tests/compilation/sample_test"
    }
  }
}
```

#### Usage
```python
from tests.compilation.test_basic_build import TestBasicBuild
from tests.config.test_config import load_config

config = load_config("config/test_config.json")
basic_build_test = TestBasicBuild(config)
results = basic_build_test.run_all_tests()
```

#### Detailed Description

**Project Structure Check**
- Verifies that the basic directory structure of the project is complete
- Checks if required directories exist
- Supports relative paths

**Build System Files Check**
- Verifies that build system files exist
- Checks build files such as CMakeLists.txt, Makefile, Kconfig files, etc.
- Supports checking multiple files

**Basic CMake Build Test**
- Uses CMake to configure the project
- Uses Ninja as build generator
- Uses CMake to build the project
- Uses temporary directories for build artifacts
- Verifies that the build process is successful

**Basic Make Build Test**
- Uses Make to build the project
- Generates build files with `make cmake` command
- Builds using `make build` command
- Uses temporary directories for build artifacts
- Verifies that the build process is successful

**Build Artifacts Check**
- Checks if expected build artifacts exist
- Verifies the completeness of build artifacts
- Supports both root and subdirectory artifacts

### 2. Sample Build Test (`test_samples_build.py`)

#### Functionality
Verifies the build capability of project samples, ensuring that sample projects can be built correctly.

#### Test Items
- **Sample Projects Check**: Verifies that sample projects exist
- **Sample Build Matrix Test**: Builds sample projects according to the test matrix

#### Configuration
Configure in the `compilation.samples_build` section of `test_config.json`:

```json
{
  "compilation": {
    "samples_build": {
      "sample_dirs": [
        "samples"
      ],
      "sample_projects": [
        "samples/gpio",
        "samples/pm",
        "samples/uart"
      ],
      "expected_artifacts": [
        "build/*.elf",
        "build/*.bin"
      ]
    }
  }
}
```

#### Test Matrix Configuration
Configure the sample build matrix in `test_matrix.json` to match CI script (`ci_build_all.sh`):

```json
{
  "test_combinations": [
    {
      "soc": "TL3218X",
      "board": "TL3218X_EVK",
      "samples": [
        {
          "name": "gpio",
          "configs": ["min", "suspend", "retention", "max", "max_all"]
        },
        {
          "name": "pm",
          "configs": ["min", "suspend", "retention", "max", "max_all"]
        },
        {
          "name": "uart",
          "configs": ["min", "suspend", "retention", "max", "max_all"]
        }
      ]
    }
  ]
}
```

#### Usage
```python
from tests.compilation.test_samples_build import TestSamplesBuild
from tests.config.test_config import load_config

config = load_config("config/test_config.json")
samples_build_test = TestSamplesBuild(config)
results = samples_build_test.run_all_tests()
```

#### Detailed Description

**Sample Projects Check**
- Verifies that sample directories exist
- Checks the completeness of sample projects
- Supports checking multiple sample directories

**Sample Build Matrix Test**
- Reads build combinations from the test matrix
- Tests combinations of different SOCs, boards, and configurations
- Uses `west tl-build` command for building
- Limits test scope to avoid long execution times
- Supports timeout control

## Test Results

Each compilation test returns a dictionary containing the following fields:

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

## Test Reports

Compilation tests generate detailed test reports, including:

1. **Overall Test Results**
   - Number of passed/failed tests
   - Overall success rate

2. **Detailed Test Results**
   - Detailed information for each test
   - Error information for failed tests

3. **Build Statistics**
   - Build time statistics
   - Build artifact size statistics
   - Build success rate statistics

## Troubleshooting

### Common Issues

1. **Incomplete Project Structure**
   - Check if required directories exist
   - Ensure the project structure meets requirements

2. **Missing Build System Files**
   - Check build files such as CMakeLists.txt, Makefile, etc.
   - Ensure build file syntax is correct

3. **Build Failures**
   - Check error information in build logs
   - Ensure toolchain and dependencies are correctly installed
   - Verify that build configurations are correct

4. **Missing Build Artifacts**
   - Check if the build process is complete
   - Verify that build artifact paths are correct
   - Ensure build artifact names match

5. **Multi-Configuration Build Failures**
   - Check if configuration files are correct
   - Verify that configuration combinations are valid
   - Ensure boards and configurations match

### Debugging Tips

1. View detailed build logs
2. Check if configuration files are correct
3. Manually verify failed builds
4. Use incremental builds to reduce build time
5. Use parallel builds to speed up the build process

## Extension

If you need to add new compilation tests, you can:

1. Create a new test class inheriting from `BaseTest`
2. Implement the `run_all_tests` method
3. Register the new test class in the test runner
4. Update the configuration file to support new test items

## Best Practices

1. Run full compilation tests regularly
2. Use CI/CD systems to automatically run compilation tests
3. Maintain different configurations for different development environments
4. Optimize the build process to reduce test time
5. Clean up build artifacts regularly to save space
6. Use incremental builds to improve test efficiency
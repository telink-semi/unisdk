#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import shutil
import subprocess
import tempfile
from typing import Dict, Any, List, Tuple

# Add current directory to Python path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from environment.base_test import BaseTest


class TestCMake(BaseTest):
    """CMake build tool test"""
    
    def __init__(self, config: Dict[str, Any]):
        super().__init__(config)
        self.cmake_config = config.get("build_tools", {}).get("cmake", {})
    
    def test_cmake_availability(self) -> Tuple[bool, str]:
        """Test if CMake is available"""
        self.logger.info("Checking CMake availability...")
        
        min_version = self.cmake_config.get("min_version", "3.13.0")
        
        if not shutil.which("cmake"):
            error_msg = "CMake command not found"
            self.logger.error(error_msg)
            return False, error_msg
        
        try:
            result = subprocess.run(
                ["cmake", "--version"],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if result.returncode == 0:
                # Extract version information
                version_line = result.stdout.split('\n')[0]
                version_str = version_line.split()[-1]
                
                if self._compare_versions(version_str, min_version) >= 0:
                    self.logger.info(f"CMake version check passed: {version_str} >= {min_version}")
                    return True, f"CMake version: {version_str}"
                else:
                    error_msg = f"CMake version too low: {version_str} < {min_version}"
                    self.logger.error(error_msg)
                    return False, error_msg
            else:
                error_msg = f"CMake not available: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
        except subprocess.TimeoutExpired:
            error_msg = "CMake command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error checking CMake: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def test_cmake_project_generation(self) -> Tuple[bool, str]:
        """Test CMake project generation"""
        self.logger.info("Testing CMake project generation...")
        
        # Check toolchain path
        toolchain_path = os.environ.get("TELINK_TOOLCHAIN_PATH")
        if not toolchain_path:
            error_msg = "TELINK_TOOLCHAIN_PATH environment variable not set"
            self.logger.error(error_msg)
            return False, error_msg
        
        # Determine executable file extension
        import platform
        exe_ext = ".exe" if platform.system() == "Windows" else ""
        
        # Set compiler paths
        cc_path = os.path.join(toolchain_path, "bin", f"riscv32-elf-gcc{exe_ext}")
        cxx_path = os.path.join(toolchain_path, "bin", f"riscv32-elf-g++{exe_ext}")
        
        # Check if compilers exist
        if not os.path.exists(cc_path):
            error_msg = f"C compiler not found: {cc_path}"
            self.logger.error(error_msg)
            return False, error_msg
        
        if not os.path.exists(cxx_path):
            error_msg = f"C++ compiler not found: {cxx_path}"
            self.logger.error(error_msg)
            return False, error_msg
        
        # Create temporary CMake project
        temp_project_dir = os.path.join(self.temp_dir, "cmake_test_project")
        build_dir = os.path.join(temp_project_dir, "build")
        
        try:
            os.makedirs(temp_project_dir)
            os.makedirs(build_dir)
            
            # Create CMakeLists.txt
            cmake_content = """
cmake_minimum_required(VERSION 3.13.0)
project(cmake_test_project)

# Set C standard
set(CMAKE_C_STANDARD 99)

# Add executable file
add_executable(test_app main.c)

# Set compile options
target_compile_options(test_app PRIVATE
    -Wall
    -Wextra
    -O2
)
"""
            
            with open(os.path.join(temp_project_dir, "CMakeLists.txt"), "w") as f:
                f.write(cmake_content)
            
            # Create main.c
            main_c_content = """
#include <stdio.h>

int main(void) {
    printf("Hello from CMake test project!\\n");
    return 0;
}
"""
            
            with open(os.path.join(temp_project_dir, "main.c"), "w") as f:
                f.write(main_c_content)
            
            # Run CMake configuration, specify compiler paths and cross-compilation options
            cmake_configure_cmd = [
                "cmake",
                "-S", temp_project_dir,
                "-B", build_dir,
                "-G", "Ninja",
                f"-DCMAKE_C_COMPILER={cc_path}",
                f"-DCMAKE_CXX_COMPILER={cxx_path}",
                "-DCMAKE_SYSTEM_NAME=Generic",
                "-DCMAKE_SYSTEM_PROCESSOR=riscv32",
                "-DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY"
            ]
            
            result = subprocess.run(
                cmake_configure_cmd,
                capture_output=True,
                text=True,
                timeout=30
            )
            
            if result.returncode != 0:
                error_msg = f"CMake configuration failed: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
            
            # Run CMake build
            cmake_build_cmd = [
                "cmake",
                "--build", build_dir
            ]
            
            result = subprocess.run(
                cmake_build_cmd,
                capture_output=True,
                text=True,
                timeout=30
            )
            
            if result.returncode == 0:
                success_msg = "CMake project generation and build test passed"
                self.logger.info(success_msg)
                return True, success_msg
            else:
                error_msg = f"CMake build failed: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
        except subprocess.TimeoutExpired:
            error_msg = "CMake command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error during CMake project generation test: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg

    def run_all_tests(self) -> List[Dict[str, Any]]:
        """Run all CMake tests"""
        results = []
        
        # Test CMake availability
        result = self.run_test("cmake_availability", self.test_cmake_availability)
        results.append(result)
        
        # Test CMake project generation
        result = self.run_test("cmake_project_generation", self.test_cmake_project_generation)
        results.append(result)
        
        return results
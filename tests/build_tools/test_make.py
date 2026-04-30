#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import shutil
import subprocess
from typing import Dict, Any, List, Tuple

# Add current directory to Python path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from environment.base_test import BaseTest


class TestMake(BaseTest):
    """Make build tool test"""
    
    def __init__(self, config: Dict[str, Any]):
        super().__init__(config)
        self.make_config = config.get("build_tools", {}).get("make", {})
    
    def test_make_availability(self) -> Tuple[bool, str]:
        """Test if Make is available"""
        self.logger.info("Checking Make availability...")
        
        if not shutil.which("make"):
            error_msg = "Make command not found"
            self.logger.error(error_msg)
            return False, error_msg
        
        try:
            result = subprocess.run(
                ["make", "--version"],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if result.returncode == 0:
                # Extract version information
                version_line = result.stdout.split('\n')[0]
                self.logger.info(f"Make available: {version_line}")
                return True, version_line
            else:
                error_msg = f"Make not available: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
        except subprocess.TimeoutExpired:
            error_msg = "Make command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error checking Make: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def test_makefile_syntax(self) -> Tuple[bool, str]:
        """Test Makefile syntax"""
        self.logger.info("Testing Makefile syntax...")
        
        project_root = self.config.get("project_root", "../")
        makefiles = self.make_config.get("makefiles", [])
        
        if not makefiles:
            self.logger.info("No Makefile configured, skipping test")
            return True, "No Makefile configured, skipping test"
        
        valid_makefiles = []
        
        for makefile in makefiles:
            # If path is relative, make it relative to project root
            if not os.path.isabs(makefile):
                full_path = os.path.join(project_root, makefile)
            else:
                full_path = makefile
            
            if os.path.exists(full_path):
                try:
                    # Use make -n to check Makefile syntax
                    result = subprocess.run(
                        ["make", "-n", "-f", full_path],
                        capture_output=True,
                        text=True,
                        timeout=10,
                        cwd=os.path.dirname(full_path)
                    )
                    
                    if result.returncode == 0:
                        valid_makefiles.append(full_path)
                        self.logger.info(f"Makefile syntax check passed: {full_path}")
                    else:
                        self.logger.warning(f"Makefile syntax check failed: {full_path} - {result.stderr}")
                except subprocess.TimeoutExpired:
                    self.logger.warning(f"Makefile syntax check timeout: {full_path}")
                except Exception as e:
                    self.logger.warning(f"Error checking Makefile: {full_path} - {str(e)}")
            else:
                self.logger.warning(f"Makefile not found: {full_path}")
        
        if not valid_makefiles:
            error_msg = "No valid Makefile found"
            self.logger.error(error_msg)
            return False, error_msg
        
        success_msg = f"Found {len(valid_makefiles)} valid Makefiles"
        self.logger.info(success_msg)
        return True, success_msg
    
    def test_simple_makefile(self) -> Tuple[bool, str]:
        """Test simple Makefile build"""
        self.logger.info("Testing simple Makefile build...")
        
        # Check toolchain path
        toolchain_path = os.environ.get("TELINK_TOOLCHAIN_PATH")
        if not toolchain_path:
            error_msg = "TELINK_TOOLCHAIN_PATH environment variable not set"
            self.logger.error(error_msg)
            return False, error_msg
        
        # Determine executable file extension
        import platform
        exe_ext = ".exe" if platform.system() == "Windows" else ""
        
        # Set compiler path
        cc_path = os.path.join(toolchain_path, "bin", f"riscv32-elf-gcc{exe_ext}")
        
        # Check if compiler exists
        if not os.path.exists(cc_path):
            error_msg = f"C compiler not found: {cc_path}"
            self.logger.error(error_msg)
            return False, error_msg
        
        # Create temporary Makefile project
        temp_project_dir = os.path.join(self.temp_dir, "make_test_project")
        
        try:
            os.makedirs(temp_project_dir)
            
            # Create Makefile
            makefile_content = f"""
CC = {cc_path}
CFLAGS = -Wall -Wextra -O2
TARGET = test_app
SOURCES = main.c

all: $(TARGET)

$(TARGET): $(SOURCES)
\t$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
\trm -f $(TARGET)

.PHONY: all clean
"""
            
            with open(os.path.join(temp_project_dir, "Makefile"), "w") as f:
                f.write(makefile_content)
            
            # Create main.c
            main_c_content = """
#include <stdio.h>

int main(void) {
    printf("Hello from Make test project!\\n");
    return 0;
}
"""
            
            with open(os.path.join(temp_project_dir, "main.c"), "w") as f:
                f.write(main_c_content)
            
            # Run make build
            make_cmd = ["make"]
            
            result = subprocess.run(
                make_cmd,
                capture_output=True,
                text=True,
                timeout=30,
                cwd=temp_project_dir
            )
            
            if result.returncode == 0:
                # Check generated executable file
                target_path = os.path.join(temp_project_dir, "test_app")
                if os.path.exists(target_path):
                    success_msg = "Simple Makefile build test passed"
                    self.logger.info(success_msg)
                    return True, success_msg
                else:
                    error_msg = "Make build succeeded but target file not found"
                    self.logger.error(error_msg)
                    return False, error_msg
            else:
                error_msg = f"Make build failed: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
        except subprocess.TimeoutExpired:
            error_msg = "Make command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error during simple Makefile build test: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def run_all_tests(self) -> List[Dict[str, Any]]:
        """Run all Make tests"""
        results = []
        
        # Test Make availability
        result = self.run_test("make_availability", self.test_make_availability)
        results.append(result)
        
        # Test Makefile syntax
        result = self.run_test("makefile_syntax", self.test_makefile_syntax)
        results.append(result)
        
        # Test simple Makefile build
        result = self.run_test("simple_makefile", self.test_simple_makefile)
        results.append(result)
        
        return results
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import shutil
import subprocess
import platform
from typing import Dict, Any, List, Tuple

# Add current directory to Python path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from environment.base_test import BaseTest


class TestToolchain(BaseTest):
    """Toolchain test"""
    
    def __init__(self, config: Dict[str, Any]):
        super().__init__(config)
        self.toolchain_config = config.get("toolchain", {})
        
        # First check if toolchain path is specified in environment variables
        self.toolchain_path = os.environ.get("TELINK_TOOLCHAIN_PATH")
        
        # Determine executable file extension
        self.exe_ext = ".exe" if platform.system() == "Windows" else ""
        
        if self.toolchain_path:
            # If there's an environment variable, use riscv32-elf- prefix
            self.gcc_prefix = "riscv32-elf-"
            self.toolchain_bin_path = os.path.join(self.toolchain_path, "bin")
        else:
            # Otherwise use the prefix from the configuration file
            self.gcc_prefix = self.toolchain_config.get("riscv_gcc_prefix", "riscv32-elf-")
            self.toolchain_bin_path = None
    
    def test_gcc_availability(self) -> Tuple[bool, str]:
        """Test RISC-V GCC toolchain availability"""
        self.logger.info("Checking RISC-V GCC toolchain availability...")
        
        gcc_cmd = f"{self.gcc_prefix}gcc{self.exe_ext}"
        
        # If there's a toolchain path, prioritize using tools from that path
        if self.toolchain_bin_path:
            gcc_full_path = os.path.join(self.toolchain_bin_path, gcc_cmd)
            if not os.path.exists(gcc_full_path):
                error_msg = f"RISC-V GCC compiler not found: {gcc_full_path}"
                self.logger.error(error_msg)
                return False, error_msg
            gcc_cmd = gcc_full_path
        else:
            # Otherwise use tools from the system PATH
            if not shutil.which(gcc_cmd):
                error_msg = f"RISC-V GCC compiler not found: {gcc_cmd}"
                self.logger.error(error_msg)
                return False, error_msg
        
        try:
            result = subprocess.run(
                [gcc_cmd, "--version"],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if result.returncode == 0:
                version_info = result.stdout.split('\n')[0]
                self.logger.info(f"RISC-V GCC available: {version_info}")
                return True, version_info
            else:
                error_msg = f"RISC-V GCC not available: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
        except subprocess.TimeoutExpired:
            error_msg = "RISC-V GCC command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error checking RISC-V GCC: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def test_required_tools(self) -> Tuple[bool, str]:
        """Test required toolchain tools"""
        self.logger.info("Checking required toolchain tools...")
        
        required_tools = self.toolchain_config.get("required_tools", [])
        missing_tools = []
        
        for tool in required_tools:
            tool_cmd = f"{self.gcc_prefix}{tool}{self.exe_ext}"
            
            # If there's a toolchain path, prioritize using tools from that path
            if self.toolchain_bin_path:
                tool_full_path = os.path.join(self.toolchain_bin_path, tool_cmd)
                if not os.path.exists(tool_full_path):
                    missing_tools.append(tool_cmd)
                    self.logger.warning(f"Tool not found: {tool_full_path}")
                else:
                    self.logger.info(f"Tool available: {tool_full_path}")
            else:
                # Otherwise use tools from the system PATH
                if not shutil.which(tool_cmd):
                    missing_tools.append(tool_cmd)
                    self.logger.warning(f"Tool not found: {tool_cmd}")
                else:
                    self.logger.info(f"Tool available: {tool_cmd}")
        
        if missing_tools:
            error_msg = f"Missing toolchain tools: {', '.join(missing_tools)}"
            self.logger.error(error_msg)
            return False, error_msg
        else:
            success_msg = f"All required toolchain tools available: {', '.join([f'{self.gcc_prefix}{tool}' for tool in required_tools])}"
            self.logger.info(success_msg)
            return True, success_msg
    
    def test_simple_compilation(self) -> Tuple[bool, str]:
        """Test simple compilation functionality"""
        self.logger.info("Testing simple compilation functionality...")
        
        gcc_cmd = f"{self.gcc_prefix}gcc{self.exe_ext}"
        
        # If there's a toolchain path, prioritize using tools from that path
        if self.toolchain_bin_path:
            gcc_full_path = os.path.join(self.toolchain_bin_path, gcc_cmd)
            if not os.path.exists(gcc_full_path):
                error_msg = f"RISC-V GCC compiler not found: {gcc_full_path}"
                self.logger.error(error_msg)
                return False, error_msg
            gcc_cmd = gcc_full_path
        
        # Create temporary C file
        temp_c_file = os.path.join(self.temp_dir, "test_simple.c")
        temp_elf_file = os.path.join(self.temp_dir, "test_simple.elf")
        
        try:
            with open(temp_c_file, "w") as f:
                f.write("""
#include <stdio.h>

int main(void) {
    printf("Hello from RISC-V!\\n");
    return 0;
}
""")
            
            # Compile C file, use -c flag to compile only without linking to avoid link errors
            compile_cmd = [
                gcc_cmd,
                "-c",
                "-nostartfiles",
                "-nodefaultlibs",
                "-nostdlib",
                "-march=rv32imac",
                "-mabi=ilp32",
                "-mcmodel=medany",
                "-mstrict-align",
                "-ffunction-sections",
                "-fdata-sections",
                "-g",
                "-O2",
                temp_c_file,
                "-o",
                temp_elf_file
            ]
            
            result = subprocess.run(
                compile_cmd,
                capture_output=True,
                text=True,
                timeout=30
            )
            
            if result.returncode == 0 and os.path.exists(temp_elf_file):
                file_size = os.path.getsize(temp_elf_file)
                success_msg = f"Simple compilation test passed, generated ELF file size: {file_size} bytes"
                self.logger.info(success_msg)
                return True, success_msg
            else:
                error_msg = f"Simple compilation test failed: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
        except subprocess.TimeoutExpired:
            error_msg = "Compilation command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error during compilation test: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg
        finally:
            # Clean up temporary files
            for file_path in [temp_c_file, temp_elf_file]:
                if os.path.exists(file_path):
                    os.remove(file_path)
    
    def run_all_tests(self) -> List[Dict[str, Any]]:
        """Run all toolchain tests"""
        results = []
        
        # Test GCC availability
        result = self.run_test("gcc_availability", self.test_gcc_availability)
        results.append(result)
        
        # Test required tools
        result = self.run_test("required_tools", self.test_required_tools)
        results.append(result)
        
        # Test simple compilation
        result = self.run_test("simple_compilation", self.test_simple_compilation)
        results.append(result)
        
        return results
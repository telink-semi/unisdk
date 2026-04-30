#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import shutil
import subprocess
import json
from typing import Dict, Any, List, Tuple

# Add current directory to Python path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from environment.base_test import BaseTest


class TestBasicBuild(BaseTest):
    """Basic build test"""
    
    def __init__(self, config: Dict[str, Any]):
        super().__init__(config)
        self.build_config = config.get("compilation", {}).get("basic_build", {})
    
    def test_project_structure(self) -> Tuple[bool, str]:
        """Test project structure"""
        self.logger.info("Checking project structure...")
        
        project_root = self.config.get("project_root", "../")
        required_dirs = self.build_config.get("required_dirs", [])
        
        if not required_dirs:
            self.logger.info("No required directories configured, skipping test")
            return True, "No required directories configured, skipping test"
        
        missing_dirs = []
        found_dirs = []
        
        for dir_path in required_dirs:
            full_path = os.path.join(project_root, dir_path)
            if os.path.exists(full_path) and os.path.isdir(full_path):
                found_dirs.append(dir_path)
                self.logger.info(f"Found directory: {dir_path}")
            else:
                missing_dirs.append(dir_path)
                self.logger.warning(f"Directory not found: {dir_path}")
        
        if missing_dirs:
            error_msg = f"Missing required directories: {', '.join(missing_dirs)}"
            self.logger.error(error_msg)
            return False, error_msg
        
        success_msg = f"All required directories exist: {', '.join(found_dirs)}"
        self.logger.info(success_msg)
        return True, success_msg
    
    def test_build_system_files(self) -> Tuple[bool, str]:
        """Test build system files"""
        self.logger.info("Checking build system files...")
        
        project_root = self.config.get("project_root", "../")
        required_files = self.build_config.get("required_files", [])
        
        if not required_files:
            self.logger.info("No required files configured, skipping test")
            return True, "No required files configured, skipping test"
        
        missing_files = []
        found_files = []
        
        for file_path in required_files:
            full_path = os.path.join(project_root, file_path)
            if os.path.exists(full_path) and os.path.isfile(full_path):
                found_files.append(file_path)
                self.logger.info(f"Found file: {file_path}")
            else:
                missing_files.append(file_path)
                self.logger.warning(f"File not found: {file_path}")
        
        if missing_files:
            error_msg = f"Missing required files: {', '.join(missing_files)}"
            self.logger.error(error_msg)
            return False, error_msg
        
        success_msg = f"All required files exist: {', '.join(found_files)}"
        self.logger.info(success_msg)
        return True, success_msg
    
    def test_basic_cmake_build(self) -> Tuple[bool, str]:
        """Test basic CMake build"""
        self.logger.info("Testing basic CMake build...")
        
        project_root = self.config.get("project_root", "../")
        build_dir = os.path.join(self.temp_dir, "cmake_build")
        test_sample_dir = self.build_config.get("test_sample_dir", "")

        try:
            os.makedirs(build_dir)
            
            # Run CMake configuration
            cmake_configure_cmd = [
                "cmake",
                "-S", os.path.join(project_root, test_sample_dir),
                "-B", build_dir,
                "-G", "Ninja",
            ]
            
            result = subprocess.run(
                cmake_configure_cmd,
                capture_output=True,
                text=True,
                timeout=60
            )
            
            if result.returncode != 0:
                error_msg = f"CMake configuration failed:\nSTDOUT: {result.stdout}\nSTDERR: {result.stderr}"
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
                timeout=120
            )
            
            if result.returncode == 0:
                result, msg = self.test_build_artifacts(build_dir)
                if not result:
                    return False, msg
                success_msg = "Basic CMake build test passed"
                self.logger.info(success_msg)
                return True, success_msg
            else:
                error_msg = f"CMake build failed:\nSTDOUT: {result.stdout}\nSTDERR: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
        except subprocess.TimeoutExpired:
            error_msg = "CMake command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error during basic CMake build test: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def test_basic_make_build(self) -> Tuple[bool, str]:
        """Test basic Make build"""
        self.logger.info("Testing basic Make build...")
        
        project_root = self.config.get("project_root", "../")
        # Use temporary directory for build
        build_dir = os.path.join(self.temp_dir, "make_build")
        test_sample_dir = self.build_config.get("test_sample_dir", ".")

        try:
            # First run cmake to generate build files
            self.logger.info(f"Running 'make cmake' to generate build files in {build_dir}...")
            make_cmake_cmd = ["make", "cmake", "APP="+test_sample_dir, f"BUILD_DIR={build_dir}"]
            result = subprocess.run(
                make_cmake_cmd,
                capture_output=True,
                text=True,
                timeout=120,
                cwd=project_root
            )
            
            if result.returncode != 0:
                    error_msg = f"Make cmake command failed with return code {result.returncode}"
                    self.logger.error(error_msg)
                    self.logger.error(f"STDOUT: {result.stdout}")
                    self.logger.error(f"STDERR: {result.stderr}")
                    return False, error_msg
            
            self.logger.info("Build files generated successfully, now running 'make build' to build...")
            
            # Then run make build to execute the build
            make_build_cmd = ["make", "build", f"BUILD_DIR={build_dir}"]
            
            result = subprocess.run(
                make_build_cmd,
                capture_output=True,
                text=True,
                timeout=120,
                cwd=project_root
            )
            if result.returncode == 0:
                result, msg = self.test_build_artifacts(build_dir)
                if not result:
                    return False, msg
                success_msg = "Basic Make build test passed"
                self.logger.info(success_msg)
                return True, success_msg
            else:
                error_msg = f"Make build failed:\nSTDOUT: {result.stdout}\nSTDERR: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg

        except subprocess.TimeoutExpired:
            error_msg = "Make command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error during basic Make build test: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def test_basic_west_build(self) -> Tuple[bool, str]:
        """Test basic West build"""
        self.logger.info("Testing basic West build...")
        
        project_root = self.config.get("project_root", "../")
        # Use temporary directory for build
        build_dir = os.path.join(self.temp_dir, "west_build")
        test_sample_dir = self.build_config.get("test_sample_dir", ".")
        
        try:
            # First run west tl-build to build
            self.logger.info(f"Running 'west tl-build' to build in {build_dir}...")
            west_build_cmd = ["west", "tl-build", test_sample_dir, "-d", build_dir]
            
            result = subprocess.run(
                west_build_cmd,
                capture_output=True,
                text=True,
                timeout=120,
                cwd=project_root
            )
            
            if result.returncode == 0:
                result, msg = self.test_build_artifacts(build_dir)
                if not result:
                    return False, msg
                success_msg = "Basic West build test passed"
                self.logger.info(success_msg)
                return True, success_msg
            else:
                error_msg = f"West build failed:\nSTDOUT: {result.stdout}\nSTDERR: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg

        except subprocess.TimeoutExpired:
            error_msg = "West command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error during basic West build test: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def test_build_artifacts(self, build_dir) -> Tuple[bool, str]:
        """Test build artifacts"""
        self.logger.info("Checking build artifacts...")

        expected_artifacts = self.build_config.get("expected_artifacts", [])
        
        if not expected_artifacts:
            self.logger.info("No expected build artifacts configured, skipping test")
            return True, "No expected build artifacts configured, skipping test"
        
        # Check if build directory exists and has files
        if not os.path.exists(build_dir):
            error_msg = f"Build directory does not exist: {build_dir}"
            self.logger.error(error_msg)
            return False, error_msg
        
        # List all files in the build directory
        found_files = []
        for root, dirs, files in os.walk(build_dir):
            for file in files:
                file_path = os.path.join(root, file)
                rel_path = os.path.relpath(file_path, build_dir)
                # Normalize path separators to use forward slashes for comparison
                normalized_path = rel_path.replace(os.sep, '/')
                found_files.append(normalized_path)
        
        if not found_files:
            error_msg = f"No files found in build directory: {build_dir}"
            self.logger.error(error_msg)
            return False, error_msg
        
        # Check for build artifacts that should be generated
        missing_artifacts = []
        found_artifacts = []
        
        for artifact in expected_artifacts:
            if artifact in found_files:
                found_artifacts.append(artifact)
                self.logger.info(f"Found build artifact: {artifact}")
            else:
                missing_artifacts.append(artifact)
                self.logger.warning(f"build artifact not found: {artifact}")
        
        # If all expected artifacts are found, consider it a success
        if found_files and (len(found_artifacts) == len(expected_artifacts)):
            success_msg = f"Build artifacts check passed. Found {len(found_files)} files including build artifacts: {', '.join(found_artifacts)}"
            self.logger.info(success_msg)
            return True, success_msg
        else:
            error_msg = f"Build artifacts check failed. Found {len(found_files)} files but missing key artifacts: {', '.join(missing_artifacts)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def run_all_tests(self) -> List[Dict[str, Any]]:
        """Run all basic build tests"""
        results = []
        
        # Test project structure
        result = self.run_test("project_structure", self.test_project_structure)
        results.append(result)
        
        # Test build system files
        result = self.run_test("build_system_files", self.test_build_system_files)
        results.append(result)
        
        # Test basic CMake build
        result = self.run_test("basic_cmake_build", self.test_basic_cmake_build)
        results.append(result)
        
        # Test basic Make build
        result = self.run_test("basic_make_build", self.test_basic_make_build)
        results.append(result)
        
        # Test basic West build
        result = self.run_test("basic_west_build", self.test_basic_west_build)
        results.append(result)
        
        return results
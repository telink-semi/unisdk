#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import subprocess
import importlib.util
from typing import Dict, Any, List, Tuple

# Add current directory to Python path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from environment.base_test import BaseTest


class TestPython(BaseTest):
    """Python environment test"""
    
    def __init__(self, config: Dict[str, Any]):
        super().__init__(config)
        self.python_config = config.get("python", {})
    
    def test_python_version(self) -> Tuple[bool, str]:
        """Test Python version"""
        self.logger.info("Checking Python version...")
        
        min_version = self.python_config.get("min_version", "3.8.0")
        current_version = f"{sys.version_info.major}.{sys.version_info.minor}.{sys.version_info.micro}"
        
        if self._compare_versions(current_version, min_version) >= 0:
            self.logger.info(f"Python version check passed: {current_version} >= {min_version}")
            return True, f"Python version: {current_version}"
        else:
            error_msg = f"Python version too low: {current_version} < {min_version}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def _load_requirements(self) -> List[str]:
        """Load required packages from requirements.txt file"""
        requirements_file = self.python_config.get("requirements_file")
        if not requirements_file:
            self.logger.warning("No requirements file specified in configuration")
            return []
        
        # Convert relative path to absolute path based on project_root
        if not os.path.isabs(requirements_file):
            project_root = self.config.get("project_root", "../")
            # If project_root is already an absolute path, use it directly
            if not os.path.isabs(project_root):
                # Get the directory where the config file is located
                config_dir = os.path.dirname(os.path.abspath(self.config.get("_config_file_path", os.getcwd())))
                project_root = os.path.abspath(os.path.join(config_dir, project_root))
            requirements_file = os.path.join(project_root, requirements_file)
        
        try:
            with open(requirements_file, 'r') as f:
                requirements = []
                for line in f:
                    line = line.strip()
                    # Skip empty lines and comments
                    if not line or line.startswith('#'):
                        continue
                    
                    # Check platform conditions
                    if ';' in line:
                        package_part, condition_part = line.split(';', 1)
                        condition_part = condition_part.strip()
                        
                        # Skip packages with platform conditions that don't match current platform
                        if condition_part.startswith('sys_platform'):
                            # Extract the platform value from condition like "sys_platform == \"win32\""
                            if '==' in condition_part:
                                platform_value = condition_part.split('==')[1].strip().strip('"\'')
                                current_platform = sys.platform
                                if platform_value != current_platform:
                                    continue  # Skip this package as it's for a different platform
                    
                    # Extract package name (remove version specifiers and extras)
                    package = line.split('>=')[0].split('==')[0].split('[')[0].split(';')[0].strip()
                    if package:
                        requirements.append(package)
                return requirements
        except Exception as e:
            self.logger.error(f"Failed to load requirements file: {e}")
            return None  # Return None to indicate error
    
    def test_required_packages(self) -> Tuple[bool, str]:
        """Test required Python packages"""
        self.logger.info("Checking required Python packages...")
        
        required_packages = self._load_requirements()
        if required_packages is None:
            error_msg = "Failed to load requirements file"
            self.logger.error(error_msg)
            return False, error_msg
        
        if not required_packages:
            error_msg = "No required packages found in requirements file"
            self.logger.error(error_msg)
            return False, error_msg
        
        # Get list of installed packages using pip list
        try:
            result = subprocess.run(
                [sys.executable, "-m", "pip", "list", "--format=json"],
                capture_output=True,
                text=True,
                check=True
            )
            import json
            installed_packages = json.loads(result.stdout)
            installed_dict = {pkg["name"].lower(): pkg["version"] for pkg in installed_packages}
        except (subprocess.SubprocessError, json.JSONDecodeError) as e:
            error_msg = f"Failed to get installed packages list: {e}"
            self.logger.error(error_msg)
            return False, error_msg
        
        missing_packages = []
        
        for package in required_packages:
            # Handle package name variations
            package_name = package.lower()
            
            # Special handling for package name mapping
            if package_name == "pyyaml":
                package_name = "pyyaml"  # pip shows it as PyYAML
            elif package_name == "python-benedict[all]":
                package_name = "python-benedict"  # pip shows it as python-benedict
            elif "[" in package_name:
                # Remove version specifiers and extras like [all]
                package_name = package_name.split("[")[0]
            
            # Check if package is installed
            if package_name in installed_dict:
                self.logger.info(f"Python package check passed: {package} (version {installed_dict[package_name]})")
            else:
                missing_packages.append(package)
                self.logger.warning(f"Missing Python package: {package}")
                
        if missing_packages:
            error_msg = f"Missing Python packages: {', '.join(missing_packages)}"
            self.logger.error(error_msg)
            return False, error_msg
        else:
            success_msg = f"All required Python packages installed: {', '.join(required_packages)}"
            self.logger.info(success_msg)
            return True, success_msg
    
    def test_pip_availability(self) -> Tuple[bool, str]:
        """Test if pip is available"""
        self.logger.info("Checking pip availability...")
        
        try:
            result = subprocess.run(
                [sys.executable, "-m", "pip", "--version"],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if result.returncode == 0:
                pip_version = result.stdout.strip()
                self.logger.info(f"pip available: {pip_version}")
                return True, pip_version
            else:
                error_msg = f"pip not available: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
        except subprocess.TimeoutExpired:
            error_msg = "pip command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error checking pip: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def run_all_tests(self) -> List[Dict[str, Any]]:
        """Run all Python environment tests"""
        results = []
        
        # Test Python version
        result = self.run_test("python_version", self.test_python_version)
        results.append(result)

        # Test pip availability
        result = self.run_test("pip_availability", self.test_pip_availability)
        results.append(result)

        # Test required Python packages
        result = self.run_test("required_packages", self.test_required_packages)
        results.append(result)
    
        return results
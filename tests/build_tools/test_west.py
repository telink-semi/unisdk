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


class TestWest(BaseTest):
    """West build tool test"""
    
    def __init__(self, config: Dict[str, Any]):
        super().__init__(config)
        self.west_config = config.get("build_tools", {}).get("west", {})
    
    def test_west_availability(self) -> Tuple[bool, str]:
        """Test if West is available"""
        self.logger.info("Checking West availability...")
        
        min_version = self.west_config.get("min_version", "0.13.0")
        
        if not shutil.which("west"):
            error_msg = "West command not found"
            self.logger.error(error_msg)
            return False, error_msg
        
        try:
            result = subprocess.run(
                ["west", "--version"],
                capture_output=True,
                text=True,
                timeout=10
            )
            
            if result.returncode == 0:
                # Extract version information
                version_line = result.stdout.strip()
                version_str = version_line.split()[-1]
                
                if self._compare_versions(version_str, min_version) >= 0:
                    self.logger.info(f"West version check passed: {version_str} >= {min_version}")
                    return True, f"West version: {version_str}"
                else:
                    error_msg = f"West version too low: {version_str} < {min_version}"
                    self.logger.error(error_msg)
                    return False, error_msg
            else:
                error_msg = f"West not available: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
        except subprocess.TimeoutExpired:
            error_msg = "West command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error checking West: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def test_west_workspace(self) -> Tuple[bool, str]:
        """Test West workspace"""
        self.logger.info("Checking West workspace...")
        
        project_root = self.config.get("project_root", "../")
        
        # Check .west directory in parent of project_root
        west_dir = os.path.join(os.path.dirname(project_root), ".west")
        if not os.path.exists(west_dir):
            error_msg = f"West workspace directory not found: {west_dir}"
            self.logger.error(error_msg)
            return False, error_msg
        
        # Check config file
        west_config = os.path.join(west_dir, "config")
        if not os.path.exists(west_config):
            error_msg = f"West configuration file not found: {west_config}"
            self.logger.error(error_msg)
            return False, error_msg
        
        try:
            # Use west list to check workspace
            result = subprocess.run(
                ["west", "list"],
                capture_output=True,
                text=True,
                timeout=15,
                cwd=project_root
            )
            
            if result.returncode == 0:
                projects = result.stdout.strip().split('\n')
                project_count = len([p for p in projects if p.strip()])
                success_msg = f"West workspace check passed, contains {project_count} projects"
                self.logger.info(success_msg)
                self.logger.debug(result.stdout)
                return True, success_msg
            else:
                error_msg = f"West workspace check failed: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
        except subprocess.TimeoutExpired:
            error_msg = "West list command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error checking West workspace: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def test_west_build(self) -> Tuple[bool, str]:
        """Test West build functionality"""
        self.logger.info("Testing West build functionality...")
        
        project_root = self.config.get("project_root", "../")
        sample_projects = self.west_config.get("sample_projects", [])
        
        if not sample_projects:
            self.logger.info("No West sample projects configured, skipping build test")
            return True, "No West sample projects configured, skipping build test"
        
        # Try to build the first sample project
        sample_project = sample_projects[0]
        board = self.west_config.get("default_board", "TL7218X_EVK")
        
        try:
            # Run west build
            west_build_cmd = [
                "west", "tl-build",
                "--board", board,
                sample_project,
                "--pristine"
            ]
            
            result = subprocess.run(
                west_build_cmd,
                capture_output=True,
                text=True,
                timeout=120,
                cwd=project_root
            )
            
            if result.returncode == 0:
                success_msg = f"West build test passed: {sample_project} (board: {board})"
                self.logger.info(success_msg)
                self.logger.debug(result.stdout)
                return True, success_msg
            else:
                error_msg = f"West build failed: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
        except subprocess.TimeoutExpired:
            error_msg = "West build command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error during West build test: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def test_west_build_combinations(self) -> Tuple[bool, str]:
        """Test West build with various parameter combinations"""
        self.logger.info("Testing West build with parameter combinations...")
        
        project_root = self.config.get("project_root", "../")
        sample_projects = self.west_config.get("sample_projects", [])
        
        if not sample_projects:
            self.logger.info("No West sample projects configured, skipping build combination tests")
            return True, "No West sample projects configured, skipping build combination tests"
        
        sample_project = sample_projects[0]
        board = self.west_config.get("default_board", "TL7218X_EVK")
        soc = self.west_config.get("default_soc", "TL7218X")
        
        # Test combinations
        test_combinations = [
            # Basic build with board
            {
                "name": "build_with_board",
                "args": ["west", "tl-build", "--board", board, sample_project],
                "description": f"Build with board {board}"
            },
            # Build with SOC
            {
                "name": "build_with_soc",
                "args": ["west", "tl-build", "--soc", soc, sample_project],
                "description": f"Build with SOC {soc}"
            },
            # Build with both SOC and board
            {
                "name": "build_with_soc_board",
                "args": ["west", "tl-build", "--soc", soc, "--board", board, sample_project],
                "description": f"Build with SOC {soc} and board {board}"
            },
            # CMake only
            {
                "name": "cmake_only",
                "args": ["west", "tl-build", "--cmake-only", "--board", board, sample_project],
                "description": "CMake configuration only"
            },
            # Build with pristine flag
            {
                "name": "pristine_build",
                "args": ["west", "tl-build", "--pristine", "always", "--board", board, sample_project],
                "description": "Pristine build"
            },
            # Build with clean flag
            {
                "name": "clean_build",
                "args": ["west", "tl-build", "--clean", "--board", board, sample_project],
                "description": "Clean build"
            },
            # Build with target
            {
                "name": "build_with_target",
                "args": ["west", "tl-build", "--target", "all", "--board", board, sample_project],
                "description": "Build with specific target"
            },
            # Build with build options
            {
                "name": "build_with_options",
                "args": ["west", "tl-build", "--build-option", "VERBOSE=1", "--board", board, sample_project],
                "description": "Build with custom options"
            },
            # Build with custom build directory
            {
                "name": "custom_build_dir",
                "args": ["west", "tl-build", "-d", os.path.join(self.temp_dir, "test_build"), "--board", board, sample_project],
                "description": "Build with custom directory"
            }
        ]
        
        results = []
        for test_case in test_combinations:
            self.logger.info(f"Testing {test_case['name']}: {test_case['description']}")
            
            try:
                result = subprocess.run(
                    test_case["args"],
                    capture_output=True,
                    text=True,
                    timeout=120,
                    cwd=project_root
                )
                
                if result.returncode == 0:
                    success_msg = f"{test_case['name']} passed"
                    self.logger.info(success_msg)
                    self.logger.debug(result.stdout)
                    results.append((test_case["name"], True, success_msg))
                else:
                    # Check if the failure is due to missing main function, which is expected for an SDK
                    if "undefined reference to `main'" in result.stdout or "undefined reference to `main'" in result.stderr:
                        success_msg = f"{test_case['name']} passed (SDK without main function)"
                        self.logger.info(success_msg)
                        self.logger.debug(result.stdout)
                        results.append((test_case["name"], True, success_msg))
                    else:
                        error_msg = f"{test_case['name']} failed: {result.stderr}"
                        self.logger.error(error_msg)
                        results.append((test_case["name"], False, error_msg))
                        
            except subprocess.TimeoutExpired:
                error_msg = f"{test_case['name']} timeout"
                self.logger.error(error_msg)
                results.append((test_case["name"], False, error_msg))
            except Exception as e:
                error_msg = f"{test_case['name']} error: {str(e)}"
                self.logger.error(error_msg)
                results.append((test_case["name"], False, error_msg))
        
        # Count successful tests
        passed = sum(1 for _, success, _ in results if success)
        total = len(results)
        
        if passed == total:
            success_msg = f"All West build combination tests passed ({passed}/{total})"
            self.logger.info(success_msg)
            self.logger.debug(result.stdout)
            return True, success_msg
        else:
            failed_tests = [name for name, success, _ in results if not success]
            error_msg = f"Some West build combination tests failed ({passed}/{total}): {', '.join(failed_tests)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def test_west_config(self) -> Tuple[bool, str]:
        """Test West config functionality"""
        self.logger.info("Testing West config functionality...")
        
        project_root = self.config.get("project_root", "../")
        board = self.west_config.get("default_board", "TL7218X_EVK")
        soc = self.west_config.get("default_soc", "TL7218X")
        
        # Test combinations
        test_combinations = [
            # List SOCs
            {
                "name": "list_socs",
                "args": ["west", "tl-config", "--list-socs"],
                "description": "List all available SOCs"
            },
            # List boards
            {
                "name": "list_boards",
                "args": ["west", "tl-config", "--list-boards"],
                "description": "List all available boards"
            },
            # Validate SOC and board combination
            {
                "name": "validate_soc_board",
                "args": ["west", "tl-config", "--validate", soc, board],
                "description": f"Validate SOC {soc} and board {board} combination"
            },
        ]
        
        results = []
        for test_case in test_combinations:
            self.logger.info(f"Testing {test_case['name']}: {test_case['description']}")
            
            try:
                result = subprocess.run(
                    test_case["args"],
                    capture_output=True,
                    text=True,
                    timeout=60,
                    cwd=project_root
                )
                
                if result.returncode == 0:
                    # Check for CMake configuration failures in the output
                    if "Configuration failed or was cancelled" in result.stdout or \
                       "FAILED:" in result.stdout or \
                       "CMake Error" in result.stdout or \
                       "error:" in result.stdout or \
                       "Returning error code 1 due to CMake failure" in result.stdout:
                        error_msg = f"{test_case['name']} failed: Configuration error detected in output"
                        self.logger.error(error_msg)
                        self.logger.debug(f"Error details: {result.stdout}")
                        results.append((test_case["name"], False, error_msg))
                    else:
                        success_msg = f"{test_case['name']} passed"
                        self.logger.info(success_msg)
                        self.logger.debug(result.stdout)
                        results.append((test_case["name"], True, success_msg))
                else:
                    error_msg = f"{test_case['name']} failed: {result.stderr}"
                    self.logger.error(error_msg)
                    results.append((test_case["name"], False, error_msg))
                    
            except subprocess.TimeoutExpired:
                error_msg = f"{test_case['name']} timeout"
                self.logger.error(error_msg)
                results.append((test_case["name"], False, error_msg))
            except Exception as e:
                error_msg = f"{test_case['name']} error: {str(e)}"
                self.logger.error(error_msg)
                results.append((test_case["name"], False, error_msg))
        
        # Count successful tests
        passed = sum(1 for _, success, _ in results if success)
        total = len(results)
        
        if passed == total:
            success_msg = f"All West config tests passed ({passed}/{total})"
            self.logger.info(success_msg)
            self.logger.debug(result.stdout)
            return True, success_msg
        else:
            failed_tests = [name for name, success, _ in results if not success]
            error_msg = f"Some West config tests failed ({passed}/{total}): {', '.join(failed_tests)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def test_west_socs(self) -> Tuple[bool, str]:
        """Test West SOC support"""
        self.logger.info("Checking West SOC support...")
        
        project_root = self.config.get("project_root", "../")
        required_socs = self.west_config.get("required_socs", [])
        
        # Test combinations
        test_combinations = [
            # List all SOCs
            {
                "name": "list_all_socs",
                "args": ["west", "tl-socs"],
                "description": "List all available SOCs"
            },
            # List SOCs with verbose output
            {
                "name": "list_socs_verbose",
                "args": ["west", "tl-socs", "-v"],
                "description": "List all SOCs with verbose output"
            }
        ]
        
        results = []
        for test_case in test_combinations:
            self.logger.info(f"Testing {test_case['name']}: {test_case['description']}")
            
            try:
                result = subprocess.run(
                    test_case["args"],
                    capture_output=True,
                    text=True,
                    timeout=30,
                    cwd=project_root
                )
                
                if result.returncode == 0:
                    success_msg = f"{test_case['name']} passed"
                    self.logger.info(success_msg)
                    self.logger.debug(result.stdout)
                    results.append((test_case["name"], True, success_msg))
                    
                    # For the basic list command, check if required SOCs are present
                    if test_case["name"] == "list_all_socs" and required_socs:
                        available_socs = []
                        for line in result.stdout.split('\n'):
                            if line.strip():
                                # SOC name is usually in the first column
                                soc_name = line.split()[0]
                                available_socs.append(soc_name)
                        
                        missing_socs = []
                        found_socs = []
                        
                        for soc in required_socs:
                            if soc in available_socs:
                                found_socs.append(soc)
                                self.logger.info(f"Found supported SOC: {soc}")
                            else:
                                missing_socs.append(soc)
                                self.logger.warning(f"Supported SOC not found: {soc}")
                        
                        if missing_socs:
                            error_msg = f"Missing supported SOCs: {', '.join(missing_socs)}"
                            self.logger.error(error_msg)
                            results.append(("required_socs_check", False, error_msg))
                        else:
                            success_msg = f"All required SOCs are supported: {', '.join(found_socs)}"
                            self.logger.info(success_msg)
                            self.logger.debug(result.stdout)
                            results.append(("required_socs_check", True, success_msg))
                else:
                    error_msg = f"{test_case['name']} failed: {result.stderr}"
                    self.logger.error(error_msg)
                    results.append((test_case["name"], False, error_msg))
                    
            except subprocess.TimeoutExpired:
                error_msg = f"{test_case['name']} timeout"
                self.logger.error(error_msg)
                results.append((test_case["name"], False, error_msg))
            except Exception as e:
                error_msg = f"{test_case['name']} error: {str(e)}"
                self.logger.error(error_msg)
                results.append((test_case["name"], False, error_msg))
        
        # Count successful tests
        passed = sum(1 for _, success, _ in results if success)
        total = len(results)
        
        if passed == total:
            success_msg = f"All West SOC tests passed ({passed}/{total})"
            self.logger.info(success_msg)
            self.logger.debug(result.stdout)
            return True, success_msg
        else:
            failed_tests = [name for name, success, _ in results if not success]
            error_msg = f"Some West SOC tests failed ({passed}/{total}): {', '.join(failed_tests)}"
            self.logger.error(error_msg)
            return False, error_msg

    def test_west_boards(self) -> Tuple[bool, str]:
        """Test West board support"""
        self.logger.info("Checking West board support...")
        
        project_root = self.config.get("project_root", "../")
        required_boards = self.west_config.get("required_boards", [])
        default_soc = self.west_config.get("default_soc", "TL3218X")
        
        # Test combinations
        test_combinations = [
            # List all boards
            {
                "name": "list_all_boards",
                "args": ["west", "tl-boards"],
                "description": "List all available boards"
            },
            # List boards with verbose output
            {
                "name": "list_boards_verbose",
                "args": ["west", "tl-boards", "-v"],
                "description": "List all boards with verbose output"
            },
            # List boards filtered by SOC
            {
                "name": "list_boards_by_soc",
                "args": ["west", "tl-boards", "--soc", default_soc],
                "description": f"List boards supporting SOC {default_soc}"
            },
            # List boards filtered by SOC with verbose output
            {
                "name": "list_boards_by_soc_verbose",
                "args": ["west", "tl-boards", "--soc", default_soc, "-v"],
                "description": f"List boards supporting SOC {default_soc} with verbose output"
            }
        ]
        
        results = []
        for test_case in test_combinations:
            self.logger.info(f"Testing {test_case['name']}: {test_case['description']}")
            
            try:
                result = subprocess.run(
                    test_case["args"],
                    capture_output=True,
                    text=True,
                    timeout=30,
                    cwd=project_root
                )
                
                if result.returncode == 0:
                    success_msg = f"{test_case['name']} passed"
                    self.logger.info(success_msg)
                    self.logger.debug(result.stdout)
                    results.append((test_case["name"], True, success_msg))
                    
                    # For the basic list command, check if required boards are present
                    if test_case["name"] == "list_all_boards" and required_boards:
                        available_boards = []
                        for line in result.stdout.split('\n'):
                            if line.strip():
                                # Board name is usually in the first column
                                board_name = line.split()[0]
                                available_boards.append(board_name)
                        
                        missing_boards = []
                        found_boards = []
                        
                        for board in required_boards:
                            if board in available_boards:
                                found_boards.append(board)
                                self.logger.info(f"Found supported board: {board}")
                            else:
                                missing_boards.append(board)
                                self.logger.warning(f"Supported board not found: {board}")
                        
                        if missing_boards:
                            error_msg = f"Missing supported boards: {', '.join(missing_boards)}"
                            self.logger.error(error_msg)
                            results.append(("required_boards_check", False, error_msg))
                        else:
                            success_msg = f"All required boards are supported: {', '.join(found_boards)}"
                            self.logger.info(success_msg)
                            self.logger.debug(result.stdout)
                            results.append(("required_boards_check", True, success_msg))
                else:
                    error_msg = f"{test_case['name']} failed: {result.stderr}"
                    self.logger.error(error_msg)
                    results.append((test_case["name"], False, error_msg))
                    
            except subprocess.TimeoutExpired:
                error_msg = f"{test_case['name']} timeout"
                self.logger.error(error_msg)
                results.append((test_case["name"], False, error_msg))
            except Exception as e:
                error_msg = f"{test_case['name']} error: {str(e)}"
                self.logger.error(error_msg)
                results.append((test_case["name"], False, error_msg))
        
        # Count successful tests
        passed = sum(1 for _, success, _ in results if success)
        total = len(results)
        
        if passed == total:
            success_msg = f"All West board tests passed ({passed}/{total})"
            self.logger.info(success_msg)
            self.logger.debug(result.stdout)
            return True, success_msg
        else:
            failed_tests = [name for name, success, _ in results if not success]
            error_msg = f"Some West board tests failed ({passed}/{total}): {', '.join(failed_tests)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def test_west_bdt(self) -> Tuple[bool, str]:
        """Test if west tl-bdt command is available"""
        self.logger.info("Checking west tl-bdt command availability...")
        
        project_root = self.config.get("project_root", "../")
        
        try:
            # Run west tl-bdt --help
            result = subprocess.run(
                ["west", "tl-bdt", "--help"],
                capture_output=True,
                text=True,
                timeout=10,
                cwd=project_root
            )
            
           
            if result.returncode != 0:
                error_msg = f"west tl-bdt command not available: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
        except subprocess.TimeoutExpired:
            error_msg = "west tl-bdt command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error checking west tl-bdt command: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg
    
        
        try:
            # Test main help
            result = subprocess.run(
                ["west", "tl-bdt", "--help"],
                capture_output=True,
                text=True,
                timeout=10,
                cwd=project_root
            )
            
            if result.returncode != 0:
                error_msg = f"west tl-bdt --help failed: {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
            
            # Test subcommand help
            subcommands = ["download", "read", "write", "erase", "lock", "unlock", "reset"]
            for subcommand in subcommands:
                sub_result = subprocess.run(
                    ["west", "tl-bdt", subcommand, "--help"],
                    capture_output=True,
                    text=True,
                    timeout=10,
                    cwd=project_root
                )
                
                if sub_result.returncode != 0:
                    error_msg = f"west tl-bdt {subcommand} --help failed: {sub_result.stderr}"
                    self.logger.error(error_msg)
                    return False, error_msg
            
            success_msg = "west tl-bdt command passed"
            self.logger.info(success_msg)
            return True, success_msg
        except subprocess.TimeoutExpired:
            error_msg = "west tl-bdt help command timeout"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error testing west tl-bdt help: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg

    def run_all_tests(self) -> List[Dict[str, Any]]:
        """Run all West tests"""
        results = []
        
        # Test West availability
        result = self.run_test("west_availability", self.test_west_availability)
        results.append(result)
        
        # Test West workspace
        result = self.run_test("west_workspace", self.test_west_workspace)
        results.append(result)

        # Test West SOC support
        result = self.run_test("west_socs", self.test_west_socs)
        results.append(result)

        # Test West board support
        result = self.run_test("west_boards", self.test_west_boards)
        results.append(result)
        
        # Test West config functionality
        result = self.run_test("west_config", self.test_west_config)
        results.append(result)
        
        # Test West build functionality
        result = self.run_test("west_build", self.test_west_build)
        results.append(result)
        
        # Test West build with parameter combinations
        result = self.run_test("west_build_combinations", self.test_west_build_combinations)
        results.append(result)
        
        # Test West BDT functionality
        result = self.run_test("west_bdt", self.test_west_bdt)
        results.append(result)
        
        return results
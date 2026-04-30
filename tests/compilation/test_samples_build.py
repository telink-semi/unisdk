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


class TestSamplesBuild(BaseTest):
    """Sample build test"""
    
    def __init__(self, config: Dict[str, Any]):
        super().__init__(config)
        self.samples_config = config.get("compilation", {}).get("samples_build", {})
        self.test_matrix = self._load_test_matrix()
    
    def _load_test_matrix(self) -> Dict[str, Any]:
        """Load test matrix"""
        project_root = self.config.get("project_root", "../")
        test_matrix_path = os.path.join(
            os.path.dirname(__file__), "..", "config", "test_matrix.json"
        )
        
        try:
            with open(test_matrix_path, "r") as f:
                return json.load(f)
        except Exception as e:
            self.logger.warning(f"Failed to load test matrix: {e}")
            return {"test_combinations": []}
    
    def test_sample_build_matrix(self) -> Tuple[bool, str]:
        """Test sample build matrix"""
        self.logger.info("Testing sample build matrix...")
        
        project_root = self.config.get("project_root", "../")
        test_combinations = self.test_matrix.get("test_combinations", [])
        
        if not test_combinations:
            self.logger.info("Test matrix is empty, skipping test")
            return True, "Test matrix is empty, skipping test"
        
        # Only test the first combination to avoid long test times
        combination = test_combinations[0]
        soc = combination.get("soc", "")
        board = combination.get("board", "")
        samples = combination.get("samples", [])
        
        if not samples:
            self.logger.info("No samples in test combination, skipping test")
            return True, "No samples in test combination, skipping test"
        
        # Only test the first sample
        sample = samples[0]
        sample_name = sample.get("name", "")
        
        try:
            # Use temporary directory for build
            build_dir = os.path.join(self.temp_dir, f"sample_build_{sample_name}")
            
            # Build the sample
            build_cmd = [
                "west", "tl-build",
                f"samples/{sample_name}",
                "--pristine",
                "-d", build_dir
            ]
            if soc:
                build_cmd.extend(["--soc", soc])
            if board:
                build_cmd.extend(["--board", board])

            self.logger.info(f"Building sample: {sample_name} (board: {board})")
            
            result = subprocess.run(
                build_cmd,
                capture_output=True,
                text=True,
                timeout=180,
                cwd=project_root
            )
            
            if result.returncode == 0:
                # Check build artifacts
                result, msg = self.test_sample_build_artifacts(build_dir)
                if not result:
                    return False, msg
                success_msg = f"Sample build test passed: {sample_name} (board: {board})"
                self.logger.info(success_msg)
                return True, success_msg
            else:
                error_msg = f"Sample build failed: {sample_name} (board: {board}) - {result.stderr}"
                self.logger.error(error_msg)
                return False, error_msg
        except subprocess.TimeoutExpired:
            error_msg = f"Sample build timeout: {sample_name} (board: {board})"
            self.logger.error(error_msg)
            return False, error_msg
        except Exception as e:
            error_msg = f"Error during sample build test: {str(e)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def test_sample_build_artifacts(self, build_dir=None) -> Tuple[bool, str]:
        """Test sample build artifacts"""
        self.logger.info("Checking sample build artifacts...")

        expected_artifacts = self.samples_config.get("expected_artifacts", [])
        
        if not expected_artifacts:
            self.logger.info("No expected sample build artifacts configured, skipping test")
            return True, "No expected sample build artifacts configured, skipping test"
        
        # Use the provided build directory or default to project root
        if not build_dir:
            project_root = self.config.get("project_root", "../")
            build_dir = os.path.join(project_root, "build")

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
                self.logger.info(f"Found sample build artifact: {artifact}")
            else:
                missing_artifacts.append(artifact)
                self.logger.warning(f"Sample build artifact not found: {artifact}")
        
        # If all expected artifacts are found, consider it a success
        if found_files and (len(found_artifacts) == len(expected_artifacts)):
            success_msg = f"Sample build artifacts check passed. Found {len(found_files)} files including build artifacts: {', '.join(found_artifacts)}"
            self.logger.info(success_msg)
            return True, success_msg
        else:
            error_msg = f"Sample build artifacts check failed. Found {len(found_files)} files but missing key artifacts: {', '.join(missing_artifacts)}"
            self.logger.error(error_msg)
            return False, error_msg
    
    def run_all_tests(self) -> List[Dict[str, Any]]:
        """Run all sample build tests"""
        results = []
        
        # Test sample build matrix
        result = self.run_test("sample_build_matrix", self.test_sample_build_matrix)
        results.append(result)
        
        return results
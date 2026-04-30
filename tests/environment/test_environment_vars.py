#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import shutil
from typing import Dict, Any, List, Tuple

# Add current directory to Python path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from environment.base_test import BaseTest


class TestEnvironmentVars(BaseTest):
    """Environment variables test"""
    
    def __init__(self, config: Dict[str, Any]):
        super().__init__(config)
        self.env_config = config.get("environment", {})
    
    def test_required_env_vars(self) -> Tuple[bool, str]:
        """Test required environment variables"""
        self.logger.info("Checking required environment variables...")
        
        required_vars = self.env_config.get("required_vars", [])
        missing_vars = []
        
        for var in required_vars:
            value = os.environ.get(var)
            if value is None:
                missing_vars.append(var)
                self.logger.warning(f"Missing environment variable: {var}")
            else:
                self.logger.info(f"Environment variable check passed: {var}={value}")
        
        if missing_vars:
            error_msg = f"Missing environment variables: {', '.join(missing_vars)}"
            self.logger.error(error_msg)
            return False, error_msg
        else:
            success_msg = f"All required environment variables set: {', '.join(required_vars)}"
            self.logger.info(success_msg)
            return True, success_msg
    
    def test_optional_env_vars(self) -> Tuple[bool, str]:
        """Test optional environment variables"""
        self.logger.info("Checking optional environment variables...")
        
        optional_vars = self.env_config.get("optional_vars", [])
        present_vars = []
        
        for var in optional_vars:
            value = os.environ.get(var)
            if value is not None:
                present_vars.append(f"{var}={value}")
                self.logger.info(f"Optional environment variable set: {var}={value}")
            else:
                self.logger.info(f"Optional environment variable not set: {var}")
        
        if present_vars:
            success_msg = f"Set optional environment variables: {', '.join(present_vars)}"
            self.logger.info(success_msg)
        else:
            success_msg = "No optional environment variables set"
            self.logger.info(success_msg)
        
        return True, success_msg
    
    def run_all_tests(self) -> List[Dict[str, Any]]:
        """Run all environment variable tests"""
        results = []
        
        # Test required environment variables
        result = self.run_test("required_env_vars", self.test_required_env_vars)
        results.append(result)
        
        # Test optional environment variables
        result = self.run_test("optional_env_vars", self.test_optional_env_vars)
        results.append(result)
        
        return results
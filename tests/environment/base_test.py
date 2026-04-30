#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import logging
import tempfile
import shutil
from abc import ABC, abstractmethod
from typing import Dict, Any, Tuple, List
from packaging import version

# Add current directory to Python path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))


class BaseTest(ABC):
    """Base test class"""
    
    def __init__(self, config: Dict[str, Any]):
        self.config = config
        self.logger = self._setup_logger()
        self.temp_dir = tempfile.mkdtemp()
    
    def _setup_logger(self) -> logging.Logger:
        """Setup logger"""
        logger = logging.getLogger(self.__class__.__name__)
        logger.setLevel(logging.INFO)
        
        # Avoid adding handlers repeatedly
        if not logger.handlers:
            handler = logging.StreamHandler(sys.stdout)
            formatter = logging.Formatter(
                '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
            )
            handler.setFormatter(formatter)
            logger.addHandler(handler)
        
        return logger
    
    def _compare_versions(self, version1: str, version2: str) -> int:
        """Compare two version numbers
        
        Args:
            version1: First version number
            version2: Second version number
            
        Returns:
            -1: version1 < version2
            0: version1 == version2
            1: version1 > version2
        """
        try:
            v1 = version.parse(version1)
            v2 = version.parse(version2)
            
            if v1 < v2:
                return -1
            elif v1 > v2:
                return 1
            else:
                return 0
        except Exception as e:
            self.logger.warning(f"Version comparison failed: {e}, using string comparison")
            if version1 < version2:
                return -1
            elif version1 > version2:
                return 1
            else:
                return 0
    
    def run_test(self, test_name: str, test_func) -> Dict[str, Any]:
        """Run a single test
        
        Args:
            test_name: Test name
            test_func: Test function
            
        Returns:
            Test result dictionary
        """
        self.logger.info(f"Running test: {test_name}")
        
        try:
            success, message = test_func()
            result = {
                "name": test_name,
                "success": success,
                "message": message,
                "details": {}
            }
            
            if success:
                self.logger.info(f"Test passed: {test_name} - {message}")
            else:
                self.logger.error(f"Test failed: {test_name} - {message}")
                
            return result
        except Exception as e:
            error_msg = f"Test execution exception: {str(e)}"
            self.logger.error(f"Test exception: {test_name} - {error_msg}")
            
            return {
                "name": test_name,
                "success": False,
                "message": error_msg,
                "details": {"exception": str(e)}
            }
    
    @abstractmethod
    def run_all_tests(self) -> List[Dict[str, Any]]:
        """Run all tests (must be implemented by subclasses)"""
        pass
    
    def cleanup(self):
        """Clean up temporary files"""
        if os.path.exists(self.temp_dir):
            shutil.rmtree(self.temp_dir)
            self.logger.info(f"Cleaned up temporary directory: {self.temp_dir}")
    
    def __del__(self):
        """Destructor to ensure cleanup of temporary files"""
        try:
            self.cleanup()
        except:
            pass
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
UniSDK Automated Testing System
Used to verify various aspects of the UniSDK development environment
"""

import os
import sys
import json
import time
import argparse
import importlib.util
import subprocess
from datetime import datetime
from typing import Dict, List, Any, Optional

# Add current directory to Python path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from config.config_loader import load_config
from environment.test_python import TestPython
from environment.test_toolchain import TestToolchain
from environment.test_environment_vars import TestEnvironmentVars
from build_tools.test_cmake import TestCMake
from build_tools.test_make import TestMake
from build_tools.test_west import TestWest
from compilation.test_basic_build import TestBasicBuild
from compilation.test_samples_build import TestSamplesBuild

class TestResult:
    """Test result class"""
    def __init__(self, name: str, category: str):
        self.name = name
        self.category = category
        self.passed = False
        self.start_time = None
        self.end_time = None
        self.duration = 0
        self.error_message = ""
        self.details = {}
    
    def start(self):
        """Start test"""
        self.start_time = time.time()
    
    def end(self):
        """End test"""
        self.end_time = time.time()
        self.duration = self.end_time - self.start_time
    
    def set_passed(self, passed: bool, error_message: str = ""):
        """Set test result"""
        self.passed = passed
        self.error_message = error_message
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary"""
        return {
            "name": self.name,
            "category": self.category,
            "passed": self.passed,
            "duration": self.duration,
            "error_message": self.error_message,
            "details": self.details
        }

class BaseTest:
    """Test base class"""
    def __init__(self, name: str, category: str):
        self.name = name
        self.category = category
        self.config = {}
        self.result = TestResult(name, category)
    
    def setup(self, config: Dict[str, Any]):
        """Set test configuration"""
        self.config = config
    
    def run(self) -> bool:
        """Run test, subclasses need to implement this method"""
        raise NotImplementedError("Subclasses must implement the run() method")
    
    def execute(self) -> TestResult:
        """Execute test"""
        self.result.start()
        try:
            passed = self.run()
            self.result.set_passed(passed)
        except Exception as e:
            self.result.set_passed(False, str(e))
        self.result.end()
        return self.result

class TestRunner:
    """Test runner"""
    def __init__(self, config_path: str = "config/config.json"):
        self.config = self._load_config(config_path)
        self.tests = {}
        self.results = []
        self._discover_tests()
    
    def _load_config(self, config_path: str) -> Dict[str, Any]:
        """Load configuration file
        
        Args:
            config_path: Configuration file path
            
        Returns:
            Configuration dictionary
        """
        try:
            config = load_config(config_path)
        except Exception as e:
            print(f"Error: Unable to load configuration file {config_path}: {e}")
            sys.exit(1)
        
        # Set project_root based on TELINK_BASE environment variable or current directory
        import os
        if os.environ.get("TELINK_BASE"):
            config["project_root"] = os.environ.get("TELINK_BASE")
        else:
            # Use the parent directory of the tests directory as project_root
            tests_dir = os.path.dirname(os.path.abspath(__file__))
            config["project_root"] = os.path.dirname(tests_dir)
        
        return config
    
    def _discover_tests(self):
        """Discover test files"""
        # Directly instantiate test classes instead of dynamic imports
        self.tests = {
            "environment": {
                "python": TestPython(self.config),
                "toolchain": TestToolchain(self.config),
                "environment_vars": TestEnvironmentVars(self.config)
            },
            "build_tools": {
                "cmake": TestCMake(self.config),
                "make": TestMake(self.config),
                "west": TestWest(self.config)
            },
            "compilation": {
                "basic_build": TestBasicBuild(self.config),
                "samples_build": TestSamplesBuild(self.config),
            }
        }
    
    def run_test(self, test_name: str) -> TestResult:
        """Run a single test"""
        # Parse test name, format should be category.test_name.method_name
        parts = test_name.split('.')
        if len(parts) != 3:
            raise ValueError(f"Invalid test name format, should be category.test_name.method_name: {test_name}")
        
        category, test_class_name, method_name = parts
        
        if category not in self.tests:
            raise ValueError(f"Test category not found: {category}")
        
        if test_class_name not in self.tests[category]:
            raise ValueError(f"Test class not found: {category}.{test_class_name}")
        
        test = self.tests[category][test_class_name]
        
        # Create TestResult object and start timing
        test_result = TestResult(
            name=test_name,
            category=category
        )
        test_result.start()
        
        try:
            # Run all tests and find the specified test method
            results = test.run_all_tests()
            
            # Find the specified test method
            found = False
            for result in results:
                if result['name'] == method_name:
                    test_result.passed = result['success']
                    test_result.error_message = result['message']
                    test_result.details = result.get('details', {})
                    found = True
                    break
            
            if not found:
                raise ValueError(f"Test method not found: {test_name}")
                
        except Exception as e:
            test_result.passed = False
            test_result.error_message = str(e)
        
        # End timing
        test_result.end()
        self.results.append(test_result)
        return test_result
    
    def _process_test_results(self, category: str, test_name: str, test, results_list: List[TestResult]) -> None:
        """Process test results and create TestResult objects
        
        Args:
            category: Test category
            test_name: Test class name
            test: Test instance
            results_list: List to append results to
        """
        # Time the entire test execution for this test class
        start_time = time.time()
        results = test.run_all_tests()
        end_time = time.time()
        class_duration = end_time - start_time
        
        # Distribute the duration evenly among all test methods
        if results:
            duration_per_test = class_duration / len(results)
            
            for result in results:
                test_result = TestResult(
                    name=f"{category}.{test_name}.{result['name']}",
                    category=category
                )
                
                # Set result
                test_result.passed = result['success']
                test_result.error_message = result['message']
                test_result.details = result.get('details', {})
                
                # Set duration
                test_result.duration = duration_per_test
                
                self.results.append(test_result)
                results_list.append(test_result)
    
    def run_category(self, category: str) -> List[TestResult]:
        """Run all tests in a specific category"""
        category_results = []
        if category in self.tests:
            for test_name, test in self.tests[category].items():
                self._process_test_results(category, test_name, test, category_results)
        return category_results
    
    def run_all(self) -> List[TestResult]:
        """Run all tests"""
        all_results = []
        for category, tests in self.tests.items():
            for test_name, test in tests.items():
                self._process_test_results(category, test_name, test, all_results)
        return all_results
    
    def run_tests(self, test_type: str = "all", quick_mode: bool = False) -> List[TestResult]:
        """Run tests
        
        Args:
            test_type: Test type (all, environment, build_tools, compilation)
            quick_mode: Quick mode, run only partial tests
            
        Returns:
            List of test results
        """
        self.results = []
        
        # Select tests to run based on test type
        if test_type == "all":
            test_categories = list(self.tests.keys())
        elif test_type in self.tests:
            test_categories = [test_type]
        else:
            print(f"Unknown test type: {test_type}")
            return self.results
        
        # Run tests
        for category in test_categories:
            if category not in self.tests:
                continue
                
            print(f"Running {category} tests...")
            category_tests = self.tests[category]
            
            for test_name, test_instance in category_tests.items():
                # Skip some time-consuming tests in quick mode
                if quick_mode and test_name in ["samples_build", "multiple_configs"]:
                    print(f"Quick mode: Skipping test {test_name}")
                    continue
                
                print(f"Running test: {test_name}")
                test_instance.setup(self.config)
                result = test_instance.execute()
                result.name = f"{category}.{test_name}"
                self.results.append(result)
                
                if result.passed:
                    print(f"Test passed: {result.name}")
                else:
                    print(f"Test failed: {result.name} - {result.error_message}")
        
        return self.results
    
    def generate_json_report(self, output_path: str):
        """Generate JSON report"""
        report = {
            "timestamp": datetime.now().isoformat(),
            "summary": {
                "total": len(self.results),
                "passed": sum(1 for r in self.results if r.passed),
                "failed": sum(1 for r in self.results if not r.passed),
                "duration": sum(r.duration for r in self.results)
            },
            "results": [r.to_dict() for r in self.results]
        }
        
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        with open(output_path, 'w', encoding='utf-8') as f:
            json.dump(report, f, indent=2, ensure_ascii=False)
        
        print(f"JSON report generated: {output_path}")
    
    def generate_html_report(self, output_path: str):
        """Generate HTML report"""
        total = len(self.results)
        passed = sum(1 for r in self.results if r.passed)
        failed = total - passed
        duration = sum(r.duration for r in self.results)
        
        html = f"""<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>UniSDK Test Report</title>
    <style>
        body {{
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 20px;
            background-color: #f5f5f5;
        }}
        .container {{
            max-width: 1200px;
            margin: 0 auto;
            background-color: white;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }}
        h1 {{
            color: #333;
            text-align: center;
        }}
        .summary {{
            display: flex;
            justify-content: space-around;
            margin-bottom: 20px;
        }}
        .summary-item {{
            text-align: center;
            padding: 15px;
            border-radius: 5px;
            color: white;
        }}
        .total {{ background-color: #3498db; }}
        .passed {{ background-color: #2ecc71; }}
        .failed {{ background-color: #e74c3c; }}
        .duration {{ background-color: #9b59b6; }}
        .test-category {{
            margin-bottom: 20px;
        }}
        .category-title {{
            background-color: #f1f1f1;
            padding: 10px;
            border-left: 4px solid #3498db;
            font-weight: bold;
        }}
        .test-item {{
            margin-bottom: 10px;
            border: 1px solid #ddd;
            border-radius: 5px;
            overflow: hidden;
        }}
        .test-header {{
            padding: 10px;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }}
        .test-passed {{ background-color: #d4edda; }}
        .test-failed {{ background-color: #f8d7da; }}
        .test-details {{
            padding: 10px;
            background-color: #f9f9f9;
            border-top: 1px solid #ddd;
        }}
        .error-message {{
            color: #721c24;
            font-family: monospace;
            white-space: pre-wrap;
        }}
        .timestamp {{
            text-align: center;
            color: #666;
            margin-top: 20px;
            font-size: 14px;
        }}
    </style>
</head>
<body>
    <div class="container">
        <h1>UniSDK Test Report</h1>
        
        <div class="summary">
            <div class="summary-item total">
                <div>Total Tests</div>
                <div>{total}</div>
            </div>
            <div class="summary-item passed">
                <div>Passed</div>
                <div>{passed}</div>
            </div>
            <div class="summary-item failed">
                <div>Failed</div>
                <div>{failed}</div>
            </div>
            <div class="summary-item duration">
                <div>Total Duration</div>
                <div>{duration:.2f}s</div>
            </div>
        </div>
        
        <div class="test-results">"""
        
        # Group test results by category
        categories = {}
        for result in self.results:
            if result.category not in categories:
                categories[result.category] = []
            categories[result.category].append(result)
        
        for category, results in categories.items():
            html += f'<div class="test-category">'
            html += f'<div class="category-title">{category}</div>'
            
            for result in results:
                status_class = "test-passed" if result.passed else "test-failed"
                status_text = "Passed" if result.passed else "Failed"
                
                html += f'<div class="test-item">'
                html += f'<div class="test-header {status_class}">'
                html += f'<div>{result.name}</div>'
                html += f'<div>{status_text} ({result.duration:.2f}s)</div>'
                html += f'</div>'
                
                if not result.passed:
                    html += f'<div class="test-details">'
                    html += f'<div class="error-message">{result.error_message}</div>'
                    html += f'</div>'
                
                html += f'</div>'
            
            html += f'</div>'
        
        html += f"""
        </div>
        
        <div class="timestamp">
            Report generated on: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
        </div>
    </div>
</body>
</html>"""
        
        os.makedirs(os.path.dirname(output_path), exist_ok=True)
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(html)
        
        print(f"HTML report generated: {output_path}")

def main():
    """Main function"""
    parser = argparse.ArgumentParser(description='UniSDK Automated Test System')
    parser.add_argument('--all', action='store_true', help='Run all tests')
    parser.add_argument('--category', choices=['environment', 'build_tools', 'compilation'], 
                        help='Run tests of a specific category')
    parser.add_argument('--test', help='Run a specific test')
    parser.add_argument('--html-report', action='store_true', help='Generate HTML report')
    parser.add_argument('--json-report', action='store_true', help='Generate JSON report')
    parser.add_argument('--config', default='config/config.json', help='Configuration file path')
    
    args = parser.parse_args()
    
    # Create test runner
    runner = TestRunner(args.config)
    
    # Run tests
    if args.all:
        print("Running all tests...")
        runner.run_all()
    elif args.category:
        print(f"Running {args.category} category tests...")
        runner.run_category(args.category)
    elif args.test:
        print(f"Running test: {args.test}")
        runner.run_test(args.test)
    else:
        print("Error: Must specify --all, --category or --test")
        parser.print_help()
        sys.exit(1)
    
    # Print test results summary
    total = len(runner.results)
    passed = sum(1 for r in runner.results if r.passed)
    failed = total - passed
    
    print("\nTest Results Summary:")
    print(f"Total Tests: {total}")
    print(f"Passed: {passed}")
    print(f"Failed: {failed}")
    print(f"Total Duration: {sum(r.duration for r in runner.results):.2f}s")
    
    # Generate reports
    reports_dir = runner.config.get("reports_dir", "reports")
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    
    if args.html_report:
        html_path = os.path.join(reports_dir, f"test_report_{timestamp}.html")
        runner.generate_html_report(html_path)
    
    if args.json_report:
        json_path = os.path.join(reports_dir, f"test_report_{timestamp}.json")
        runner.generate_json_report(json_path)
    
    # If any tests failed, return non-zero exit code
    if failed > 0:
        print("\nFailed Tests:")
        for result in runner.results:
            if not result.passed:
                print(f"- {result.name}: {result.error_message}")
        sys.exit(1)
    else:
        print("\nAll tests passed!")
        sys.exit(0)

if __name__ == "__main__":
    main()
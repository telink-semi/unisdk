#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
import os
from typing import Dict, Any


def load_config(config_path: str) -> Dict[str, Any]:
    """Load test configuration
    
    Args:
        config_path: Configuration file path
        
    Returns:
        Configuration dictionary
    """
    # If it's a relative path, make it relative to the current script directory
    if not os.path.isabs(config_path):
        script_dir = os.path.dirname(os.path.abspath(__file__))
        # If the path already includes the config directory, connect directly
        if config_path.startswith("config/"):
            config_path = os.path.join(script_dir, config_path[7:])  # Remove "config/" prefix
        else:
            config_path = os.path.join(script_dir, config_path)
    
    try:
        with open(config_path, "r", encoding="utf-8") as f:
            config = json.load(f)
        # Add the config file path to the config for reference
        config["_config_file_path"] = config_path
        return config
    except FileNotFoundError:
        raise FileNotFoundError(f"Configuration file not found: {config_path}")
    except json.JSONDecodeError as e:
        raise ValueError(f"Configuration file format error: {e}")
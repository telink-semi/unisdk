"""
Shared validation utilities for west commands.
Provides SOC and BOARD validation logic that can be used across multiple commands.
"""

import pathlib
from typing import Callable, Optional, Tuple


def validate_soc_board(soc: str, board: str, warn_func: Optional[Callable] = None) -> Tuple[bool, str]:
    """
    Validate if SOC and BOARD combination is valid.

    Args:
        soc: SOC name (e.g., 'B91', 'B92')
        board: BOARD name (e.g., 'tlsr9518adk80d')
        warn_func: Optional warning function for logging (e.g., self.wrn)

    Returns:
        Tuple of (is_valid, error_message)
        If valid, error_message will be empty string
    """
    try:
        from kconfig_parser import KconfigParser
    except ImportError as e:
        if warn_func:
            warn_func(f"Warning: Failed to import kconfig_parser: {e}")
        return True, ""

    try:
        telink_base = pathlib.Path(__file__).resolve().parent.parent.parent
        kconfig_file = telink_base / 'Kconfig.chip'

        if not kconfig_file.exists():
            return True, ""

        parser = KconfigParser(str(kconfig_file), str(telink_base))
        is_valid, error_msg = parser.validate_soc_board_combination(soc, board)
        return is_valid, error_msg

    except Exception as e:
        if warn_func:
            warn_func(f"Warning: SOC/BOARD validation failed: {e}")
        return True, ""
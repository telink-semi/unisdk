#!/usr/bin/env python3
"""
Generate or update chip.config from Kconfig.chip.
Usage: script.py <kconfig_path> <output_path> [--soc-series <series>] [--soc <soc>] [--board <board>]

Inputs: --soc-series, --soc, --board (all optional, any combination).

How SoC / SoC series are resolved, in priority order:
  1. --board given: it fully determines SoC and SoC series (a board
     depends on exactly one SoC). If --soc and/or --soc-series are also
     given, they must match the board's own SoC/series, or it's an error.
  2. else --soc given: its SoC series is taken from the Kconfig
     dependency. If --soc-series is also given, it must match.
  3. else --soc-series given (no --soc): no SoC is selected, so Kconfig
     falls back to its own default SoC for that series.
  4. else nothing given: the current SoC/series from chip.config (if any)
     are kept unchanged.

How the board is resolved:
  - --board given: that board is selected (after the compatibility check
    above).
  - --board not given: the previously selected board (from an existing
    chip.config) is kept only if the resolved SoC and SoC series above
    are unchanged from chip.config; if either changed, the board is
    deselected, since it may not fit the new SoC/series. If chip.config
    does not exist yet, no board is selected.

Every --soc-series / --soc / --board value is validated against the
Kconfig tree; unknown names abort with an error.

Note: SoC series, SoC and board are members of `optional` Kconfig
choices. Symbol.set_value(2) alone only records the selection - the
enclosing Choice must also be switched to y-mode with
Choice.set_value(2), or the selection has no effect on chip.config.
"""

import argparse
import os
import re
import sys
import kconfiglib

_ENABLED_RE = re.compile(r"^CONFIG_TLK_(SOC_SERIES_|SOC_|BOARD_)(.+)=y$")
_KEY_BY_PREFIX = {"SOC_SERIES_": "series", "SOC_": "soc", "BOARD_": "board"}


def get_parent(sym, prefix):
    """Find the direct-dependency symbol whose name starts with `prefix`."""
    if not sym:
        return None

    deps = list(kconfiglib.expr_items(sym.direct_dep))
    if sym.choice:
        deps += kconfiglib.expr_items(sym.choice.direct_dep)
        for node in sym.choice.nodes:
            deps += kconfiglib.expr_items(node.dep)
    for node in sym.nodes:
        deps += kconfiglib.expr_items(node.dep)

    return next(
        (
            item
            for item in deps
            if isinstance(item, kconfiglib.Symbol)
            and item.name
            and item.name.startswith(prefix)
        ),
        None,
    )


def read_current_config(filepath):
    """Read the currently-enabled series/SoC/board names from an existing chip.config."""
    curr = {"series": None, "soc": None, "board": None}
    if not os.path.exists(filepath):
        return curr

    with open(filepath, "r", encoding="utf-8") as f:
        for line in f:
            match = _ENABLED_RE.match(line.strip())
            if match:
                prefix, name = match.groups()
                curr[_KEY_BY_PREFIX[prefix]] = name
    return curr


def lookup_symbol(kconf, prefix, name, label):
    """Resolve a CLI argument to a Kconfig symbol, or exit with a clear error."""
    if not name:
        return None
    sym = kconf.syms.get(f"{prefix}{name.upper()}")
    if sym is None:
        sys.exit(f"Error: Unknown {label} '{name}'")
    return sym


def select(sym):
    """Select a symbol, forcing its enclosing choice into y-mode first."""
    if sym.choice:
        sym.choice.set_value(2)
    sym.set_value(2)


def main():
    parser = argparse.ArgumentParser(description="Generate or update chip.config")
    parser.add_argument("kconfig", help="Path to Kconfig.chip")
    parser.add_argument("output", help="Path to output chip.config")
    parser.add_argument("--soc-series", help="SoC Series name")
    parser.add_argument("--soc", help="SoC name")
    parser.add_argument("--board", help="Board name")
    args = parser.parse_args()

    kconf = kconfiglib.Kconfig(args.kconfig)
    config_exists = os.path.exists(args.output)
    curr = read_current_config(args.output)

    req_series = lookup_symbol(kconf, "TLK_SOC_SERIES_", args.soc_series, "SOC_SERIES")
    req_soc = lookup_symbol(kconf, "TLK_SOC_", args.soc, "SOC")
    req_board = lookup_symbol(kconf, "TLK_BOARD_", args.board, "BOARD")

    if req_board:
        board_soc = get_parent(req_board, "TLK_SOC_")
        board_series = get_parent(board_soc, "TLK_SOC_SERIES_") if board_soc else None

        if req_soc and board_soc != req_soc:
            sys.exit(f"Error: BOARD '{args.board}' is incompatible with SOC '{args.soc}'")
        if req_series and board_series != req_series:
            sys.exit(
                f"Error: BOARD '{args.board}' is incompatible with SOC_SERIES"
                f" '{args.soc_series}'"
            )

        target_series, target_soc, target_board = board_series, board_soc, req_board

    elif req_soc:
        soc_series = get_parent(req_soc, "TLK_SOC_SERIES_")
        if req_series and soc_series != req_series:
            sys.exit(
                f"Error: SOC '{args.soc}' is incompatible with SOC_SERIES"
                f" '{args.soc_series}'"
            )
        target_series, target_soc, target_board = soc_series or req_series, req_soc, None

    elif req_series:
        target_series, target_soc, target_board = req_series, None, None

    else:
        # Nothing requested: keep whatever is currently configured.
        target_soc = kconf.syms.get(f"TLK_SOC_{curr['soc']}") if curr["soc"] else None
        target_series = (
            kconf.syms.get(f"TLK_SOC_SERIES_{curr['series']}") if curr["series"] else None
        )
        target_board = None

    if not req_board:
        changed = config_exists and (
            (req_soc is not None and req_soc.name != f"TLK_SOC_{curr['soc']}")
            or (req_series is not None and req_series.name != f"TLK_SOC_SERIES_{curr['series']}")
        )
        if config_exists and not changed and curr["board"]:
            target_board = kconf.syms.get(f"TLK_BOARD_{curr['board']}")

    for sym in (target_series, target_soc, target_board):
        if sym:
            select(sym)

    kconf.write_config(args.output)


if __name__ == "__main__":
    main()
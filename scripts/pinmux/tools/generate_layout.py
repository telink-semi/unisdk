"""Generate placeholder (schematic, not physically accurate) case layout YAML
files for every soc/<series>/pinmux/pinmux_<case>.yaml found in the repo.

Run with: python -m scripts.pinmux.tools.generate_layout

Each generated soc/<series>/pinmux/layout_<case>.yaml lists every physical
pin number for the package and marks which ones are GPIO (with their port/pin) versus unknown/non-GPIO (power,
ground, RF, etc. - identity intentionally left blank pending real datasheet
data). Existing layout_*.yaml files are left untouched unless --force is
passed, so hand-edited real pinout data is never clobbered by a re-run.

Pins carry no side: the pinmux editor places them counterclockwise from pin 1
at the top of the left side, splitting them evenly across the 4 sides unless
the layout gives an explicit 'sides: [left, bottom, right, top]' pin count.
"""

import argparse
import re
from pathlib import Path
from typing import Optional

import yaml

ROOT = Path(__file__).resolve().parents[3]


def load_yaml(path: Path) -> dict:
    if not path.exists():
        return {}
    with path.open(encoding="utf-8") as f:
        return yaml.safe_load(f) or {}


def gpio_pins_for_case(pinmux_dir: Path, case_file: Path) -> list[str]:
    """Return the ordered list of PortPin names (e.g. 'PA0') that physically
    exist for the given case, applying the base map's port/pin entries plus
    the case-specific 'delete' overrides. Function filtering (exclude/include)
    is irrelevant here - only physical existence matters."""

    base = load_yaml(pinmux_dir / "pinmux.yaml").get("pinmux", {}).get("port", {})
    override = load_yaml(case_file).get("pinmux", {}).get("port", {})

    pins = []
    for port_name in sorted(base.keys()):
        if not isinstance(port_name, str) or len(port_name) != 2 or port_name[0] != "p":
            continue

        port_entry = base[port_name]
        override_entry = override.get(port_name) if isinstance(override, dict) else None

        if override_entry == "delete":
            continue
        if port_entry == "delete":
            continue

        base_pins = (port_entry.get("pins", {}) if isinstance(port_entry, dict) else {}) or {}
        override_pins = (override_entry.get("pins", {}) if isinstance(override_entry, dict) else {}) or {}

        for pin_name in sorted(base_pins.keys()):
            if not isinstance(pin_name, str) or len(pin_name) != 2 or pin_name[0] != "p":
                continue
            if override_pins.get(pin_name) == "delete":
                continue
            pins.append(f"P{port_name[1].upper()}{pin_name[1].upper()}")

    return pins


def find_pin_count(case: str) -> Optional[int]:
    m = re.search(r"(\d+)$", case)
    return int(m.group(1)) if m else None


def build_layout(case: str, gpio_pins: list[str], pin_count: int) -> dict:
    total = max(pin_count, len(gpio_pins))
    slots: list[Optional[str]] = list(gpio_pins) + [None] * (total - len(gpio_pins))

    pins_out = []
    for number, slot in enumerate(slots, start=1):
        if slot is not None:
            pins_out.append({"number": number, "kind": "gpio", "port_pin": slot})
        else:
            pins_out.append({"number": number, "kind": "unknown", "label": None})

    return {"case": case, "pin_count": total, "pins": pins_out}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--force", action="store_true", help="Overwrite existing layout_*.yaml files")
    args = parser.parse_args()

    packages_dir = ROOT / "packages"
    for package_dir in sorted(packages_dir.iterdir()):
        soc_dir = package_dir / "soc"
        if not soc_dir.is_dir():
            continue

        for series_dir in sorted(soc_dir.iterdir()):
            pinmux_dir = series_dir / "pinmux"
            if not pinmux_dir.is_dir():
                continue

            for case_file in sorted(pinmux_dir.glob("pinmux_*.yaml")):
                case = case_file.stem[len("pinmux_"):].upper()
                pin_count = find_pin_count(case)
                if pin_count is None:
                    print(f"Skipping {case_file}: cannot infer pin count from case name '{case}'")
                    continue

                layout_path = pinmux_dir / f"layout_{case.lower()}.yaml"
                if layout_path.exists() and not args.force:
                    print(f"Skipping existing {layout_path}")
                    continue

                gpio_pins = gpio_pins_for_case(pinmux_dir, case_file)
                layout = build_layout(case, gpio_pins, pin_count)

                with layout_path.open("w", encoding="utf-8") as f:
                    f.write(
                        "# Placeholder pin layout - schematic only, NOT physically accurate.\n"
                        "# Pins are numbered counterclockwise from the top of the left side,\n"
                        "# evenly split across the 4 sides unless 'sides: [left, bottom, right,\n"
                        "# top]' gives the per-side pin counts. GPIO pins carry a 'port_pin'\n"
                        "# (e.g. PA0), everything else is 'kind: unknown' pending real datasheet\n"
                        "# pinout data. Replace this file with the real per-pin table\n"
                        "# (GPIO/VCC/GND/RF/...) when available.\n"
                    )
                    yaml.safe_dump(layout, f, sort_keys=False)

                print(f"Wrote {layout_path} ({len(gpio_pins)} gpio / {pin_count} total pins)")


if __name__ == "__main__":
    main()

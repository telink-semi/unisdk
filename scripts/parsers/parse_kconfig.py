"""
Preprocess a single Kconfig file: expand ${...} macros sourced from a
pre-parsed properties cache (pickle) and expand `for ... endfor` blocks.

Unlike preprocess_kconfig.py (which walks whole directories and parses
YAML files itself), this script processes exactly one Kconfig file per
invocation and reads already-parsed properties from a pickle cache
produced by yaml_cache_gen.py. This lets each Kconfig file become its
own CMake custom_command output, so Ninja only re-parses the files that
actually changed instead of the whole tree.

Usage:
    python parse_kconfig_file.py <properties_cache.pkl> <input_kconfig> <output_kconfig>
"""

import argparse
import pickle
import re
from pathlib import Path


def get_yaml_value(properties, key):
    """Resolve a dotted/indexed key such as chip.memory.rom.size or bar[2]
    against the properties dict loaded from the pickle cache."""
    parts = key.split('.')
    val = properties
    for part in parts:
        m = re.match(r"(\w+)\[(\d+)\]$", part)
        if m:
            name, idx = m.group(1), int(m.group(2))
            if isinstance(val, dict) and name in val and isinstance(val[name], list):
                val = val[name]
                if 0 <= idx < len(val):
                    val = val[idx]
                else:
                    return None
            else:
                return None
        else:
            if isinstance(val, dict) and part in val:
                val = val[part]
            else:
                return None
    return val


def expand_macros(text, properties):
    """Replace ${key} occurrences with values resolved from properties."""

    def repl(match):
        key = match.group(1)

        if key.endswith(".count"):
            val = get_yaml_value(properties, key[:-6])
            if isinstance(val, list):
                return str(len(val))

        val = get_yaml_value(properties, key)

        if isinstance(val, list):
            return ",".join(str(item) for item in val)

        if isinstance(val, bool):
            return 'y' if val else 'n'

        return str(val) if val is not None else match.group(0)

    return re.sub(r"\$\{([A-Za-z0-9_.\[\]]+)\}", repl, text)


def extract_block(lines, start_idx):
    """Extract lines up to the matching endfor, supporting nesting."""
    block_lines = []
    i = start_idx
    nest_level = 0
    while i < len(lines):
        line = lines[i]
        if re.match(r"for\s+\w+\s+in\s+range\s+\d+\s+\d+", line.strip()) or \
           re.match(r"for\s+\w+\s+in\s*\[.+\]", line.strip()):
            nest_level += 1
        elif line.strip().startswith("endfor"):
            if nest_level == 0:
                break
            nest_level -= 1
        block_lines.append(line)
        i += 1
    return block_lines, i


def expand_for_loops(template_text, properties):
    """Expand `for x in range a b` / `for x in [a, b, c]` ... `endfor` blocks."""
    output_lines = []
    lines = template_text.splitlines()
    i = 0
    while i < len(lines):
        line = lines[i]
        m_range = re.match(r"for\s+(\w+)\s+in\s+range\s+(\d+)\s+(\d+)", line.strip())
        m_list = re.match(r"for\s+(\w+)\s+in\s*\[(.+)\]", line.strip())

        if m_range:
            var_name, start, end = m_range.groups()
            start, end = int(start), int(end)
            block_lines, block_end = extract_block(lines, i + 1)
            for val in range(start, end):
                block_text = '\n'.join(
                    bl.replace(f"{{{var_name}}}", str(val)) for bl in block_lines
                )
                output_lines.append(expand_for_loops(block_text, properties))
            i = block_end

        elif m_list:
            var_name, values = m_list.groups()
            items = [v.strip() for v in values.split(',')]
            block_lines, block_end = extract_block(lines, i + 1)
            for val in items:
                block_text = '\n'.join(
                    bl.replace(f"{{{var_name}}}", val) for bl in block_lines
                )
                output_lines.append(expand_for_loops(block_text, properties))
            i = block_end

        elif line.strip().startswith("endfor"):
            pass
        else:
            output_lines.append(line)

        i += 1
    return "\n".join(output_lines)


def process_kconfig(properties, input_path: Path, output_path: Path):
    text = input_path.read_text(encoding="utf-8")

    # First macro pass (resolves plain ${...} references)
    text = expand_macros(text, properties)
    # Expand for-loops (may introduce new ${...} references via {var} substitution)
    text = expand_for_loops(text, properties)
    # Second macro pass, to resolve macros produced by the for-loop expansion
    text = expand_macros(text, properties)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(text, encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description="Parse a single Kconfig file using a properties cache")
    parser.add_argument("properties_cache", type=Path,
                         help="Path to the pickle file produced by yaml_cache_gen.py")
    parser.add_argument("input_file", type=Path,
                         help="Source Kconfig file to preprocess")
    parser.add_argument("output_file", type=Path,
                         help="Destination path for the preprocessed Kconfig file")
    args = parser.parse_args()

    if not args.properties_cache.is_file():
        parser.error(f"properties cache not found: {args.properties_cache}")
    if not args.input_file.is_file():
        parser.error(f"input Kconfig file not found: {args.input_file}")

    with args.properties_cache.open("rb") as f:
        properties = pickle.load(f)

    process_kconfig(properties, args.input_file, args.output_file)
    print(f"Generated {args.output_file}")


if __name__ == "__main__":
    main()

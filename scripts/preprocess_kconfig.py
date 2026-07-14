import os
import re
import sys
import yaml
from pathlib import Path
from benedict.core import merge
import argparse

def load_config(config_path, config_name):
    with open(config_path) as f:
        for line in f:
            m = re.match(fr'{config_name}="?([A-Za-z0-9_]+)"?', line.strip())
            if m:
                return m.group(1)
    return None

def load_yaml_vars(chip_dir):
    vars = {}
    for yfile in Path(chip_dir).glob("*.yaml"):
        with open(yfile) as f:
            data = yaml.safe_load(f)
            if data:
                vars.update(data)
    return vars

def get_yaml_value(yaml_dict, key):
    # Supports nested keys: chip.memory.rom.size
    parts = key.split('.')
    val = yaml_dict
    for p in parts:
        # Check for array index, e.g., bar[2]
        m = re.match(r"(\w+)\[(\d+)\]$", p)
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
            if isinstance(val, dict) and p in val:
                val = val[p]
            else:
                return None
    return val

def expand_macros_from_yaml(text, yaml_vars, additional_options):
    # Replace ${...} (but not {...}) with values from yaml_vars
    def repl(m):
        key = m.group(1)

        # Try to return the fake .count property for arrays
        if key.endswith(".count"):
            val = get_yaml_value(yaml_vars, key[:-6])
            if isinstance(val, list):
                return str(len(val)) 

        val = get_yaml_value(yaml_vars, key)
        if (val == None):
            val = next((opt["value"] for opt in additional_options if opt["name"] == key), None)

    
        # Return the arrays in format A,B,C,1,2,3
        if isinstance(val, list):
            return ",".join([str(e) for e in val])
        
        if isinstance(val, bool):
            return 'y' if val else 'n'
    
        return str(val) if val is not None else m.group(0)
    
    return re.sub(r"\$\{([A-Za-z0-9_.\[\]]+)\}", repl, text)

def extract_block(lines, start_idx):
    """Extracts a block of lines until the matching endfor, supporting nesting."""
    block_lines = []
    i = start_idx
    nest_level = 0
    while i < len(lines):
        line = lines[i]
        if re.match(r"for\s+\w+\s+in\s+range\s+\d+\s+\d+", line.strip()) or re.match(r"for\s+\w+\s+in\s*\[.+\]", line.strip()):
            nest_level += 1
        elif line.strip().startswith("endfor"):
            if nest_level == 0:
                break
            nest_level -= 1
        block_lines.append(line)
        i += 1
    return block_lines, i

def expand_for_loops(template_text, macro_expander):
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
                block_text = '\n'.join(bl.replace(f"{{{var_name}}}", str(val)) for bl in block_lines)
                expanded_block = expand_for_loops(block_text, macro_expander)
                output_lines.append(expanded_block)
            i = block_end
        elif m_list:
            var_name, values = m_list.groups()
            items = [v.strip() for v in values.split(',')]
            block_lines, block_end = extract_block(lines, i + 1)
            for val in items:
                block_text = '\n'.join(bl.replace(f"{{{var_name}}}", val) for bl in block_lines)
                expanded_block = expand_for_loops(block_text, macro_expander)
                output_lines.append(expanded_block)
            i = block_end
        elif line.strip().startswith("endfor"):
            pass
        else:
            output_lines.append(line)
        i += 1
    return "\n".join(output_lines)


if __name__ == "__main__":
    # Parse command line arguments
    parser = argparse.ArgumentParser(description='Preprocess Kconfig files')
    parser.add_argument('--root-dir', default=os.getcwd(), 
                        help='Root directory of the project')
    parser.add_argument('--build-dir', default='build', 
                        help='Build directory for output files')
    args = parser.parse_args()
    
    # Convert to absolute paths
    root_dir = Path(args.root_dir).resolve()
    build_dir = Path(args.build_dir).resolve()
    
    # 1. Detect chip
    config_path = build_dir / "chip.config"
    core = load_config(config_path, 'CONFIG_TLK_CORE')
    soc = load_config(config_path, 'CONFIG_TLK_SOC_SERIES')
    board = load_config(config_path, 'CONFIG_TLK_BOARD')
    
    # 2. Load YAML vars
    all_yaml_files = []
    additional_options = []
    yaml_dir = root_dir / f"core/{core}/properties"
    yaml_vars = load_yaml_vars(yaml_dir)
    all_yaml_files.extend(yaml_dir.glob("*.yaml"))
    
    yaml_dir = root_dir / f"soc/{soc}/properties"
    merge(yaml_vars, load_yaml_vars(yaml_dir))
    all_yaml_files.extend(yaml_dir.glob("*.yaml"))
    
    if board:
        yaml_dir = root_dir / f"boards/{board}"
        merge(yaml_vars, load_yaml_vars(yaml_dir))
        all_yaml_files.extend(Path(yaml_dir).glob("*.yaml"))
    
    # Create build directory if it doesn't exist
    os.makedirs(build_dir, exist_ok=True)

    # 3. Process all Kconfig* files
    kconfig_dirs = [ # TODO: Made it automatically
        root_dir / 'core/configs',
        root_dir / 'samples/uart_demo',
        root_dir / 'system'
    ]

    for kconfig_dir in kconfig_dirs:
        for src_file in kconfig_dir.glob('Kconfig*'):
            # 4. First macro expansion
            text = src_file.read_text()
            text = expand_macros_from_yaml(text, yaml_vars, additional_options)
            # 5. Expand for-loops
            text = expand_for_loops(text, lambda t: expand_macros_from_yaml(t, yaml_vars, additional_options))
            # 6. Expand macros again (for those generated by for-loops)
            text = expand_macros_from_yaml(text, yaml_vars, additional_options)
            out_file = build_dir / f'Kconfig/{src_file.name}'
            os.makedirs(os.path.dirname(out_file), exist_ok=True)
            out_file.write_text(text)
            print(f"Generated {out_file}")

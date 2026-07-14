"""
Property descriptions in YAML files rules:

"""

import sys
import yaml
import re

INPUT_FILE = ""
OUT_FILE   = ""

#  Custom YAML integer constructor that preserves hexadecimal values as strings.
def construct_int_keeping_hex(loader, node):
    value_str = loader.construct_scalar(node)
    
    # Check if it matches the hex pattern
    if re.match(r'^0x[0-9a-fA-F]+$', value_str):
        return value_str
    
    return yaml.SafeLoader.construct_yaml_int(loader, node)

# Custom loader based on SafeLoader.  
# We create our own loader so we can modify how YAML values are interpreted.
class MySafeLoader(yaml.SafeLoader):
    pass

# Register a custom constructor for the YAML integer tag.  
# This replaces the default handling of integers so we can keep hex values unchanged.
MySafeLoader.add_constructor(
    'tag:yaml.org,2002:int',
    construct_int_keeping_hex
)

# Logging helper
def log(message, is_error=False):
    print(f"[properties_gen.py]: {INPUT_FILE} {message}")
    if is_error:
        sys.exit(1)

# Recursively parse nested YAML dict/list into a list of #define strings
def emit_defines(prefix, value, out):
    if isinstance(value, dict):
        # Recurse into each key, extending the define prefix
        for key, value in value.items():
            emit_defines(prefix + [key], value, out)

    elif isinstance(value, list):
        # Add elements count then each indexed element
        define_base = "UNISDK_" + "_".join(str(p).upper() for p in prefix)
        out.append(f"#define {define_base}_COUNT {len(value)}")
        
        for i, elem in enumerate(value):
            emit_defines(prefix + [i], elem, out)

    else:
        # Single element — Add a single #define
        define = "UNISDK_" + "_".join(str(p).upper() for p in prefix)
        if isinstance(value, bool):
            value = "true" if value else "false"

        out.append(f"#define {define} {value}")

# Build the full header content from parsed YAML data
def generate_header(data):
    if not isinstance(data, dict):
        log("Top-level YAML must be a dict", is_error=True)

    defines = []
    for key, value in data.items():
        emit_defines([key], value, defines)

    lines = []
    lines.append("// Auto-generated from YAML")
    lines.append("")
    for define in defines:
        lines.append(define)
    lines.append("")

    return "\n".join(lines)

def main():
    if len(sys.argv) != 3:
        log("Usage: properties_gen.py <input.yaml> <output.h>", is_error=True)

    global INPUT_FILE, OUT_FILE
    INPUT_FILE = sys.argv[1]
    OUT_FILE   = sys.argv[2]

    # Read and parse the input YAML file
    try:
        with open(INPUT_FILE, "r") as f:
            data = yaml.load(f, Loader=MySafeLoader)
    except FileNotFoundError:
        log(f"File not found: {INPUT_FILE}", is_error=True)
    except yaml.YAMLError as e:
        log(f"YAML parse error: {e}", is_error=True)
    except Exception as e:
        log(f"Failed to read input: {e}", is_error=True)

    if not data:
        log("Empty or null YAML — nothing to generate")
        return

    # Generate header content
    try:
        content = generate_header(data)
    except SystemExit:
        raise
    except Exception as e:
        log(f"Failed to generate header: {e}", is_error=True)

    # Write output header file
    try:
        with open(OUT_FILE, "w") as f:
            f.write(content)
    except Exception as e:
        log(f"Failed to write output: {e}", is_error=True)

    log("Generated")

if __name__ == "__main__":
    main()

"""
Register descriptions in YAML files rules:

{device name}:                           # Peripheral device name (prefix for structures and macros).
  reg:                                   # Root node for describing registers. (without _base_addr - user key)
    {reg name}: 0x{reg address}          # Register name and its address.
    {reg name}: 0x{reg address}          # Register name and its address.
    ...

{device name}:                           # Peripheral device name (prefix for structures and macros).
  reg:                                   # Mandatory: root node for describing registers.
    _base_addr: 0x{base address}         # Mandatory: base (starting) physical memory address of the device.
    _repeated:                           # Optional:  indicates that the current node is an array (repeated).
      count: {devices count}             # Mandatory: total number of elements (instances).
      offset: {main offset}              # Mandatory: standard step (offset) between elements.
      offset{i}: {offset for {i} device} # Optional:  individual offset (after previous element) for a specific element with index {i}. Range: [1, {devices count} - 1]

    {group name}:                        # Optional: name of the nested register group (generated as a structure).
      _repeated:                         # Optional: indicates that the current node is an array (repeated).
        count: {group repeat count}      # Mandatory: total number of elements (instances).

      {reg name}:                        # Register name.
        offset: 0x{reg offset}           # Mandatory: absolute register offset (addresses MUST go in ascending order!).
        len: {reg length}                # Mandatory: register size in bytes (allowed values: 1, 2, 4, 8).

      # {auto reservation}: {size}       # The generator automatically calculates memory "holes" and inserts reserved arrays.

      {reg name}:                        # Register name.
        offset: 0x{reg offset}           # Mandatory: absolute register offset (addresses MUST go in ascending order!).
        len: {reg length}                # Mandatory: register size in bytes (allowed values: 1, 2, 4, 8).
        bfld:                            # Optional:  node for describing bitfields inside the register.
          {bfld name}: {bfld size}       # Bitfield name and its size in bits.
          reserved: 7                    # Reserved bits (the total sum of bits in bfld must equal len * 8).

      {reg name}:                        # Register name.
        _repeated:                       # Optional:  indicates that the current node is an array (repeated).
          count: {reg array length}      # Mandatory: total number of elements (instances).
        offset: 0x{reg offset}           # Mandatory: absolute register offset (addresses MUST go in ascending order!).
        len: {reg length}                # Mandatory: register size in bytes (allowed values: 1, 2, 4, 8).
        bfld:                            # Optional:  node for describing bitfields inside the register.
          {bfld name}: {bfld size}       # Bitfield name and its size in bits.
          reserved0: 3                   # Reserved bits (the total sum of bits in bfld must equal len * 8).
          {bfld name}: {bfld size}       # Bitfield name and its size in bits.
          reserved1: 3                   # Reserved bits (the total sum of bits in bfld must equal len * 8).

      {reg name}:                        # Register name.
        _repeated:                       # Optional:  indicates that the current node is an array (repeated).
          count{i}: {reg array length}   # Mandatory: dimension size for multi-dimensional arrays (up to 3 dimensions allowed: count0, count1, count2).
          ...
        offset: 0x{reg offset}           # Mandatory: absolute register offset (addresses MUST go in ascending order!).
        len: {reg length}                # Mandatory: register size in bytes (allowed values: 1, 2, 4, 8).
      ...

    {reg name}:                          # Register name.
        offset: 0x{reg offset}           # Mandatory: absolute register offset (addresses MUST go in ascending order!).
        len: {reg length}                # Mandatory: register size in bytes (allowed values: 1, 2, 4, 8).
    ...

{device name}:                           # Peripheral device name (prefix for structures and macros).
  reg:                                   # Mandatory: root node for describing registers.
    ...
...

The analog register description uses the same format with multiple restrictions:
    - Analog register description starts with 'areg' key instead of 'reg'.
    - It has no '_base_addr' (it is zero by default).
    - It has no '_repeated' key at the root of 'areg'.
    - It can't have groups.
    - Aregs can't have 'bfld' and '_repeated' at the same time.
    - Areg length 8 bytes is unsupported.
"""


import logging
import sys
from pathlib import Path

try:
    from scripts.readers.yaml_reader import YamlReader
except ModuleNotFoundError:
    import yaml

    class YamlReader:
        def __init__(self, filename):
            with open(filename, "r") as f:
                self.data = yaml.safe_load(f)

INDENT="    "

logger = logging.getLogger(__name__)

def get_c_type(byte_len):
    return {1: "uint8_t", 2: "uint16_t", 4: "uint32_t", 8: "uint64_t"}.get(byte_len, "uint8_t")

def is_reserved_bfld(name: str) -> bool:
    return name.lower().startswith("reserved")

def get_bfld_width(b_name: str, b_desc) -> int:
    """Return bitfield width.

    YAML supports both the legacy form:
        field_name: 2

    and the extended form:
        field_name:
          width: 2
          values:
            value_name: 0
    """
    if b_desc is None:
        logger.error(f"Bitfield '{b_name}' has no width")
        sys.exit(1)

    if isinstance(b_desc, dict):
        for key in ("width", "bits", "len"):
            if key in b_desc:
                return int(b_desc[key])

        logger.error(f"Bitfield '{b_name}' is a map but has no 'width' key")
        sys.exit(1)

    return int(b_desc)


def get_bfld_named_values(b_desc) -> dict:
    """Return named values for a multi-bit field, if present."""
    if not isinstance(b_desc, dict):
        return {}

    for key in ("values", "enum", "mode", "modes"):
        values = b_desc.get(key)
        if isinstance(values, dict):
            return values

    return {}


def get_flag_enum_bfld_values(device_name: str, path: list[str], bfld: dict) -> list[tuple[str, int]]:
    """Return enum constants for 1-bit, non-reserved bitfields only."""
    bit_offset = 0
    values: list[tuple[str, int]] = []
    value_prefix = f"TLK_{device_name.upper()}_{'_'.join(part.upper() for part in path)}"

    for b_name, b_desc in bfld.items():
        bit_width = get_bfld_width(b_name, b_desc)

        if bit_width == 1 and not is_reserved_bfld(b_name):
            values.append((f"{value_prefix}_{b_name.upper()}", bit_offset))

        bit_offset += bit_width

    return values


def get_wide_bfld_enums(device_name: str, path: list[str], bfld: dict) -> list[tuple[str, str, list[tuple[str, int]]]]:
    """Return separate enum definitions for multi-bit fields with named values.

    Each returned tuple is: (bitfield_name, enum_tag, enum_values).
    Values are unshifted field values, suitable for assigning to the bitfield
    member itself, e.g. reg.control1_bit.data_lane = TLK_SPI_CONTROL1_DATA_LANE_QUAD.
    """
    result: list[tuple[str, str, list[tuple[str, int]]]] = []
    value_prefix = f"TLK_{device_name.upper()}_{'_'.join(part.upper() for part in path)}"
    tag_prefix = f"tlk_{device_name.lower()}_{'_'.join(part.lower() for part in path)}"

    for b_name, b_desc in bfld.items():
        bit_width = get_bfld_width(b_name, b_desc)
        named_values = get_bfld_named_values(b_desc)

        if bit_width <= 1 or is_reserved_bfld(b_name) or not named_values:
            continue

        max_value = (1 << bit_width) - 1
        enum_values: list[tuple[str, int]] = []

        for value_name, value in named_values.items():
            value_int = int(value)
            if value_int < 0 or value_int > max_value:
                logger.error(
                    f"Value '{value_name}: {value_int}' does not fit into "
                    f"{bit_width}-bit field '{b_name}' in '{'.'.join(path)}'"
                )
                sys.exit(1)

            enum_values.append((f"{value_prefix}_{b_name.upper()}_{str(value_name).upper()}", value_int))

        result.append((b_name, f"{tag_prefix}_{b_name.lower()}_e", enum_values))

    return result


def get_wide_bfld_enum_tag(device_name: str, path: list[str], bfld_name: str, bfld_desc) -> str | None:
    """Return the separate enum tag for a named multi-bit field, otherwise None."""
    bit_width = get_bfld_width(bfld_name, bfld_desc)
    if bit_width <= 1 or is_reserved_bfld(bfld_name) or not get_bfld_named_values(bfld_desc):
        return None

    return f"tlk_{device_name.lower()}_{'_'.join(part.lower() for part in path)}_{bfld_name.lower()}_e"

def get_enum_tag(device_name: str, path: list[str]) -> str:
    return f"tlk_{device_name.lower()}_{'_'.join(part.lower() for part in path)}_e"


def emit_enum_definition(device_name: str, path: list[str], bfld: dict) -> list[str]:
    """Emit enum definitions for a bitfield register.

    - The register-level enum contains only 1-bit flag masks.
    - Each multi-bit field with YAML named values gets its own separate enum.
    """
    lines = []

    flag_values = get_flag_enum_bfld_values(device_name, path, bfld)
    if flag_values:
        enum_tag = get_enum_tag(device_name, path)
        max_name_len = max(len(name) for name, _ in flag_values)

        lines.append(f"typedef enum {enum_tag} {{")
        for value_name, bit_offset in flag_values:
            lines.append(f"{INDENT}{value_name:<{max_name_len}} = TLK_BIT({bit_offset}),")
        lines.append(f"}} {enum_tag};")

    for _bfld_name, enum_tag, enum_values in get_wide_bfld_enums(device_name, path, bfld):
        if lines:
            lines.append("")

        max_name_len = max(len(name) for name, _ in enum_values)
        lines.append(f"typedef enum {enum_tag} {{")
        for value_name, value in enum_values:
            lines.append(f"{INDENT}{value_name:<{max_name_len}} = 0x{value:x}U,")
        lines.append(f"}} {enum_tag};")

    return lines


def collect_enum_definitions(device_name: str, data: dict, path=None) -> list[str]:
    """Collect public enum definitions for all registers that contain bitfields."""
    if path is None:
        path = []

    lines: list[str] = []

    for name, val in data.items():
        if name.startswith('_') or not isinstance(val, dict):
            continue

        field_path = path + [name]

        if "offset" in val and "bfld" in val:
            enum_lines = emit_enum_definition(device_name, field_path, val["bfld"])
            if enum_lines:
                lines.extend(enum_lines)
                lines.append("")
        elif "offset" not in val:
            lines.extend(collect_enum_definitions(device_name, val, field_path))

    return lines


def emit_register_overlay(device_name: str, path: list[str], reg_name: str, reg_len: int, bfld: dict, indent: str, reg_comment: str, array_postfix: str = "") -> list[str]:
    """Emit an additional register overlay for a bitfield register.

    If the register has at least one enum value, emit the enum-typed bitfield
    overlay. If no enum values can be generated, emit the fixed-width raw
    integer member so the union still has a whole-register access primitive.
    """
    values = get_flag_enum_bfld_values(device_name, path, bfld)

    if values:
        enum_tag = get_enum_tag(device_name, path)
        bit_width = reg_len * 8

        return [
            f"{indent}    struct {{",
            f"{indent}        enum {enum_tag} __attribute__((packed)) {reg_name} : {bit_width};",
            f"{indent}    }};{reg_comment}",
        ]

    return [f"{indent}    {get_c_type(reg_len)} {reg_name}{array_postfix};{reg_comment}"]

def get_node_offset(name, val):
    if "offset" in val:
        return int(str(val["offset"]), 0)
    
    for k, v in val.items():
        if not k.startswith('_') and isinstance(v, dict):
            offset = get_node_offset(k, v)

            if offset is not None:
                return offset
    
    logger.error(f"Node '{name}' has no offset and no child offsets")
    sys.exit(1)

def parse_repeated(repeated_content):
    if not repeated_content:
        return []
    
    if "count" in repeated_content:
        return [int(repeated_content["count"])]
    
    counts = []

    for i in range(4):
        key = f"count{i}"

        if key in repeated_content:
            if len(counts) != i:
                logger.error(f"Non-sequential dimension '{key}'")
                sys.exit(1)

            counts.append(int(repeated_content[key]))
        else:
            break
    
    if not counts:
        logger.error("Missing 'count' in _repeated block")
        sys.exit(1)
    if len(counts) > 3:
        logger.error("Max 3 counts allowed")
        sys.exit(1)

    return counts

def process_node(data, current_pointer, reserved_idx, base_addr, indent_lvl=1, device_name="", path=None):
    if path is None:
        path = []

    lines = []
    indent = "    " * indent_lvl
    fields = {k: v for k, v in data.items() if not k.startswith('_')}
    pointer = current_pointer

    for name, val in fields.items():
        if not isinstance(val, dict):
            continue

        field_path = path + [name]
        repeated_content = val.get("_repeated", {})
        counts = parse_repeated(repeated_content)
        total_elements = 1

        for d in counts:
            total_elements *= d
        
        array_postfix = "".join([f"[{d}]" for d in counts])

        node_start = get_node_offset(name, val)
        
        if node_start < pointer:
            logger.error(f"Overlap error: '{name}' offset {hex(node_start)} < current {hex(pointer)}")
            sys.exit(1)

        if node_start > pointer:
            gap = node_start - pointer
            reserved_name = f"reserved{reserved_idx[0]}"

            reserved_comment = f" // address: {hex(base_addr + pointer)}, offset: {hex(pointer)}"
            lines.append(f"{indent}uint8_t {reserved_name}[{hex(gap)}];{reserved_comment}" if gap > 1 else f"{indent}uint8_t {reserved_name};{reserved_comment}")
            
            reserved_idx[0] += 1
            pointer = node_start

        reg_comment = f" // address: {hex(base_addr + node_start)}, offset: {hex(node_start)}"

        if "offset" in val:
            if "len" not in val:
                logger.error(f"Register '{name}' missing 'len'")
                sys.exit(1)

            reg_len = int(val["len"])

            if reg_len not in [1, 2, 4, 8]:
                logger.error(f"Invalid len {reg_len} for '{name}'")
                sys.exit(1)

            if "bfld" in val:
                lines.append(f"{indent}union {{")
                lines.append(f"{indent}    struct {{")

                bit_sum = 0

                for b_name, b_bits in val["bfld"].items():
                    bit_width = get_bfld_width(b_name, b_bits)
                    enum_tag = get_wide_bfld_enum_tag(device_name, field_path, b_name, b_bits)

                    if enum_tag:
                        lines.append(f"{indent}        enum {enum_tag} __attribute__((packed)) {b_name} : {bit_width};")
                    else:
                        lines.append(f"{indent}        unsigned {b_name} : {bit_width};")
                    bit_sum += bit_width
                
                if bit_sum != reg_len * 8:
                    logger.error(f"Bitfield sum {bit_sum} != {reg_len*8} in '{name}'")
                    sys.exit(1)
                
                lines.append(f"{indent}    }} __attribute__((packed)) {name}_bit{array_postfix};")
                lines.extend(emit_register_overlay(device_name, field_path, name, reg_len, val["bfld"], indent, reg_comment, array_postfix))
                lines.append(f"{indent}}};")
            else:
                lines.append(f"{indent}{get_c_type(reg_len)} {name}{array_postfix};{reg_comment}")
            
            pointer += reg_len * total_elements

        else:
            lines.append(f"{indent}struct {{")

            sub_body, end_pointer = process_node(val, node_start, reserved_idx, base_addr, indent_lvl + 1, device_name, field_path)

            lines.extend(sub_body)
            lines.append(f"{indent}}} __attribute__((packed)) {name}{array_postfix};{reg_comment}")
            
            unit_size = end_pointer - node_start
            pointer += unit_size * total_elements

    return lines, pointer

def process_device_reg(name: str, data: dict[str, dict]) -> list[str]:
    """Process the digital registers of the device."""

    base_addr = data.get('_base_addr')

    if not isinstance(base_addr , int):
        logger.warning(f"Device '{name}' has missing or invalid '_base_addr'")
        return []

    name_upper = name.upper()
    name_lower = name.lower()

    lines: list[str] = []

    enum_lines = collect_enum_definitions(name, data)
    if enum_lines:
        lines.extend(enum_lines)

    lines.append("typedef struct __attribute__((packed)) {")

    reserved_idx = [0]
    body, _ = process_node(data, 0, reserved_idx, base_addr, device_name=name)
    lines.extend(body)

    struct_name = f"tlk_{name_lower}_reg_t"
    lines.append(f"}} {struct_name};")
    lines.append("")
    lines.append(f"#define TLK_{name_upper}_BASE_ADDR ({hex(base_addr)}U)")

    if "_repeated" in data:
        repeated_content = data.get("_repeated", {})
        count = repeated_content.get("count")
        offset = repeated_content.get("offset")

        if not isinstance(count, int) or not isinstance(offset, int):
            logger.error(f"Repeated device '{name}' needs a valid 'count' and 'offset' keys")
            sys.exit(1)

        addr = base_addr
        for i in range(count):
            if i > 0:
                addr += repeated_content.get(f"offset{i}", offset)
            lines.append(f"#define TLK_{name_upper}{i}_ADDR ({hex(addr)}U)")
        
        lines.append(f"#define tlk_{name_lower}_reg(i) (*(volatile {struct_name} *) ( \\")
        lines.extend([f"{INDENT}((i) == {j}) ? TLK_{name_upper}{j}_ADDR : \\" for j in range(count)])
        lines.append(f"{INDENT}0 ))") #TODO: replace it with assert

    else:
        lines.append(f"#define tlk_{name_lower}_reg (*(volatile {struct_name} *) TLK_{name_upper}_BASE_ADDR)")

    return lines

def process_device_areg(name: str, aregs: dict[str, dict]) -> list[str]:
    lines: list[str] = []
    pointer = 0
    reserved_idx = 0

    for name, data in aregs.items():
        if not isinstance(data, dict) or name.startswith("_"):
            continue

        if "_repeated" in data:
            repeated_content = data.get("_repeated", {})
            count = repeated_content.get("count")

            if not isinstance(count, int):
                logger.error(f"Repeated device '{name}' needs a valid 'count' key")
                sys.exit(1)
        else:
            count = 1

        node_start = get_node_offset(name, data)
        
        if node_start < pointer:
            logger.error(f"Overlap error: '{name}' offset {hex(node_start)} < current {hex(pointer)}")
            sys.exit(1)

        if node_start > pointer:
            gap = node_start - pointer
            reserved_name = f"reserved{reserved_idx}"

            reserved_comment = f" // address: {hex(pointer)}"
            lines.append(f"// uint8_t {reserved_name}[{hex(gap)}];{reserved_comment}" if gap > 1 else f"// uint8_t {reserved_name};{reserved_comment}")
            lines.append("")
            
            reserved_idx += 1
            pointer = node_start

        if "offset" in data:
            if "len" not in data:
                logger.error(f"Register '{name}' missing 'len'")
                sys.exit(1)

            reg_len = int(data["len"])

            if reg_len not in [1, 2, 4]:
                logger.error(f"Invalid len {reg_len} for '{name}'")
                sys.exit(1)

            reg_addr = f"0x{node_start:02x}"

            if "bfld" in data:
                lines.append(f"#define TLK_AREG_{name.upper()} {reg_addr}")
                lines.append(f"#define TLK_AREG_{reg_addr}_SIZE {reg_len * 8}")
                lines.append("")

                lines.append(f"typedef union tlk_areg_{name} {{")
                lines.append(f"{INDENT}struct {{")

                bit_sum = 0

                for b_name, b_bits in data["bfld"].items():
                    bit_width = get_bfld_width(b_name, b_bits)

                    lines.append(f"{INDENT*2}unsigned {b_name} : {bit_width};")
                    bit_sum += bit_width
                
                if bit_sum != reg_len * 8:
                    logger.error(f"Bitfield sum {bit_sum} != {reg_len*8} in '{name}'")
                    sys.exit(1)
                
                lines.append(f"{INDENT}}} __attribute__((packed)) bit;")
                lines.append(f"{INDENT}{get_c_type(reg_len)} raw;")
                lines.append(f"}} tlk_areg_{reg_addr};")
            else:
                if count == 1:
                    lines.append(f"#define TLK_AREG_{name.upper()} 0x{node_start:02x}")
                else:
                    lines.append(f"#define TLK_AREG_{name.upper()}(offset) (TLK_AREG_{name.upper()}0 + offset * {reg_len})")
                    for i in range(count):
                        lines.append(f"#define TLK_AREG_{name.upper()}{i} 0x{(node_start + i * reg_len):02x}")
            
            pointer += reg_len * count
            lines.append("")

        else:
            logger.error(f"{name} doesn't have an offset!")
            sys.exit(1)

    return lines

def process_data(raw: dict) -> list[str]:
    """Process the root keys of YAML that represent the device."""
    
    lines: list[str] = []
    lines.append(f"#ifndef {guard}")
    lines.append(f"#define {guard}")
    lines.append("")
    lines.append("#include <stdint.h>")
    lines.append("#include <common/include/tlk_bit.h>")
    lines.append("")

    for device, data in raw.items():
        if not isinstance(device, str) or not isinstance(data, dict):
            logger.warning(f"Device '{device}' has an inconsistent name, or its data is not a dictionary")
            continue

        if "reg" in data:
            result = process_device_reg(device, data["reg"])
            lines.extend(result)
        
        if "reg" in data and "areg" in data:
            lines.append("\n\n")

        if "areg" in data:
            result = process_device_areg(device, data["areg"])
            lines.extend(result)

        lines.append("\n\n")

    lines.pop()
    lines.append("")
    lines.append("#endif")

    return lines


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit(1)
    
    logging.basicConfig(
        format="[%(levelname)s] %(funcName)s in %(filename)s:%(lineno)d: %(message)s",
    )

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    output_filename = Path(output_file).stem
    guard = f"TLK_{output_filename.upper()}_REGISTERS_H_"

    raw = YamlReader(input_file).data

    lines = process_data(raw)

    with open(output_file, 'w') as f:
        f.write("\n".join(lines))

    logger.info(f"Generated: {output_file}")

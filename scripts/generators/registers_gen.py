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
"""


import sys
import yaml

INPUT_FILE = ""
OUT_FILE = ""

def log(message, is_error=False):
    print(f"[sdk_gen.py]: {INPUT_FILE} {message}")
    if is_error:
        sys.exit(1)

def get_c_type(byte_len):
    return {1: "uint8_t", 2: "uint16_t", 4: "uint32_t", 8: "uint64_t"}.get(byte_len, "uint8_t")

def get_node_offset(name, val):
    if "offset" in val:
        return int(str(val["offset"]), 0)
    
    for k, v in val.items():
        if not k.startswith('_') and isinstance(v, dict):
            offset = get_node_offset(k, v)

            if offset is not None:
                return offset
    
    log(f"Node '{name}' has no offset and no child offsets", True)

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
                log(f"Non-sequential dimension '{key}'", True)

            counts.append(int(repeated_content[key]))
        else:
            break
    
    if not counts:
        log("Missing 'count' in _repeated block", True)
    if len(counts) > 3:
        log("Max 3 counts allowed", True)

    return counts

def process_node(data, current_pointer, reserved_idx, base_addr, indent_lvl=1):
    lines = []
    indent = "    " * indent_lvl
    fields = {k: v for k, v in data.items() if not k.startswith('_')}
    pointer = current_pointer

    for name, val in fields.items():
        if not isinstance(val, dict):
            continue

        # repeated attribute
        repeated_content = val.get("_repeated", {})
        counts = parse_repeated(repeated_content)
        total_elements = 1

        for d in counts:
            total_elements *= d
        
        array_postfix = "".join([f"[{d}]" for d in counts])

        # new offset
        node_start = get_node_offset(name, val)
        
        if node_start < pointer:
            log(f"Overlap error: '{name}' offset {hex(node_start)} < current {hex(pointer)}", True)

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
                log(f"Register '{name}' missing 'len'", True)

            reg_len = int(val["len"])

            if reg_len not in [1, 2, 4, 8]:
                log(f"Invalid len {reg_len} for '{name}'", True)

            if "bfld" in val:
                lines.append(f"{indent}union {{")
                lines.append(f"{indent}    struct {{")

                bit_sum = 0

                for b_name, b_bits in val["bfld"].items():
                    if b_bits is None:
                        log(f"Bitfield '{b_name}' in '{name}' has no width", True)

                    lines.append(f"{indent}        unsigned {b_name} : {b_bits};")
                    bit_sum += int(b_bits)
                
                if bit_sum != reg_len * 8:
                    log(f"Bitfield sum {bit_sum} != {reg_len*8} in '{name}'", True)
                
                lines.append(f"{indent}    }} __attribute__((packed)) {name}_bit{array_postfix};")
                lines.append(f"{indent}    {get_c_type(reg_len)} {name}{array_postfix};{reg_comment}")
                lines.append(f"{indent}}};")
            else:
                lines.append(f"{indent}{get_c_type(reg_len)} {name}{array_postfix};{reg_comment}")
            
            pointer += reg_len * total_elements

        else:
            lines.append(f"{indent}struct {{")

            sub_body, end_pointer = process_node(val, node_start, reserved_idx, base_addr, indent_lvl + 1)

            lines.extend(sub_body)
            lines.append(f"{indent}}} __attribute__((packed)) {name}{array_postfix};{reg_comment}")
            
            unit_size = end_pointer - node_start
            pointer += unit_size * total_elements

    return lines, pointer

def process_device(name, data):
    if not isinstance(data, dict) or "reg" not in data:
        log(f"Device \'{name}\' missing \'reg\' key")
        return []
    
    reg_content = data["reg"]

    if "_base_addr" not in reg_content:
        log(f"Device \'{name}\' missing \'_base_addr\' key")
        return []

    name_caps = name.upper()
    guard = f"TLK_{name_caps}_REGISTERS_H_"
    base_addr = int(str(reg_content["_base_addr"]), 0)
    
    header_file = []
    header_file.append(f"#ifndef {guard}")
    header_file.append(f"#define {guard}")
    header_file.append("")
    header_file.append("#include <stdint.h>")
    header_file.append("")
    header_file.append("typedef struct __attribute__((packed)) {")
    
    reserved_idx = [0]
    body, _ = process_node(reg_content, 0, reserved_idx, base_addr)
    header_file.extend(body)
    
    struct_name = f"tlk_{name.lower()}_reg_t"
    header_file.append(f"}} {struct_name};")
    header_file.append("")
    header_file.append(f"#define TLK_{name_caps}_BASE_ADDR ({hex(base_addr)}U)")

    if "_repeated" in reg_content:
        repeated_content = reg_content["_repeated"]

        if "count" not in repeated_content or "offset" not in repeated_content:
            log(f"Repeated device \'{name}\' needs \'count\' and \'offset\' keys", True)
        
        count = int(repeated_content["count"])
        offset = int(str(repeated_content["offset"]), 0)

        addr = base_addr
        for i in range(count):
            if i > 0:
                addr += int(str(repeated_content.get(f"offset{i}", offset)), 0)
            header_file.append(f"#define TLK_{name_caps}{i}_ADDR ({hex(addr)}U)")
        
        ternary_chain = " : \\\n    ".join([f"((i) == {j}) ? TLK_{name_caps}{j}_ADDR" for j in range(count)]) + " : \\\n    0"
        header_file.append(f"#define tlk_{name.lower()}_reg(i) (*(volatile {struct_name} *) ( \\\n    {ternary_chain} ))")

    else:
        header_file.append(f"#define tlk_{name.lower()}_reg (*(volatile {struct_name} *) TLK_{name_caps}_BASE_ADDR)")

    header_file.append(f"\n#endif")
    return header_file


def main():
    if len(sys.argv) != 3:
        sys.exit(1)
    
    global INPUT_FILE
    global OUT_FILE

    INPUT_FILE = sys.argv[1]
    OUT_FILE = sys.argv[2]

    try:
        with open(INPUT_FILE, 'r') as f:
            raw = yaml.safe_load(f)           #TODO: make by yaml parser

        if not raw:
            log("Empty")
            return

        header_file = []
        for device, data in raw.items():
            result = process_device(device, data)

            if result:
                header_file.extend(result)
                header_file.append("\n")

        if header_file:
            with open(OUT_FILE, 'w') as f:
                f.write("\n".join(header_file))

            log("Generated")

    except Exception as e:
        log(f"Error: {str(e)}", True)

if __name__ == "__main__":
    main()
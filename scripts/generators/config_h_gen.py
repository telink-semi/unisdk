import re
import sys
from pathlib import Path

CONFIG_LINE_RE = re.compile(r'^CONFIG_([A-Za-z0-9_]+)=(.*)$')
QUOTED_STRING_RE = re.compile(r'^"(.*)"$')

def parse_config_file(path: Path, values: dict) -> None:
    if not path.is_file():
        print(f"CRITICAL ERROR: config file not found: {path}", file=sys.stderr)
        while (1):
            None
        sys.exit(1)

    with path.open("r", encoding="utf-8") as f:
        for line in f:
            match = CONFIG_LINE_RE.match(line.strip())
            if not match:
                continue

            name, value = match.group(1), match.group(2)
            full_name = f"CONFIG_{name}"

            if value == "y":
                values[full_name] = "1"
            elif value == "n":
                values[full_name] = "0"
            else:
                quoted = QUOTED_STRING_RE.match(value)
                values[full_name] = quoted.group(1) if quoted else value


def main() -> int:
    output_header = Path(sys.argv[1])
    config_files = [Path(p) for p in sys.argv[2:]]

    values: dict = {}
    for cfg in config_files:
        parse_config_file(cfg, values)

    with output_header.open("w", encoding="utf-8") as f:
        f.write("/* Auto-generated from .config */\n")
        for name, value in values.items():
            f.write(f"#define {name} {value}\n")

    return 0


if __name__ == "__main__":
    sys.exit(main())
"""
Parse all YAML files once and store the parsed data in a Python pickle file.

The output file contains the already parsed Python data structure,
so other scripts can load it directly with pickle.load() without parsing
the YAML files again.

Usage:
    python yaml_dict_gen.py <output.pkl> <file1.yaml> <file2.yaml> ...

To use the generated file:
    with open("properties.pkl", "rb") as f:
        properties = pickle.load(f)
"""

import argparse
import pickle
from pathlib import Path

import yaml


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("output", type=Path)
    parser.add_argument("yaml_files", nargs="+", type=Path)

    args = parser.parse_args()

    properties = {}

    for yaml_file in args.yaml_files:
        with yaml_file.open("r", encoding="utf-8") as file:
            data = yaml.safe_load(file)

        if data:
            properties.update(data)

    args.output.parent.mkdir(parents=True, exist_ok=True)

    with args.output.open("wb") as file:
        pickle.dump(
            properties,
            file,
            protocol=pickle.HIGHEST_PROTOCOL,
        )


if __name__ == "__main__":
    main()
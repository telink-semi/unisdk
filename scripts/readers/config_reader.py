import logging
from pathlib import Path
from typing import Any
import re


logger = logging.getLogger(__name__)


class ConfigReader:
    """Reader for Linux-style ".config" config files.

    Supports parsing both the `CONFIG_FOO=y` and the `# CONFIG_FOO is not set`
    forms that are produced by Kconfig-based build systems.

    Parsed keys have no `CONFIG_` prefix.
    Parsed values are converted to Python types:
    - `y` becomes `True`
    - `n` / unset becomes `False`
    - numeric values (decimal + 0x hex) become `int`
    - quoted values remain as `str` (quotes are stripped)
    - unrecognized values fallback to `None`
    """

    def __init__(self, path: str | Path) -> None:
        self.configs: dict[str, Any] = {}
        self.path: Path = Path(path)

        self._load()

    def _load(self) -> None:
        """Load a .config file and parse options into a dict."""

        if not self.path.exists():
            logger.error(f"Config file not found: {self.path}")
            return

        try:
            with self.path.open("r", encoding="utf-8", errors="replace") as f:
                for lineno, line in enumerate(f, 1):
                    line = line.strip()
                    if not line or line.startswith("#") and "is not set" not in line:
                        continue

                    # Handle: # CONFIG_FOO is not set
                    m = re.match(r"^#\s*CONFIG_([A-Za-z0-9_]+)\s+is not set$", line)
                    if m:
                        self.configs[m.group(1)] = False
                        continue

                    # Handle: CONFIG_FOO=... (value may contain = if quoted)
                    m = re.match(r"^CONFIG_([A-Za-z0-9_]+)=(.*)$", line)
                    if not m:
                        continue

                    raw_value = m.group(2).strip()
                    value = self._parse_value(raw_value)

                    if value is None:
                        logger.warning(f"Unrecognized type for value {raw_value} in {self.path}:{lineno}")
                    else:
                        self.configs[m.group(1)] = value

        except Exception as e:
            logger.error(f"Error reading {self.path}: {e}")

    def _parse_value(self, raw: str) -> Any:
        """Convert raw string value into a Python type."""

        if raw == "y":
            return True
        if raw == "n":
            return False

        if len(raw) >= 2 and ((raw[0] == raw[-1] == '"') or (raw[0] == raw[-1] == "'")):
            inner = raw[1:-1]
            inner = inner.replace('\\"', '"').replace("\\'", "'")
            inner = inner.replace('\\n', '\n').replace('\\t', '\t')
            return inner

        try:
            return int(raw, 0)
        except ValueError:
            pass

        return None

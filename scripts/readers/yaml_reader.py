import logging
from pathlib import Path
import yaml


logger = logging.getLogger(__name__)


class YamlReader:
    """Reader for yaml files"""

    def __init__(self, path: str | Path) -> None:
        self.data: dict = {}
        self.path = Path(path)

        self._load()

    def _load(self) -> None:
        """Load a YAML file into a dict (empty dict on error)."""

        if not self.path.exists():
            logger.error(f"Yaml file not found: {self.path}")
            return
        
        try:
            with self.path.open("r", encoding="utf-8", errors="replace") as f:
                data = yaml.safe_load(f)
                if data is None:
                    logger.warning(f"File is empty: {self.path}")
                    return
                elif not isinstance(data, dict):
                    logger.error(f"Top-level is not a dict: {self.path}")
                    return
                
                self.data = data
        except yaml.YAMLError as e:
            logger.error(f"YAML parse error in {self.path}: {e}")
        except Exception as e:
            logger.error(f"Error reading {self.path}: {e}")

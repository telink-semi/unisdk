import logging
from pathlib import Path
import argparse
import sys


from scripts.pinmux.data.path_manager import PathManager
from scripts.pinmux.data.pinmux_manager import PinmuxManager


# Parse command line arguments
parser = argparse.ArgumentParser(description="Pinmux Configuration Tool")
parser.add_argument("--root_dir", nargs='?', type=Path, help="Root directory")
parser.add_argument("--build_dir", nargs='?', type=Path, help="Build directory")
parser.add_argument("--project_dir", required=True, type=Path, help="Project directory")
parser.add_argument("--try-skip-ui", action="store_true", help="Skip opening the UI if possible")
parser.add_argument("--no-ui", action="store_true", help="Prohibit opening the UI")

group = parser.add_mutually_exclusive_group()
group.add_argument("--verbose", action="store_const", dest="log_level", const=logging.INFO, help="Set logging level to INFO")
group.add_argument("--debug", action="store_const", dest="log_level", const=logging.DEBUG, help="Set logging level to DEBUG")
parser.set_defaults(log_level=logging.WARNING)

args = parser.parse_args()


logging.basicConfig(
    level=args.log_level,
    format="[%(levelname)s] %(funcName)s in %(filename)s:%(lineno)d: %(message)s",
)

logger = logging.getLogger(__name__)


path_manager = PathManager(root_dir=args.root_dir, build_dir=args.build_dir, project_dir=args.project_dir)

manager = PinmuxManager(path_manager)
manager.read()

if args.try_skip_ui and manager.is_valid and not manager.is_modified:
    logger.info("Updating the pinmux before skipping the UI opening...")
    manager.write()
    sys.exit(0)

if args.no_ui:
    if manager.is_valid:
        if manager.is_modified:
            logger.info("Updating the pinmux...")
            manager.write()
        else:
            manager.update_header()
        logger.info("Closing the pinmux configurator without errors...")
        sys.exit(0)
    else:
        logger.error("Closing the pinmux configurator with errors...")
        sys.exit(1)

from .app import PinmuxApp
PinmuxApp(manager).run()

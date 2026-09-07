import logging
from pathlib import Path


logger = logging.getLogger(__name__)


class PathManager:
    """Validate and compose the main paths used by the application."""

    root_dir: Path
    build_dir: Path
    project_dir: Path | None

    def __init__(self, root_dir: Path | None = None, build_dir: Path | None = None, project_dir: Path | None = None) -> None:
        self.root_dir = Path(__file__).resolve().parents[3]
        
        if root_dir is not None:
            root_dir = root_dir.expanduser().resolve()

            if not root_dir.is_dir():
                logger.warning(f"Provided root_dir is not a directory: {root_dir}")
            else:
                self.root_dir = root_dir

        logger.info(f"Set the root_dir to {self.root_dir}")


        self.build_dir = self.root_dir / "build"

        if build_dir is not None:
            build_dir = build_dir.expanduser().resolve()

            if not build_dir.is_dir():
                logger.warning(f"Provided build_dir is not a directory: {build_dir}")
            else:
                self.build_dir = build_dir
        
        logger.info(f"Set the build_dir to {self.build_dir}")


        self.project_dir = None

        if project_dir is not None:
            project_dir = project_dir.expanduser().resolve()

            if not project_dir.is_dir():
                logger.warning(f"Provided project_dir is not a directory: {project_dir}")
            else:
                self.project_dir = project_dir
        else:
            logger.warning("Project directory was not provided!")
        
        logger.info(f"Set the project_dir to {self.project_dir}")
        
import copy

from scripts.pinmux.data.path_manager import PathManager
from scripts.pinmux.data.pinmux_context import PinmuxContext
from scripts.pinmux.data.pinmux_data import PinmuxData
from scripts.pinmux.data.pinmux_identifier import PinmuxType
from scripts.readers.config_reader import ConfigReader


class PinmuxManager:
    """Delegate the high-level logic of the pinmux application."""

    build_pinmux: PinmuxData | None = None
    project_pinmux: PinmuxData | None = None
    default_pinmux: PinmuxData
    current_pinmux: PinmuxData

    context: PinmuxContext

    def __init__(self, path_manager: PathManager) -> None:
        self.path_manager = path_manager

    @property
    def is_modified(self) -> bool:
        return self.build_pinmux is None or self.current_pinmux != self.build_pinmux
    
    @property
    def is_valid(self) -> bool:
        return self.current_pinmux.is_valid(self.context)
    
    def validate(self) -> None:
        self.current_pinmux.validate(self.context)

    def read(self) -> None:
        dotconfig_path = self.path_manager.build_dir / ".config"
        self.dotconfig = ConfigReader(dotconfig_path)

        self.context = PinmuxContext(self.path_manager, self.dotconfig)

        self.context.read_id()
        self.context.read_functions()
        self.context.read_ports()

        self.default_pinmux = self.context.default_pinmux_data
        self.current_pinmux = copy.deepcopy(self.default_pinmux)
    

        self.project_pinmux = PinmuxData()

        generic_project_pinmux_path = self.path_manager.project_dir / ".pinmux"
        if generic_project_pinmux_path.exists():
            self.project_pinmux.read(generic_project_pinmux_path, self.context)

        project_pinmux_path = self.path_manager.project_dir / f"{self.context.id.prefix}.pinmux"
        if project_pinmux_path.exists():
            project_pinmux = PinmuxData()
            project_pinmux.read(project_pinmux_path, self.context)
            self.project_pinmux.override_by(project_pinmux, self.context)

        self.current_pinmux.override_by(self.project_pinmux, self.context)            


        build_pinmux_path = self.path_manager.build_dir / f"{self.context.id.prefix}.pinmux"
        if build_pinmux_path.exists():
            self.build_pinmux = PinmuxData()
            self.build_pinmux.read(build_pinmux_path, self.context)
            self.current_pinmux.override_by(self.build_pinmux, self.context)
        

        self.context.apply_pinmux(self.current_pinmux)

    def update_header(self) -> None:
        build_pinmux_h_path = self.path_manager.build_dir / "pinmux.h"
        self.current_pinmux.write_header(build_pinmux_h_path, self.context)

    def write(self) -> None:
        build_pinmux_path = self.path_manager.build_dir / f"{self.context.id.prefix}.pinmux"
        self.current_pinmux.write(build_pinmux_path)
        self.update_header()

        self.build_pinmux = copy.deepcopy(self.current_pinmux)

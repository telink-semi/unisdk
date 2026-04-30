import logging

from scripts.pinmux.data.path_manager import PathManager
from scripts.pinmux.data.pin_data import PinData
from scripts.pinmux.data.pinmux_data import PinmuxData
from scripts.pinmux.data.pinmux_function import PinmuxFunction
from scripts.pinmux.data.pinmux_identifier import PinmuxIdentifier, PinmuxType
from scripts.pinmux.data.pinmux_name import PinmuxFunctionName, PortName, PortPinName
from scripts.pinmux.data.port_data import PortData
from scripts.readers.config_reader import ConfigReader
from scripts.readers.yaml_reader import YamlReader


from benedict.core import merge


from typing import Any, Generator


logger = logging.getLogger(__name__)


class PinmuxContext:
    """Represent the context in which the given PinmuxData can be interpreted."""

    id: PinmuxIdentifier = PinmuxIdentifier()
    ports: dict[PortName, PortData] = {}
    functions: dict[PinmuxFunctionName, PinmuxFunction] = {}

    def __init__(self, path_manager: PathManager, dotconfig: ConfigReader) -> None:
        self.path_manager = path_manager
        self.dotconfig = dotconfig

    @property
    def port_names(self) -> set[PortName]:
        return self.ports.keys()

    @property
    def function_names(self) -> set[PinmuxFunctionName]:
        return self.functions.keys()

    @property
    def default_pinmux_data(self) -> "PinmuxData":
        """Calculate the default pinmux from the function's 'default' property."""

        data = PinmuxData()
        data.id = self.id

        for function_name, function_data in self.functions.items():
            if function_data.default is not None and function_data.default.is_valid(self):
                data.pinmux[function_data.default] = function_name

        return data


    def read_ports(self) -> None:
        data_yaml_path = self.path_manager.root_dir / "soc" / self.id.soc_series / "pinmux" / "pinmux.yaml"
        data_yaml = YamlReader(data_yaml_path)
        ports = data_yaml.data.get("pinmux", {}).get("port", {})
        if not isinstance(ports, dict):
            logger.error(f"Cannot read port data in {data_yaml.path}")
            return

        data_yaml_override_path = self.path_manager.root_dir / "soc" / self.id.soc_series / "pinmux" / f"pinmux_{self.id.case.lower()}.yaml"
        data_yaml_override = YamlReader(data_yaml_override_path)
        ports_override = data_yaml_override.data.get("pinmux", {}).get("port", {})
        if isinstance(ports_override, dict) and ports_override:
            merge(ports, ports_override)

        def listify(v: Any)-> list[str]:
            if v is None:
                return []
            if isinstance(v, str):
                return [v]
            if isinstance(v, list):
                return [str(x) for x in v]
            return [str(v)]

        def get_functions_starting_with(parts: set[str]) -> Generator[PinmuxFunctionName, Any, None]:
            for part in parts:
                for function in self.function_names:
                    if function.startswith(part.upper()):
                        yield function

        for port_name, port_entry in ports.items():
            if not isinstance(port_name, str) or len(port_name) != 2 or port_name[0] != "p":
                logger.warning(f"Unknown port entry with key '{port_name}'")
                continue

            if port_entry == "delete":
                continue

            if not isinstance(port_entry, dict):
                logger.warning(f"Unknown port entry with key '{port_name}'")
                continue

            port = PortData(port_name[1].upper(), self)
            
            port.disabled = not self.dotconfig.configs.get(f"TLK_GPIO_PORT_{port.name}_ENABLED", False)

            port_ex = listify(port_entry.get("exclude"))
            # port_in = listify(port_entry.get("include")) # unsupported feature for now

            if "all" in port_ex:
                port_functions = set()
            else:
                port_functions = set(self.function_names) - set(get_functions_starting_with(port_ex))

            pins = port_entry.get("pins", {})

            if not isinstance(pins, dict):
                logger.warning(f"Cannot load pins in port '{port_name}'")
                continue

            for pin_name, pin_entry in pins.items():
                if not isinstance(pin_name, str) or len(pin_name) != 2 or pin_name[0] != "p":
                    logger.warning(f"Unknown pin entry with key '{pin_name}' in {port_name}")
                    continue

                if pin_entry == "delete":
                    continue

                pin_functions = set(port_functions)

                if pin_entry is not None:
                    if not isinstance(pin_entry, dict):
                        logger.warning(f"Unknown port entry with key '{port_name}'")
                        continue

                    pin_ex = listify(pin_entry.get("exclude"))
                    pin_in = listify(pin_entry.get("include"))

                    if "all" in pin_ex:
                        pin_functions.clear()
                    else:
                        pin_functions -= set(get_functions_starting_with(pin_ex))

                    if "all" in pin_in:
                        pin_functions = set(self.function_names)
                    else:
                        pin_functions |= set(get_functions_starting_with(pin_in))

                pin = PinData(port, pin_name[1].upper(), sorted(pin_functions))
                port.pins[pin.name] = pin

            if len(port.pins):
                self.ports[port_name[1].upper()] = port

    def read_functions(self) -> None:
        if self.id.type == PinmuxType.BOARD:
            # TODO: parse board's yaml as well
            pass

        yaml_path = self.path_manager.root_dir / "soc" / self.id.soc_series / "pinmux" / "function.yaml"
        yaml = YamlReader(yaml_path).data

        nested_functions: dict[str, dict] = yaml.get("pinmux", {}).get("function", {})

        if not nested_functions:
            logger.error(f"Nothing to load for {self.id.name}")
            return

        for group, functions in nested_functions.items():
            for subname, function_data in functions.items():
                function = PinmuxFunction()
                function.name = f"{group}_{subname}".upper()
                function.reg_value = function_data.get("reg_value")
                function.depends_on = function_data.get("depends_on", "GPIO")

                if function_data.get("default") is not None:
                    function.default = PortPinName(function_data.get("default"))

                function.optional = function_data.get("optional", False)
                function.repeatable = function_data.get("repeatable", False)

                if not self.dotconfig.configs.get(function.depends_on, None):
                    logger.info(f"Skipping {function.name} function...")
                    continue

                self.functions[function.name] = function

    def read_id(self) -> None:
        """Read the identifier from the dotconfig."""

        if self.dotconfig.configs.get("TLK_HAS_BOARD_SELECTED", False):
            self.id.board = self.dotconfig.configs.get("TLK_BOARD")

        self.id.soc_series = self.dotconfig.configs.get("TLK_SOC_SERIES", "")
        if not self.id.soc_series:
            logger.error("CONFIG_TLK_SOC_SERIES not found")
            return
        
        self.id.soc = self.dotconfig.configs.get("TLK_SOC", "")
        if not self.id.soc:
            logger.error("CONFIG_TLK_SOC not found")
            return

        self.id.case = self.dotconfig.configs.get("TLK_CASE", "")
        if not self.id.case:
            logger.error("CONFIG_TLK_CASE not found")
            return
    
    def apply_pinmux(self, pinmux: PinmuxData) -> None:
        """Syncronize the given pinmux with PinData's selected function property."""

        for port_pin, function in pinmux.pinmux.items():
            port_pin.get(self).selected_function = function
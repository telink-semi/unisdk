from collections import defaultdict
from pathlib import Path
import logging
import re

from scripts.pinmux.data.pinmux_error import ConflictedPinsError, UnassignedFunctionError
from scripts.pinmux.data.pinmux_name import PinmuxFunctionName, PortPinName
from scripts.pinmux.data.pinmux_identifier import PinmuxIdentifier, PinmuxType


from typing import TYPE_CHECKING


if TYPE_CHECKING:
    from scripts.pinmux.data.pinmux_context import PinmuxContext


logger = logging.getLogger(__name__)


class PinmuxData:
    """Represent the complete pinmux configuration unit."""

    id: PinmuxIdentifier
    pinmux: dict[PortPinName, PinmuxFunctionName]

    def __init__(self) -> None:
        self.id = PinmuxIdentifier()
        self.pinmux = {}

    def __eq__(self, other) -> bool:
        return isinstance(other, PinmuxData) and self.id == other.id and self.pinmux == other.pinmux

    @property
    def reversed(self) -> defaultdict[PinmuxFunctionName, set[PortPinName]]:
        """Calculate the reversed one-to-many relation between the functions and pins."""

        reversed: defaultdict[PinmuxFunctionName, set[PortPinName]] = defaultdict(set)

        for port_pin, function in self.pinmux.items():
            reversed[function].add(port_pin)

        return reversed

    def reverse_lookup(self, _function: PinmuxFunctionName) -> set[PortPinName]:
        """Search for pins that select the given function."""

        return set([port_pin for port_pin, function in self.pinmux.items() if function == _function])

    def is_valid(self, context: "PinmuxContext") -> bool:
        """Check if pinmux is valid in the given context."""
        try:
            return self.validate(context)
        except:
            return False

    def validate(self, context: "PinmuxContext") -> bool:
        """Return True if pinmux is valid in the given context, otherwise throw PinmuxError."""

        reversed = self.reversed

        conflicted = dict((function, port_pins) for function, port_pins in reversed.items() 
                          if not context.functions[function].repeatable and len(port_pins) > 1)
        
        if len(conflicted.items()):
            raise ConflictedPinsError(conflicted)
        
        configured = set(reversed.keys())
        required = set([f.name for f in context.functions.values() if not f.optional])

        left = required - configured

        if len(left):
            raise UnassignedFunctionError(left)
        
        return True

    def override_by(self, other: "PinmuxData", context: "PinmuxContext") -> None:
        """Merge the given pinmux with current with priority to the given one."""

        for pin_port, function in other.pinmux.items():
            if function not in context.function_names:
                logger.warning(f"Rejected {function} since it is not in the context")
            else:
                if not context.functions[function].repeatable and function in self.pinmux.values():
                    for _port_pin in self.reverse_lookup(function):
                        self.pinmux.pop(_port_pin)

                self.pinmux[pin_port] = function

    def read(self, file: str | Path, context: "PinmuxContext") -> None:
        """Read the .pinmux file with a proper identifier in the file name."""

        file_name = Path(file).name
        file_pattern = re.compile(r"^((\w+)\.(board|soc))?\.pinmux$")
        file_match = file_pattern.match(file_name)

        if not file_match:
            logger.error(f"Cannot get metadata from file name {file_name}")
            return None

        prefix, id_name, id_type = file_match.groups()

        if prefix is not None:
            self.id = PinmuxIdentifier(PinmuxType[id_type.upper()], id_name.upper())

        pattern = re.compile(r"^(P[A-Z][0-7])\s*=\s*([A-Z0-9_]+)")

        with open(file) as f:
            for lineno, line in enumerate(f, 1):
                line = line.strip()

                if not line:
                    continue

                m = pattern.match(line)

                if not m:
                    logger.warning(f"Cannot read {file}:{lineno}")
                    continue

                port_pin, function = m.groups()
                port_pin, function = PortPinName(port_pin), PinmuxFunctionName(function)

                if function not in context.function_names:
                    logger.info(f"Function '{function}' is not in context {file}:{lineno}")
                    continue

                if function in self.pinmux.values() and not context.functions[function].repeatable:
                    logger.warning(f"Cannot repeat the '{function}' function {file}:{lineno}")
                    continue

                if port_pin.port not in context.port_names:
                    logger.warning(f"Unknown port '{port_pin.port}' {file}:{lineno}")
                    continue

                if context.ports[port_pin.port].disabled:
                    logger.warning(f"Port '{port_pin.port}' is disabled {file}:{lineno}")
                    continue

                self.pinmux[port_pin] = function

    def write(self, file: str | Path) -> None:
        """Write the .pinmux file."""

        with open(file, "wt") as f:
            for port_pin, function in self.pinmux.items():
                f.write(f"{port_pin}={function}\n")

    def write_header(self, file: str | Path, context: "PinmuxContext") -> None:
        """Write the C header file for UniSDK."""

        with open(file, "wt") as f:
            def write_pair(function: PinmuxFunctionName, port_pin: PortPinName, index: int | None = None):
                maybe_index = "" if index is None else f"_{index}"
                f.write(f"#define PINMUX_{function}{maybe_index}_PORT TLK_GPIO_PORT_{port_pin.port}\n")
                f.write(f"#define PINMUX_{function}{maybe_index}_PIN TLK_GPIO_PIN_{port_pin.pin}\n")

            for function, port_pins in self.reversed.items():
                if context.functions[function].repeatable:
                    for index, port_pin in enumerate(port_pins):
                        write_pair(function, port_pin, index)
                    f.write(f"#define PINMUX_{function}_COUNT {len(port_pins)}")
                else:
                    port_pin = port_pins.pop()
                    write_pair(function, port_pin)
from typing import TYPE_CHECKING, Optional


if TYPE_CHECKING:
    from scripts.pinmux.data.pin_data import PinData
    from scripts.pinmux.data.pinmux_context import PinmuxContext


class PinmuxFunctionName(str):
    """Derive the str to type hint the pinmux function name."""
    pass


class PortName(str):
    """Derive the str to type hint the port name consisting of a single upper-case character."""
    pass


class PinName(str):
    """Derive the str to type hint the pin name consisting of a single number character."""
    pass


class PortPinName(str):
    """Derive the str to type hint the port and pin name like PB0 or PC5."""

    @property
    def port(self) -> PortName:
        return self[1]

    @property
    def pin(self) -> PinName:
        return self[2]

    def is_valid(self, context: "PinmuxContext") -> bool:
        """Check if it is a valid port and pin name."""

        if self[0] != "P":
            return False

        if self.port not in context.port_names:
            return False

        if context.ports[self.port].disabled:
            return False

        if self.pin not in context.ports[self.port].pin_names:
            return False
        
        return True
    
    def get(self, context: "PinmuxContext") -> Optional["PinData"]:
        """Find the PinData in the context by its own name."""

        port = context.ports.get(self.port)
        return port.pins.get(self.pin) if port else None
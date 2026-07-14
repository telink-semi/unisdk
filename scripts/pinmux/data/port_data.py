from typing import TYPE_CHECKING

from scripts.pinmux.data.pinmux_name import PinName, PortName

if TYPE_CHECKING:
    from scripts.pinmux.data.pin_data import PinData
    from scripts.pinmux.data.pinmux_context import PinmuxContext


class PortData:
    """Contain the metadata for the GPIO port."""

    context: "PinmuxContext"
    disabled = False
    pins: dict[PinName, "PinData"]

    def __init__(self, name: PortName, context: "PinmuxContext") -> None:
        self.name = name
        self.pins = {}
        self.context = context

    @property
    def pin_names(self) -> set[PinName]:
        return self.pins.keys()

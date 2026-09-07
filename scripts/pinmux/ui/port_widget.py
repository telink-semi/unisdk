from ..data.port_data import PortData
from .pin_widget import PinWidget

from textual.app import ComposeResult
from textual.containers import Horizontal, Container


class PortWidget(Horizontal):
    """Represents a single GPIO port."""

    def __init__(self, port: PortData) -> None:
        super().__init__()
        self.port = port
        self.border_title = f"PORT {port.name}"

    def compose(self) -> ComposeResult:
        for i in range(8):
            pin = self.port.pins.get(str(i))
            if pin is not None:
                yield PinWidget(pin)
            else:
                yield Container(classes="pin_stub")
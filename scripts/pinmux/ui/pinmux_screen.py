from .pin_widget import PinWidget
from .port_widget import PortWidget

from textual.app import ComposeResult
from textual.containers import Vertical
from textual.widgets import Footer, Header


class PinmuxScreen(Vertical):
    """Main content area with ports displayed horizontally."""

    def compose(self) -> ComposeResult:
        yield Header()
        yield Vertical(*[PortWidget(port) for _, port in self.app.manager.context.ports.items()], id="body")
        yield Footer()

from textual.screen import ModalScreen
from textual.app import ComposeResult
from textual.widgets import Button, Label
from textual.containers import Horizontal, Vertical

class QuitScreen(ModalScreen[str]):
    def compose(self) -> ComposeResult:
        yield Vertical(
            Horizontal(
                Label("Save configuration?"),
                id="text-container"
            ),
            Horizontal(
                Button("Yes", id="yes"),
                Button("No", id="no"),
                Button("Cancel", id="cancel"),
                id="buttons-container"
            ),
            id="dialog"
        )

    def on_button_pressed(self, event: Button.Pressed) -> None:
        if event.button.id == "yes":
            self.dismiss("s")
        elif event.button.id == "no":
            self.dismiss("q")
        else:
            self.dismiss(None)
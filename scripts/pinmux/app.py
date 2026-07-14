from scripts.pinmux.data.pinmux_error import ConflictedPinsError, UnassignedFunctionError
from scripts.pinmux.data.pinmux_manager import PinmuxManager

from .ui.pinmux_screen import PinmuxScreen
from .ui.quit_screen import QuitScreen

from textual.app import App, ComposeResult


class PinmuxApp(App):
    CSS_PATH = "styles.tcss"

    BINDINGS = [("q", "quit", "Quit"), ("s", "save", "Save")]
    
    def __init__(self, manager: PinmuxManager,**kwargs) -> None:
        super().__init__(**kwargs)
        self.manager = manager

    def compose(self) -> ComposeResult:
        yield PinmuxScreen()

    def action_quit(self) -> None:
        def check_quit(result: str | None):
            if result == "s":
                if self.action_save():
                    self.exit()

            elif result == "q":
                self.exit()

        if self.app.manager.is_modified or not self.manager.is_valid:
            self.push_screen(QuitScreen(), check_quit)
            return

        self.exit()

    def action_save(self) -> bool:
        try:
            self.app.manager.validate()
            self.app.manager.write()
        except ConflictedPinsError as e:
            self.notify(e.stringified_conflicts,
                        title="Resolve the conflicts before saving:",
                        severity="error")
            return False

        except UnassignedFunctionError as e:
            self.notify(e.stringified_functions, 
                        title="Assign all necessary functions before saving:",
                        severity="error")
            return False

        else:
            output_path = self.app.manager.path_manager.build_dir / f"{self.app.manager.context.id.prefix}.pinmux"
            self.notify("", title=f"Saved to {output_path}")
            return True
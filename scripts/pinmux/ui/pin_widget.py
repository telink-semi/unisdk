from ..data.pin_data import PinData


from textual.widgets import Button


class PinWidget(Button):
    """Represents a single GPIO pin."""

    def __init__(self, pin: PinData) -> None:
        super().__init__()
        self.pin = pin
        self.pin.add_observer(self.on_pin_changed)
        self.update_label()
        self.update_selection()
        self.update_has_conflict()
        self.disabled = self.pin.port.disabled

    def on_button_pressed(self, event: Button.Pressed) -> None:
        from .function_select_screen import FunctionSelectScreen
        self.app.push_screen(FunctionSelectScreen(self))

    def on_pin_changed(self, _, __, ___) -> None:
        self.update_label()
        self.update_selection()
        self.update_has_conflict()
        self.refresh()

    def on_unmount(self) -> None:
        self.pin.remove_observer(self.on_pin_changed)

    def update_label(self) -> None:
        self.label = f"{self.pin.full_name}\n{self.pin.selected_function if self.pin.selected_function is not None else 'NONE'}"

    def update_selection(self) -> None:
        if self.pin.selected_function is None:
            self.remove_class("pin_selected")
        else:
            self.add_class("pin_selected")

    def update_has_conflict(self) -> None:
        if self.pin.has_conflict:
            self.add_class("pin_conflict")
        else:
            self.remove_class("pin_conflict")

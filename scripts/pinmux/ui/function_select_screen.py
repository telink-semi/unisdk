from .pin_widget import PinWidget

from textual.app import ComposeResult
from textual.screen import ModalScreen
from textual.widgets import OptionList
from textual.widgets.option_list import Option


class FunctionSelectScreen(ModalScreen[str]):
    """Popup for selecting a pin function."""

    def __init__(self, pin_widget: PinWidget) -> None:
        super().__init__()
        self.pin_widget = pin_widget

    def compose(self) -> ComposeResult:
        sorted_functions = sorted(self.pin_widget.pin.supported_functions)
        options = [Option(f, id=f) for f in sorted_functions]
        options.insert(0, Option("NONE", id=None))

        option_list = OptionList(*options)
        option_list.border_title = f"Select function for {self.pin_widget.label}"

        if self.pin_widget.pin.selected_function in sorted_functions:
            option_list.highlighted = sorted_functions.index(self.pin_widget.pin.selected_function) + 1

        yield option_list

    def on_option_list_option_selected(self, event: OptionList.OptionSelected) -> None:
        new_function = event.option.id
        self.pin_widget.pin.select_function(new_function, self.app.manager.current_pinmux)
        self.dismiss(new_function)

    def on_mount(self) -> None:
        self.query_one(OptionList).focus()

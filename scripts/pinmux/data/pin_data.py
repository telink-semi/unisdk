from scripts.pinmux.data.observable import Observable
from scripts.pinmux.data.pinmux_name import PinName, PinmuxFunctionName, PortPinName

from typing import TYPE_CHECKING

if TYPE_CHECKING:
    from scripts.pinmux.data.pinmux_data import PinmuxData
    from scripts.pinmux.data.port_data import PortData

class PinData(Observable):
    """
    Contain metadata for a specific pin, such as supported functions,
    implementing Observable to be able to track the selected function change.
    """

    has_conflict = False

    def __init__(self, port: "PortData", name: PinName, supported_functions: set[PinmuxFunctionName]) -> None:
        super().__init__()
        self.name = name
        self.port = port
        self.supported_functions = supported_functions
        self.selected_function = None

    @property
    def full_name(self) -> PortPinName:
        return PortPinName(f"P{self.port.name}{self.name}")
    
    def select_function(self, new_function: PinmuxFunctionName | None, pinmux: "PinmuxData") -> None:
        """Select the new function for the current pin and check for conflicts with others."""

        old_function = self.selected_function
        self.selected_function = new_function

        pinmux.pinmux.pop(self.full_name, None)

        if old_function is not None and not self.port.context.functions[old_function].repeatable:
            pins_had_conflict = pinmux.reverse_lookup(old_function)

            if len(pins_had_conflict) > 0:
                if len(pins_had_conflict) == 1:
                    pins_had_conflict.pop().get(self.port.context).has_conflict = False

                self.has_conflict = False

        if new_function is not None and not self.port.context.functions[new_function].repeatable:
            pins_have_conflict = pinmux.reverse_lookup(new_function)

            if len(pins_have_conflict) > 0:
                if len(pins_have_conflict) == 1:
                    pins_have_conflict.pop().get(self.port.context).has_conflict = True

                self.has_conflict = True

        if new_function is not None:
            pinmux.pinmux[self.full_name] = new_function
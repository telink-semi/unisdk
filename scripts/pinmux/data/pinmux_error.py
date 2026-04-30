from scripts.pinmux.data.pinmux_name import PinmuxFunctionName, PortPinName


class PinmuxError(Exception):
    """Base class for all pinmux-related errors."""


class ConflictedPinsError(PinmuxError):
    def __init__(self, conflicts: dict[PinmuxFunctionName, set[PortPinName]]) -> None:
        self.conflicts = conflicts
        super().__init__(f"Conflicted pins detected: {self.stringified_conflicts}")
    
    @property
    def stringified_conflicts(self) -> str:
        return "; ".join([f"{function}: {', '.join(port_pins)}" for function, port_pins in self.conflicts.items()])


class UnassignedFunctionError(PinmuxError):
    def __init__(self, unassigned: set[PinmuxFunctionName]) -> None:
        self.unassigned = unassigned
        super().__init__(f"Unassigned mux detected: {self.stringified_functions}")
    
    @property
    def stringified_functions(self) -> str:
        return ", ".join(self.unassigned)

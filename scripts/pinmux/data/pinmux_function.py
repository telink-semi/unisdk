from scripts.pinmux.data.pinmux_name import PinmuxFunctionName, PortPinName


class PinmuxFunction:
    """Describe the pinmux function."""

    name: PinmuxFunctionName
    reg_value: int | None = None
    depends_on: str
    default: PortPinName | None = None
    optional: bool = False
    repeatable: bool = False
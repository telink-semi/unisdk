from enum import Enum, auto


class PinmuxType(Enum):
    SOC = auto()
    BOARD = auto()


class PinmuxIdentifier:
    """Identify the target of the pinmux configuration."""

    case: str = ""
    board: str | None = None
    soc: str = ""
    soc_series: str = ""

    def __init__(self, type: PinmuxType = PinmuxType.SOC, name: str = "") -> None:
        if type == PinmuxType.BOARD:
            self.board = name
        else:
            self.soc = name
        # TODO: parse the rest parameters from yaml 

    def __eq__(self, other) -> bool:
        return isinstance(other, PinmuxIdentifier) and self.type == other.type and self.name == other.name

    @property
    def has_board(self) -> bool:
        return self.board is not None

    @property
    def name(self) -> str:
        """Return the board name or SoC name if the board is not selected."""

        return self.board if self.has_board else self.soc

    @property
    def prefix(self) -> str:
        """Compose the prefix for the file name."""

        return f"{self.name}.{self.type.name.lower()}"
    
    @property
    def type(self) -> PinmuxType:
        return PinmuxType.BOARD if self.has_board else PinmuxType.SOC
class Observable:
    """Implement the Observable pattern."""

    def __init__(self):
        object.__setattr__(self, "_observers", set())

    def add_observer(self, callback) -> None:
        self._observers.add(callback)

    def remove_observer(self, callback) -> None:
        self._observers.discard(callback)

    def notify_observers(self, key, value) -> None:
        for callback in self._observers:
            callback(self, key, value)

    def __setattr__(self, key, value) -> None:
        old_value = getattr(self, key, None)
        object.__setattr__(self, key, value)
        if key not in {"_observers"} and old_value != value:
            self.notify_observers(key, value)

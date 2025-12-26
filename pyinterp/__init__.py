"""Pyinterp package."""

import copyreg
from typing import Any

from . import core


def _unpickle_grid(state: tuple[Any, ...]) -> core.GridHolder:
    """Unpickle a Grid from state tuple (axes..., array)."""
    return core.Grid(*state)


def _reduce_grid(grid: core.GridHolder) -> tuple[Any, ...]:
    """Pickle reducer for Grid objects."""
    return (_unpickle_grid, (grid.__getstate__(),))


# Register the pickle reducer for GridHolder (the actual C++ class)
copyreg.pickle(core.GridHolder, _reduce_grid)

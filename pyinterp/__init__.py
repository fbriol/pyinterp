"""Pyinterp - Interpolation and geospatial operations for Python.

This package provides efficient interpolation methods for gridded data,
geospatial operations, and statistical analysis tools.
"""

import copyreg
from typing import Any

from . import core
from .core import (
    Axis,
    Grid,
    Period,
    PeriodList,
    RTree3D,
    TemporalAxis,
    config,
    dateutils,
    geometry,
)
from .regular_grid_interpolator import bivariate, quadrivariate, trivariate


__all__ = [
    "Axis",
    "Grid",
    "Period",
    "PeriodList",
    "RTree3D",
    "TemporalAxis",
    "bivariate",
    "config",
    "dateutils",
    "geometry",
    "quadrivariate",
    "trivariate",
]


def _unpickle_grid(state: tuple[Any, ...]) -> core.GridHolder:
    """Unpickle a Grid from state tuple (axes..., array)."""
    return core.Grid(*state)


def _reduce_grid(grid: core.GridHolder) -> tuple[Any, ...]:
    """Pickle reducer for Grid objects."""
    return (_unpickle_grid, (grid.__getstate__(),))


# Register the pickle reducer for GridHolder (the actual C++ class)
copyreg.pickle(core.GridHolder, _reduce_grid)

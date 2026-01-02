"""Pyinterp - Interpolation and geospatial operations for Python.

This package provides efficient interpolation methods for gridded data,
geospatial operations, and statistical analysis tools.
"""

import copyreg
from typing import Any

from . import core, fill
from .core import (
    Axis,
    Grid,
    RTree3D,
    TemporalAxis,
    config,
    dateutils,
    geometry,
    period,
)
from .regular_grid_interpolator import bivariate, quadrivariate, trivariate
from .rtree import (
    inverse_distance_weighting,
    kriging,
    query,
    radial_basis_function,
    window_function,
)


__all__ = [
    "Axis",
    "Grid",
    "RTree3D",
    "TemporalAxis",
    "bivariate",
    "config",
    "dateutils",
    "fill",
    "geometry",
    "inverse_distance_weighting",
    "kriging",
    "period",
    "quadrivariate",
    "query",
    "radial_basis_function",
    "trivariate",
    "window_function",
]


def _unpickle_grid(state: tuple[Any, ...]) -> core.GridHolder:
    """Unpickle a Grid from state tuple (axes..., array)."""
    return core.Grid(*state)


def _reduce_grid(grid: core.GridHolder) -> tuple[Any, ...]:
    """Pickle reducer for Grid objects."""
    return (_unpickle_grid, (grid.__getstate__(),))


# Register the pickle reducer for GridHolder (the actual C++ class)
copyreg.pickle(core.GridHolder, _reduce_grid)

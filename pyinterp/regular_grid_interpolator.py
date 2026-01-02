"""Regular Grid Interpolator.

This module provides convenient wrapper functions around pyinterp.core
interpolation functions that accept both string-based method names and
configuration objects.

For most use cases, you can simply pass a string method name:
>>> result = bivariate(grid, x, y, "bilinear")

For advanced configuration, pass a config object:
>>> from pyinterp.config import windowed
>>> config = windowed.Bivariate.bicubic().with_window_size_x(10)
>>> result = bivariate(grid, x, y, config)
"""

from __future__ import annotations

from typing import TYPE_CHECKING, Literal, TypeVar, cast, get_args, overload

from . import core
from .core.config import geometric, windowed

if TYPE_CHECKING:
    from .core import GridHolder
    from .type_hints import (
        NDArray1DFloat32,
        NDArray1DFloat64,
        NDArray1DDateTime64,
    )

__all__ = ["bivariate", "quadrivariate", "trivariate"]

#: Geometric interpolation methods (simple, non-windowed)
GeometricMethods = Literal["bilinear", "idw", "nearest"]

#: Windowed interpolation methods (with window functions)
WindowedMethods = Literal[
    "akima",
    "akima_periodic",
    "bicubic",
    "bilinear",
    "c_spline",
    "c_spline_not_a_knot",
    "c_spline_periodic",
    "linear",
    "polynomial",
    "steffen",
]

#: All valid interpolation method names
InterpolationMethods = GeometricMethods | WindowedMethods

#: Boundary mode strings
BoundaryMode = Literal["expand", "symmetric", "undefined", "wrap"]

#: Axis configuration strings
AxisConfigStr = Literal["linear", "nearest"]

# Extract valid method names for runtime validation
_GEOMETRIC_METHODS = get_args(GeometricMethods)
_WINDOWED_METHODS = get_args(WindowedMethods)

# Boundary mode string to enum mapping
_BOUNDARY_MAP = {
    "expand": windowed.Boundary.EXPAND,
    "symmetric": windowed.Boundary.SYM,
    "undefined": windowed.Boundary.UNDEF,
    "wrap": windowed.Boundary.WRAP,
}

# TypeVars for generic config creation
_GeometricConfig = TypeVar(
    "_GeometricConfig",
    geometric.Bivariate,
    geometric.Trivariate,
    geometric.Quadrivariate,
)

_WindowedConfig = TypeVar(
    "_WindowedConfig",
    windowed.Bivariate,
    windowed.Trivariate,
    windowed.Quadrivariate,
)


def _make_geometric_config(
    method: GeometricMethods,
    class_type: type[_GeometricConfig],
    bounds_error: bool = False,
    num_threads: int = 0,
) -> _GeometricConfig:
    """Create a geometric interpolation configuration."""
    if method not in _GEOMETRIC_METHODS:
        raise ValueError(
            f"Unknown geometric method: '{method}'. "
            f"Valid methods: {', '.join(_GEOMETRIC_METHODS)}"
        )

    factory = getattr(class_type, method)
    config = factory()

    return cast(
        "_GeometricConfig",
        config.with_bounds_error(bounds_error).with_num_threads(num_threads),
    )


def _make_windowed_config(
    method: WindowedMethods,
    class_type: type[_WindowedConfig],
    bounds_error: bool = False,
    num_threads: int = 0,
    window_size_x: int | None = None,
    window_size_y: int | None = None,
    boundary_mode: BoundaryMode | None = None,
    third_axis: AxisConfigStr | None = None,
    fourth_axis: AxisConfigStr | None = None,
) -> _WindowedConfig:
    """Create a windowed interpolation configuration."""
    if method not in _WINDOWED_METHODS:
        raise ValueError(
            f"Unknown windowed method: '{method}'. "
            f"Valid methods: {', '.join(_WINDOWED_METHODS)}"
        )

    factory = getattr(class_type, method)
    config = factory()

    config = config.with_bounds_error(bounds_error).with_num_threads(
        num_threads
    )

    if window_size_x is not None:
        config = config.with_window_size_x(window_size_x)
    if window_size_y is not None:
        config = config.with_window_size_y(window_size_y)
    if boundary_mode is not None:
        config = config.with_boundary_mode(_BOUNDARY_MAP[boundary_mode])

    def _axis_config(axis: AxisConfigStr) -> windowed.AxisConfig:
        return (
            windowed.AxisConfig.linear()
            if axis == "linear"
            else windowed.AxisConfig.nearest()
        )

    # Apply axis configurations for 3D/4D
    if third_axis is not None:
        config = config.with_third_axis(_axis_config(third_axis))

    if fourth_axis is not None:
        config = config.with_fourth_axis(_axis_config(fourth_axis))

    return cast("_WindowedConfig", config)


def _validate_no_windowed_options(
    method: str,
    window_size_x: int | None,
    window_size_y: int | None,
    boundary_mode: BoundaryMode | None,
    third_axis: AxisConfigStr | None = None,
    fourth_axis: AxisConfigStr | None = None,
) -> None:
    """Validate that windowed options aren't used with geometric methods."""
    invalid_opts = []
    if window_size_x is not None:
        invalid_opts.append("window_size_x")
    if window_size_y is not None:
        invalid_opts.append("window_size_y")
    if boundary_mode is not None:
        invalid_opts.append("boundary_mode")
    if third_axis is not None:
        invalid_opts.append("third_axis")
    if fourth_axis is not None:
        invalid_opts.append("fourth_axis")

    if invalid_opts:
        raise TypeError(
            f"Options {invalid_opts} are not valid for geometric method "
            f"'{method}'. These options are only available for windowed "
            f"methods: {', '.join(_WINDOWED_METHODS)}"
        )


@overload
def bivariate(
    grid: GridHolder,
    x: NDArray1DFloat64,
    y: NDArray1DFloat64,
    method: geometric.Bivariate | windowed.Bivariate,
) -> NDArray1DFloat32 | NDArray1DFloat64: ...


@overload
def bivariate(
    grid: GridHolder,
    x: NDArray1DFloat64,
    y: NDArray1DFloat64,
    method: GeometricMethods = "bilinear",
    *,
    bounds_error: bool = False,
    num_threads: int = 0,
) -> NDArray1DFloat32 | NDArray1DFloat64: ...


@overload
def bivariate(
    grid: GridHolder,
    x: NDArray1DFloat64,
    y: NDArray1DFloat64,
    method: WindowedMethods,
    *,
    bounds_error: bool = False,
    num_threads: int = 0,
    window_size_x: int | None = None,
    window_size_y: int | None = None,
    boundary_mode: BoundaryMode | None = None,
) -> NDArray1DFloat32 | NDArray1DFloat64: ...


def bivariate(
    grid: GridHolder,
    x: NDArray1DFloat64,
    y: NDArray1DFloat64,
    method: geometric.Bivariate
    | windowed.Bivariate
    | InterpolationMethods = "bilinear",
    *,
    bounds_error: bool = False,
    num_threads: int = 0,
    window_size_x: int | None = None,
    window_size_y: int | None = None,
    boundary_mode: BoundaryMode | None = None,
) -> NDArray1DFloat32 | NDArray1DFloat64:
    """Bivariate interpolation.

    Args:
        grid : The 2D grid to interpolate from
        x: X coordinates at which to interpolate
        y: Y coordinates at which to interpolate
        method : Interpolation method (config object or string)
        bounds_error : If True, raise error for out-of-bounds coordinates
        num_threads : Number of threads to use (0 = auto)
        window_size_x : Window size for X axis (windowed methods only)
        window_size_y : Window size for Y axis (windowed methods only)
        boundary_mode : Boundary handling mode (windowed methods only)

    Return:
        Interpolated values

    Examples:
        Simple usage:

        >>> result = bivariate(grid, x, y, "bilinear")
        >>> result = bivariate(grid, x, y, "bicubic", window_size_x=10)

        Advanced usage with config objects:

        >>> from pyinterp.core.config import windowed
        >>> config = windowed.Bivariate.bicubic().with_window_size_x(10)
        >>> result = bivariate(grid, x, y, config)

    """
    config: geometric.Bivariate | windowed.Bivariate

    # If method is a config object, use it directly
    if not isinstance(method, str):
        return core.bivariate(grid, x, y, method)

    # String-based method - create config
    if method in _GEOMETRIC_METHODS:
        # Validate no windowed options for geometric methods
        _validate_no_windowed_options(
            method,
            window_size_x,
            window_size_y,
            boundary_mode,
        )
        config = _make_geometric_config(
            method,  # type: ignore[arg-type]
            geometric.Bivariate,
            bounds_error,
            num_threads,
        )
    else:
        # Windowed method
        config = _make_windowed_config(
            method,  # type: ignore[arg-type]
            windowed.Bivariate,
            bounds_error,
            num_threads,
            window_size_x,
            window_size_y,
            boundary_mode,
        )

    return core.bivariate(grid, x, y, config)


@overload
def trivariate(
    grid: GridHolder,
    x: NDArray1DFloat64,
    y: NDArray1DFloat64,
    z: NDArray1DFloat64 | NDArray1DDateTime64,
    method: geometric.Trivariate | windowed.Trivariate,
) -> NDArray1DFloat32 | NDArray1DFloat64: ...


@overload
def trivariate(
    grid: GridHolder,
    x: NDArray1DFloat64,
    y: NDArray1DFloat64,
    z: NDArray1DFloat64 | NDArray1DDateTime64,
    method: GeometricMethods = "bilinear",
    *,
    bounds_error: bool = False,
    num_threads: int = 0,
) -> NDArray1DFloat32 | NDArray1DFloat64: ...


@overload
def trivariate(
    grid: GridHolder,
    x: NDArray1DFloat64,
    y: NDArray1DFloat64,
    z: NDArray1DFloat64 | NDArray1DDateTime64,
    method: WindowedMethods,
    *,
    bounds_error: bool = False,
    num_threads: int = 0,
    window_size_x: int | None = None,
    window_size_y: int | None = None,
    boundary_mode: BoundaryMode | None = None,
    third_axis: AxisConfigStr | None = None,
) -> NDArray1DFloat32 | NDArray1DFloat64: ...


def trivariate(
    grid: GridHolder,
    x: NDArray1DFloat64,
    y: NDArray1DFloat64,
    z: NDArray1DFloat64 | NDArray1DDateTime64,
    method: geometric.Trivariate
    | windowed.Trivariate
    | InterpolationMethods = "bilinear",
    *,
    bounds_error: bool = False,
    num_threads: int = 0,
    window_size_x: int | None = None,
    window_size_y: int | None = None,
    boundary_mode: BoundaryMode | None = None,
    third_axis: AxisConfigStr | None = None,
) -> NDArray1DFloat32 | NDArray1DFloat64:
    """Trivariate interpolation.

    Args:
    grid : The 3D grid to interpolate from
    x: X coordinates at which to interpolate
    y: Y coordinates at which to interpolate
    z: Z coordinates at which to interpolate
    method : Interpolation method (config object or string)
    bounds_error : If True, raise error for out-of-bounds coordinates
    num_threads : Number of threads to use (0 = auto)
    window_size_x : Window size for X axis (windowed methods only)
    window_size_y : Window size for Y axis (windowed methods only)
    boundary_mode : Boundary handling mode (windowed methods only)
    third_axis : Interpolation method for Z axis (windowed methods only)

    Return:
        Interpolated values

    Raise:
        TypeError: If windowed options are provided with geometric methods

    Examples:
        >>> result = trivariate(grid, x, y, z, "bilinear")
        >>> result = trivariate(grid, x, y, z, "bicubic", third_axis="linear")

    """
    config: geometric.Trivariate | windowed.Trivariate

    # If method is a config object, use it directly
    if not isinstance(method, str):
        return core.trivariate(grid, x, y, z, method)

    # String-based method - create config
    if method in _GEOMETRIC_METHODS:
        # Validate no windowed options for geometric methods
        _validate_no_windowed_options(
            method,
            window_size_x,
            window_size_y,
            boundary_mode,
            third_axis,
        )
        config = _make_geometric_config(
            method,  # type: ignore[arg-type]
            geometric.Trivariate,
            bounds_error,
            num_threads,
        )
    else:
        # Windowed method
        config = _make_windowed_config(
            method,  # type: ignore[arg-type]
            windowed.Trivariate,
            bounds_error,
            num_threads,
            window_size_x,
            window_size_y,
            boundary_mode,
            third_axis,
        )

    return core.trivariate(grid, x, y, z, config)


@overload
def quadrivariate(
    grid: GridHolder,
    x: NDArray1DFloat64,
    y: NDArray1DFloat64,
    z: NDArray1DFloat64 | NDArray1DDateTime64,
    u: NDArray1DFloat64,
    method: geometric.Quadrivariate | windowed.Quadrivariate,
) -> NDArray1DFloat32 | NDArray1DFloat64: ...


@overload
def quadrivariate(
    grid: GridHolder,
    x: NDArray1DFloat64,
    y: NDArray1DFloat64,
    z: NDArray1DFloat64 | NDArray1DDateTime64,
    u: NDArray1DFloat64,
    method: GeometricMethods = "bilinear",
    *,
    bounds_error: bool = False,
    num_threads: int = 0,
) -> NDArray1DFloat32 | NDArray1DFloat64: ...


@overload
def quadrivariate(
    grid: GridHolder,
    x: NDArray1DFloat64,
    y: NDArray1DFloat64,
    z: NDArray1DFloat64 | NDArray1DDateTime64,
    u: NDArray1DFloat64,
    method: WindowedMethods,
    *,
    bounds_error: bool = False,
    num_threads: int = 0,
    window_size_x: int | None = None,
    window_size_y: int | None = None,
    boundary_mode: BoundaryMode | None = None,
    third_axis: AxisConfigStr | None = None,
    fourth_axis: AxisConfigStr | None = None,
) -> NDArray1DFloat32 | NDArray1DFloat64: ...


def quadrivariate(
    grid: GridHolder,
    x: NDArray1DFloat64,
    y: NDArray1DFloat64,
    z: NDArray1DFloat64 | NDArray1DDateTime64,
    u: NDArray1DFloat64,
    method: geometric.Quadrivariate
    | windowed.Quadrivariate
    | InterpolationMethods = "bilinear",
    *,
    bounds_error: bool = False,
    num_threads: int = 0,
    window_size_x: int | None = None,
    window_size_y: int | None = None,
    boundary_mode: BoundaryMode | None = None,
    third_axis: AxisConfigStr | None = None,
    fourth_axis: AxisConfigStr | None = None,
) -> NDArray1DFloat32 | NDArray1DFloat64:
    """Quadrivariate interpolation.

    Args:
    grid : The 4D grid to interpolate from
    x: X coordinates at which to interpolate
    y: Y coordinates at which to interpolate
    z: Z coordinates at which to interpolate
    u: U coordinates at which to interpolate
    method: Interpolation method (config object or string)
    bounds_error : If True, raise error for out-of-bounds coordinates
    num_threads : Number of threads to use (0 = auto)
    window_size_x : Window size for X axis (windowed methods only)
    window_size_y : Window size for Y axis (windowed methods only)
    boundary_mode : Boundary handling mode (windowed methods only)
    third_axis : Interpolation method for Z axis (windowed methods only)
    fourth_axis : Interpolation method for U axis (windowed methods only)

    Return:
        Interpolated values

    Raise:
        TypeError: If windowed options are provided with geometric methods

    Examples:
        >>> result = quadrivariate(grid, x, y, z, u, "bilinear")
        >>> result = quadrivariate(
        ...     grid,
        ...     x,
        ...     y,
        ...     z,
        ...     u,
        ...     "bicubic",
        ...     third_axis="linear",
        ...     fourth_axis="linear",
        ... )

    """
    config: geometric.Quadrivariate | windowed.Quadrivariate

    # If method is a config object, use it directly
    if not isinstance(method, str):
        return core.quadrivariate(grid, x, y, z, u, method)

    # String-based method - create config
    if method in _GEOMETRIC_METHODS:
        # Validate no windowed options for geometric methods
        _validate_no_windowed_options(
            method,
            window_size_x,
            window_size_y,
            boundary_mode,
            third_axis,
            fourth_axis,
        )
        config = _make_geometric_config(
            method,  # type: ignore[arg-type]
            geometric.Quadrivariate,
            bounds_error,
            num_threads,
        )
    else:
        # Windowed method
        config = _make_windowed_config(
            method,  # type: ignore[arg-type]
            windowed.Quadrivariate,
            bounds_error,
            num_threads,
            window_size_x,
            window_size_y,
            boundary_mode,
            third_axis,
            fourth_axis,
        )

    return core.quadrivariate(grid, x, y, z, u, config)

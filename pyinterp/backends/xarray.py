# Copyright (c) 2025 CNES
#
# All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE file.
"""XArray backend.

Build interpolation objects from xarray.DataArray instances
"""

from __future__ import annotations

from typing import TYPE_CHECKING, Any, cast

from .. import cf, core
from ..regular_grid_interpolator import (
    InterpolationMethods,
    bivariate,
    quadrivariate,
    trivariate,
)


if TYPE_CHECKING:
    from collections.abc import Callable, Hashable

    import numpy as np
    import xarray as xr

    from ..type_hints import NDArray1D, NDArray1DDateTime64, NDArray1DFloat64

__all__ = ["Grid2D", "Grid3D", "Grid4D"]

#: Two dimensional grid.
TWO_DIMENSIONS = 2

#: Three dimensional grid.
THREE_DIMENSIONS = 3

#: Four dimensional grid.
FOUR_DIMENSIONS = 4


class AxisIdentifier:
    """Identify axes defining longitudes and latitudes in a CF file.

    This class determines which dimensions in a data array correspond to
    longitude and latitude coordinates based on CF conventions.

    Args:
        data_array: The data array to be identified.

    """

    def __init__(self, data_array: xr.DataArray) -> None:
        """Initialize the AxisIdentifier with the provided data array."""
        self.data_array = data_array

    def _axis(self, units: cf.AxisUnit) -> str | None:
        """Return the name of the dimension that defines an axis.

        Args:
            units: The units of the axis

        Returns:
            The name of the coordinate

        """
        for name, coord in self.data_array.coords.items():
            if hasattr(coord, "units") and coord.units in units:
                return str(name)
        return None

    def longitude(self) -> str | None:
        """Return the name of the dimension that defines a longitude axis.

        Returns:
            The name of the longitude coordinate

        """
        return self._axis(cf.AxisLongitudeUnit())

    def latitude(self) -> str | None:
        """Return the name of the dimension that defines a latitude axis.

        Returns:
            The name of the latitude coordinates

        """
        return self._axis(cf.AxisLatitudeUnit())


def _dims_from_data_array(
    data_array: xr.DataArray,
    ndims: int = 2,
) -> tuple[tuple[Hashable, ...], bool]:
    """Get the name of dimensions that define the grid axes."""
    size = len(data_array.shape)
    if size != ndims:
        raise ValueError(
            "The number of dimensions of the variable is incorrect. Expected "
            f"{ndims}, found {size}."
        )

    # Get all coordinate names
    coord_names = tuple(data_array.coords.keys())

    # Always try to identify lon/lat for first two dimensions
    ident = AxisIdentifier(data_array)
    lon = ident.longitude()
    lat = ident.latitude()

    if lon is not None and lat is not None:
        # Geodetic: start with lon/lat, append remaining dimensions
        remaining = [c for c in coord_names if c not in (lon, lat)]
        return (lon, lat, *tuple(remaining)), True
    return coord_names, False


def _coords(
    coords: dict[Hashable, NDArray1D],
    dims: tuple[Hashable, ...],
    datetime64: tuple[Hashable, core.TemporalAxis] | None = None,
) -> tuple[NDArray1D | NDArray1DDateTime64, ...]:
    """Get the list of arguments to provide to grid interpolation functions."""
    if not isinstance(coords, dict):
        raise TypeError("coords must be an instance of dict")
    if len(coords) != len(dims):
        raise IndexError(
            f"Number of coordinates ({len(coords)}) doesn't match "
            f"number of dimensions ({len(dims)})"
        )
    unknown = set(coords) - set(dims)
    if unknown:
        raise IndexError(
            "axes not handled by this grid: "
            + ", ".join([str(item) for item in unknown])
        )

    # Is it necessary to manage a time axis?
    if datetime64 is not None:
        temporal_dim, temporal_axis = datetime64
        result: list[NDArray1D | NDArray1DDateTime64] = []
        for dim in dims:
            coord_value = coords[dim]
            if dim != temporal_dim:
                # Regular coordinate
                result.append(cast("NDArray1D", coord_value))
            else:
                # Cast temporal coordinates
                result.append(
                    temporal_axis.cast_to_temporal_axis(
                        cast("NDArray1DDateTime64", coord_value)
                    )
                )
        return tuple(result)

    # No temporal axis - cast all to NDArray1D
    return tuple(cast("NDArray1D", coords[dim]) for dim in dims)


class Grid2D(core.Grid2D):
    """Build a Grid2D from Xarray data.

    Create a 2D grid interpolation object from the provided Xarray data array,
    with optional axis ordering and geodetic coordinate support.

    Args:
        data_array: Provided data

    Raises:
        ValueError: if the number of dimensions is different of 2.

    """

    def __init__(self, data_array: xr.DataArray) -> None:
        """Initialize the 2D grid from an Xarray data array."""
        self._dims, geodetic = _dims_from_data_array(data_array, ndims=2)
        self._datetime64: tuple[Hashable, core.TemporalAxis] | None = None
        self._instance = core.Grid(
            core.Axis(
                data_array.coords[self._dims[0]].values,
                period=360.0 if geodetic else None,
            ),
            core.Axis(data_array.coords[self._dims[1]].values),
            data_array.transpose(*self._dims).values,
        )

    def bivariate(
        self,
        coords: dict[Hashable, NDArray1DFloat64],
        method: InterpolationMethods = "bilinear",
        **kwargs: Any,  # noqa: ANN401
    ) -> np.ndarray:
        """Evaluate the interpolation defined for the given coordinates.

        Args:
            coords: Mapping from dimension names to the
                coordinates to interpolate. Coordinates must be array-like.
            method: Interpolation method. See
                :py:func:`pyinterp.regular_grid_interpolator.bivariate`
                for more details.
            **kwargs: Additional keyword arguments provided to the
                interpolation method.

        Returns:
            The interpolated values.

        Raises:
            IndexError: If coordinate dimensions don't match grid dimensions

        """
        x, y = _coords(coords, self._dims, self._datetime64)
        return bivariate(
            self._instance,
            cast("NDArray1DFloat64", x),
            cast("NDArray1DFloat64", y),
            method=method,
            **kwargs,
        )


class Grid3D(core.Grid3D):
    """Build a Grid3D from Xarray data.

    Create a 3D grid interpolation object from the provided Xarray data array.
    Supports temporal axes via datetime64 coordinates.

    Args:
        data_array: Provided 3D data array

    Raises:
        ValueError: if the number of dimensions is different from 3.

    """

    def __init__(self, data_array: xr.DataArray) -> None:
        """Initialize the 3D grid from an Xarray data array."""
        self._dims, geodetic = _dims_from_data_array(data_array, ndims=3)
        self._datetime64: tuple[Hashable, core.TemporalAxis] | None = None

        # Check if third axis is temporal
        third_coord = data_array.coords[self._dims[2]]
        is_temporal = hasattr(third_coord, "dtype") and "datetime64" in str(
            third_coord.dtype
        )
        z_axis: core.Axis | core.TemporalAxis

        if is_temporal:
            # Create temporal grid
            z_axis = core.TemporalAxis(third_coord.values)
            self._datetime64 = (self._dims[2], z_axis)
        else:
            z_axis = core.Axis(third_coord.values)

        self._instance = core.Grid(
            core.Axis(
                data_array.coords[self._dims[0]].values,
                period=360.0 if geodetic else None,
            ),
            core.Axis(data_array.coords[self._dims[1]].values),
            z_axis,
            data_array.transpose(*self._dims).values,
        )

    def trivariate(
        self,
        coords: dict[Hashable, NDArray1DFloat64 | NDArray1DDateTime64],
        method: InterpolationMethods = "bilinear",
        **kwargs: Any,  # noqa: ANN401
    ) -> np.ndarray:
        """Evaluate the interpolation defined for the given coordinates.

        Args:
            coords: Mapping from dimension names to the
                coordinates to interpolate. Coordinates must be array-like.
                If the third axis is temporal, provide datetime64 array.
            method: Interpolation method. See
                :py:func:`pyinterp.regular_grid_interpolator.trivariate`
                for more details.
            **kwargs: Additional keyword arguments provided to the
                interpolation method.

        Returns:
            The interpolated values.

        Raises:
            IndexError: If coordinate dimensions don't match grid dimensions

        """
        x, y, z = _coords(coords, self._dims, self._datetime64)
        return trivariate(
            self._instance,
            cast("NDArray1DFloat64", x),
            cast("NDArray1DFloat64", y),
            cast("NDArray1DFloat64 | NDArray1DDateTime64", z),
            method=method,
            **kwargs,
        )


class Grid4D(core.Grid4D):
    """Build a Grid4D from Xarray data.

    Create a 4D grid interpolation object from the provided Xarray data array.
    Supports temporal axes via datetime64 coordinates.

    Args:
        data_array: Provided 4D data array

    Raises:
        ValueError: if the number of dimensions is different from 4.

    """

    def __init__(self, data_array: xr.DataArray) -> None:
        """Initialize the 4D grid from an Xarray data array."""
        self._dims, geodetic = _dims_from_data_array(data_array, ndims=4)
        self._datetime64: tuple[Hashable, core.TemporalAxis] | None = None

        # Check if third axis is temporal
        third_coord = data_array.coords[self._dims[2]]
        is_temporal = hasattr(third_coord, "dtype") and "datetime64" in str(
            third_coord.dtype
        )
        z_axis: core.Axis | core.TemporalAxis

        if is_temporal:
            # Create temporal 4D grid
            z_axis = core.TemporalAxis(third_coord.values)
            self._datetime64 = (self._dims[2], z_axis)
        else:
            z_axis = core.Axis(third_coord.values)

        self._instance = core.Grid(
            core.Axis(
                data_array.coords[self._dims[0]].values,
                period=360.0 if geodetic else None,
            ),
            core.Axis(data_array.coords[self._dims[1]].values),
            z_axis,
            core.Axis(data_array.coords[self._dims[3]].values),
            data_array.transpose(*self._dims).values,
        )

    def quadrivariate(
        self,
        coords: dict[Hashable, NDArray1DFloat64 | NDArray1DDateTime64],
        method: InterpolationMethods = "bilinear",
        **kwargs: Any,  # noqa: ANN401
    ) -> np.ndarray:
        """Evaluate the interpolation defined for the given coordinates.

        Args:
            coords: Mapping from dimension names to the
                coordinates to interpolate. Coordinates must be array-like.
                If the third axis is temporal, provide datetime64 array.
            method: Interpolation method. See
                :py:func:`pyinterp.regular_grid_interpolator.quadrivariate`
                for more details.
            **kwargs: Additional keyword arguments provided to the
                interpolation method.

        Returns:
            The interpolated values.

        Raises:
            IndexError: If coordinate dimensions don't match grid dimensions

        """
        x, y, z, u = _coords(coords, self._dims, self._datetime64)
        return quadrivariate(
            self._instance,
            cast("NDArray1DFloat64", x),
            cast("NDArray1DFloat64", y),
            cast("NDArray1DFloat64 | NDArray1DDateTime64", z),
            cast("NDArray1DFloat64", u),
            method=method,
            **kwargs,
        )


class RegularGridInterpolator:
    """Interpolate on a regular grid in arbitrary dimensions.

    Perform interpolation on a regular grid with uneven spacing support.
    Automatically detects geodetic coordinates (lon/lat) using CF conventions
    and temporal axes (datetime64).

    The data must be defined on a regular grid; the grid spacing however may be
    uneven. Linear, nearest neighbors, inverse distance weighting, and bicubic
    interpolation are supported.

    Args:
        array: The xarray DataArray defining the regular grid in ``n``
            dimensions. Must be 2D, 3D, or 4D.

    Raises:
        NotImplementedError: if the number of dimensions in the array is
            less than 2 or more than 4.

    Notes:
        **Automatic Detection:**

        The interpolator automatically detects:

        - **Geodetic coordinates**: If lon/lat are found via CF conventions
          (units attribute).
        - **Temporal axes**: If a coordinate has dtype='datetime64', it will
          be treated as a temporal axis with proper interpolation
        - **Dimension count**: Automatically selects Grid2D, Grid3D, or Grid4D

        **Geodetic Detection (CF Conventions):**

        Longitude axes are detected if the coordinate has units attribute
        matching: ``degrees_east``, ``degree_east``, ``degree_E``,
        ``degrees_E``, ``degreeE``, or ``degreesE``

        Latitude axes are detected if the coordinate has units attribute
        matching: ``degrees_north``, ``degree_north``, ``degree_N``,
        ``degrees_N``, ``degreeN``, or ``degreesN``

        **Temporal Detection:**

        Any coordinate with dtype containing 'datetime64' is automatically
        treated as a temporal axis.

    Examples:
        >>> # 2D sea surface temperature
        >>> sst = xr.open_dataarray("sst.nc")  # (lon, lat)
        >>> interp = RegularGridInterpolator(sst)
        >>> result = interp(
        ...     {"lon": [10.5, 20.3], "lat": [45.2, -30.1]}, method="bilinear"
        ... )

        >>> # 3D ocean temperature with depth
        >>> temp = xr.open_dataarray("temp.nc")  # (lon, lat, depth)
        >>> interp = RegularGridInterpolator(temp)
        >>> result = interp(
        ...     {"lon": [10.5], "lat": [45.2], "depth": [25.0]},
        ...     method="bilinear",
        ... )

        >>> # 3D SST time series (automatic temporal handling)
        >>> sst_time = xr.open_dataarray("sst_time.nc")  # (lon, lat, time)
        >>> interp = RegularGridInterpolator(sst_time)
        >>> result = interp(
        ...     {
        ...         "lon": [10.5],
        ...         "lat": [45.2],
        ...         "time": np.array(["2020-01-01"], dtype="datetime64"),
        ...     },
        ...     method="bilinear",
        ... )

    """

    def __init__(self, array: xr.DataArray) -> None:
        """Initialize the interpolator from an Xarray data array.

        Args:
            array: The xarray DataArray to interpolate. Must be 2D, 3D, or 4D.

        Raises:
            NotImplementedError: If array is not 2D, 3D, or 4D.

        """
        ndim = len(array.shape)

        self._grid: Grid2D | Grid3D | Grid4D
        self._interp: Callable[..., Any]

        if ndim == TWO_DIMENSIONS:
            self._grid = Grid2D(array)
            self._interp = self._grid.bivariate
        elif ndim == THREE_DIMENSIONS:
            self._grid = Grid3D(array)
            self._interp = self._grid.trivariate
        elif ndim == FOUR_DIMENSIONS:
            self._grid = Grid4D(array)
            self._interp = self._grid.quadrivariate
        else:
            raise NotImplementedError(
                f"Only 2D, 3D, and 4D grids can be interpolated. "
                f"Got {ndim}D grid."
            )

    @property
    def ndim(self) -> int:
        """Get the number of array dimensions.

        Returns:
            Number of array dimensions (2, 3, or 4).

        """
        return len(self._grid._dims)

    @property
    def grid(self) -> Grid2D | Grid3D | Grid4D:
        """Get the instance handling the regular grid for interpolations.

        Returns:
            The underlying Grid2D, Grid3D, or Grid4D instance.

        """
        return self._grid

    def __call__(
        self,
        coords: dict,
        method: InterpolationMethods = "bilinear",
        **kwargs: Any,  # noqa: ANN401
    ) -> np.ndarray:
        """Interpolate at coordinates.

        Perform interpolation at the specified coordinates using the chosen
        method and parameters.

        Args:
            coords: Mapping from dimension names to the new coordinates.
                Coordinates can be scalars or array-like. For temporal axes,
                provide datetime64 arrays.
            method: The method of interpolation to perform. Supported methods
                depend on the grid type. Common methods include:

                - Geometric methods: ``nearest``, ``bilinear``, `ìdw``
                - Windowed methods: ``akima``, ``akima_periodic``, ``bicubic``,
                    ``bilinear``, ``c_spline``, ``c_spline_not_a_knot``,
                    ``c_spline_periodic``, ``linear``, ``polynomial``,
                    ``steffen``.

            **kwargs: Additional keyword arguments passed to the interpolation
                function. Common options include:

                - ``bounds_error`` (bool): Raise error if coordinates are
                  out of bounds. Default: False (returns NaN).
                - ``num_threads`` (int): Number of threads for parallel
                  computation. 0 uses all CPUs. Default: 0.

                For windowed methods (bicubic, c_spline, etc.), additional
                options include:

                - ``window_size_x`` (int): Window size in X direction
                - ``window_size_y`` (int): Window size in Y direction
                - ``boundary_mode`` (str): Boundary handling mode
                  (``"expand"``, ``"wrap"``, ``"symmetric"``, ``"undefined"``)

                For 3D/4D grids:

                - ``third_axis`` (str): Method for 3rd axis
                  (``"linear"``, ``"nearest"``)
                - ``fourth_axis`` (str): Method for 4th axis
                  (``"linear"``, ``"nearest"``)

        Returns:
            Interpolated values as numpy array with same shape as input
            coordinate arrays.

        Raises:
            ValueError: If bounds_error=True and coordinates are out of bounds.
            IndexError: If coordinate dimensions don't match grid dimensions.

        Examples:
            >>> # Simple bilinear interpolation
            >>> result = interp(
            ...     {"lon": [10.5], "lat": [45.2]}, method="bilinear"
            ... )

            >>> # Bicubic with custom window size
            >>> result = interp(
            ...     {"lon": [10.5], "lat": [45.2]},
            ...     method="bicubic",
            ...     window_size_x=10,
            ...     window_size_y=10,
            ... )

            >>> # With bounds checking
            >>> result = interp(
            ...     {"lon": [10.5], "lat": [45.2]},
            ...     method="bilinear",
            ...     bounds_error=True,
            ... )

            >>> # Multi-threaded
            >>> result = interp(
            ...     {"lon": lon_array, "lat": lat_array},
            ...     method="bilinear",
            ...     num_threads=4,
            ... )

            >>> # 3D with temporal axis
            >>> result = interp(
            ...     {
            ...         "lon": [10.5],
            ...         "lat": [45.2],
            ...         "time": np.array(["2020-01-01"], dtype="datetime64"),
            ...     },
            ...     method="bilinear",
            ... )

        """
        return self._interp(coords, method=method, **kwargs)

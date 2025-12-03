# Copyright (c) 2025 CNES
#
# All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE file.
"""Unit tests for Grid."""

import numpy as np
import pytest

from pyinterp import core

from .. import load_grid2d, load_grid3d, load_grid4d

DType = type[np.int8] | type[np.float32] | type[np.float64]


def _get_suffix(dtype: DType) -> str:
    suffix_map: dict[DType, str] = {
        np.int8: 'Int8',
        np.float32: 'Float32',
        np.float64: 'Float64'
    }
    return suffix_map.get(dtype, f"Unsupported dtype: {dtype}")


@pytest.mark.parametrize(
    'dtype',
    [
        np.int8,
        np.float32,
        np.float64,
    ],
)
def test_grid2d_load(dtype: DType) -> None:
    """Test loading a 2D grid."""
    grid = load_grid2d()
    x_axis = core.Axis(grid.lon.values, period=360.0)
    y_axis = core.Axis(grid.lat.values)
    matrix = np.ascontiguousarray(grid.mss.values.transpose())
    if np.issubdtype(dtype, np.integer):
        matrix[np.isnan(matrix)] = np.iinfo(
            dtype).min  # type: ignore[type-var]
    matrix = matrix.astype(dtype)

    grid = getattr(core, f"Grid2D{_get_suffix(dtype)}")(x_axis, y_axis, matrix)
    assert len(grid.x) == matrix.shape[0]
    assert len(grid.y) == matrix.shape[1]
    assert np.shares_memory(grid.array, matrix)
    assert 'Grid2D' in repr(grid)
    np.testing.assert_array_equal(grid.array, matrix)


@pytest.mark.parametrize(
    'dtype',
    [
        np.int8,
        np.float32,
        np.float64,
    ],
)
@pytest.mark.parametrize('temporal_axis', [True, False])
def test_grid3d_load(dtype: DType, temporal_axis: bool) -> None:
    """Test loading a 3D grid."""
    grid = load_grid3d()
    x_axis = core.Axis(grid.longitude.values, period=360.0)
    y_axis = core.Axis(grid.latitude.values)
    z_axis = (core.TemporalAxis(grid.time.values) if temporal_axis else
              core.Axis(grid.time.values.astype('float64')))
    matrix = np.ascontiguousarray(grid.tcw.values.transpose())
    if np.issubdtype(dtype, np.integer):
        matrix[np.isnan(matrix)] = np.iinfo(
            dtype).min  # type: ignore[type-var]
    matrix = matrix.astype(dtype)
    class_name = 'TemporalGrid3D' if temporal_axis else 'Grid3D'
    grid = getattr(core, f"{class_name}{_get_suffix(dtype)}")(x_axis, y_axis,
                                                              z_axis, matrix)
    assert len(grid.x) == matrix.shape[0]
    assert len(grid.y) == matrix.shape[1]
    assert len(grid.z) == matrix.shape[2]
    assert np.shares_memory(grid.array, matrix)
    assert class_name in repr(grid)
    np.testing.assert_array_equal(grid.array, matrix)


@pytest.mark.parametrize(
    'dtype',
    [
        np.int8,
        np.float32,
        np.float64,
    ],
)
@pytest.mark.parametrize('temporal_axis', [True, False])
def test_grid4d_load(dtype: DType, temporal_axis: bool) -> None:
    """Test loading a 4D grid."""
    grid = load_grid4d()
    x_axis = core.Axis(grid.longitude.values, period=360.0)
    y_axis = core.Axis(grid.latitude.values)
    z_axis = (core.TemporalAxis(grid.time.values) if temporal_axis else
              core.Axis(grid.time.values.astype('float64')))
    u_axis = core.Axis(grid.level.values)
    matrix = np.ascontiguousarray(grid.temperature.values.transpose())
    if np.issubdtype(dtype, np.integer):
        matrix[np.isnan(matrix)] = np.iinfo(
            dtype).min  # type: ignore[type-var]
    matrix = matrix.astype(dtype)
    class_name = 'TemporalGrid4D' if temporal_axis else 'Grid4D'
    grid = getattr(core,
                   f"{class_name}{_get_suffix(dtype)}")(x_axis, y_axis, z_axis,
                                                        u_axis, matrix)
    assert len(grid.x) == matrix.shape[0]
    assert len(grid.y) == matrix.shape[1]
    assert len(grid.z) == matrix.shape[2]
    assert len(grid.u) == matrix.shape[3]
    assert np.shares_memory(grid.array, matrix)
    assert class_name in repr(grid)
    np.testing.assert_array_equal(grid.array, matrix)

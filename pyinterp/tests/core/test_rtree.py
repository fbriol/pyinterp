# Copyright (c) 2025 CNES
#
# All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE file.
"""Unit tests for RTree3D with parametrized Float32/Float64 tests."""

from __future__ import annotations

import pickle
from typing import TYPE_CHECKING

import numpy as np
import pytest

from pyinterp import core

if TYPE_CHECKING:
    from _pytest.fixtures import FixtureRequest


@pytest.fixture(params=[core.RTree3DFloat64, core.RTree3DFloat32])
def rtree_class(
    request: FixtureRequest,
) -> type[core.RTree3DFloat64 | core.RTree3DFloat32]:
    """Parametrize tests with RTree3D classes (Float64 and Float32)."""
    return request.param  # type: ignore[no-any-return]


@pytest.fixture(params=[np.float64, np.float32])
def float_dtype(request: FixtureRequest) -> type[np.float64 | np.float32]:
    """Parametrize tests with numpy float dtypes."""
    return request.param  # type: ignore[no-any-return]


class TestRTree3DInitialization:
    """Tests for RTree3D initialization."""

    def test_init_no_spheroid_float64(self) -> None:
        """Test RTree3DFloat64 initialization without spheroid."""
        tree = core.RTree3DFloat64()
        assert tree.spheroid is None
        assert tree.empty()
        assert tree.size() == 0

    def test_init_no_spheroid_float32(self) -> None:
        """Test RTree3DFloat32 initialization without spheroid."""
        tree = core.RTree3DFloat32()
        assert tree.spheroid is None
        assert tree.empty()
        assert tree.size() == 0

    def test_init_with_spheroid_float64(self) -> None:
        """Test RTree3DFloat64 initialization with spheroid."""
        spheroid = core.geodetic.Spheroid()
        tree = core.RTree3DFloat64(spheroid=spheroid)
        assert tree.spheroid is not None
        assert tree.empty()
        assert tree.size() == 0

    def test_init_with_spheroid_float32(self) -> None:
        """Test RTree3DFloat32 initialization with spheroid."""
        spheroid = core.geodetic.Spheroid()
        tree = core.RTree3DFloat32(spheroid=spheroid)
        assert tree.spheroid is not None
        assert tree.empty()
        assert tree.size() == 0

    def test_init_parametrized(
        self,
        rtree_class: type[core.RTree3DFloat64 | core.RTree3DFloat32],
    ) -> None:
        """Test initialization for both Float64 and Float32."""
        tree = rtree_class()
        assert tree.empty()
        assert tree.size() == 0


class TestRTree3DPacking:
    """Tests for RTree3D packing operations."""

    def test_packing_3d_cartesian_float64(self) -> None:
        """Test packing 3D Cartesian points with float64."""
        tree = core.RTree3DFloat64()

        coordinates = np.array(
            [
                [0.0, 0.0, 0.0],
                [1.0, 0.0, 0.0],
                [0.0, 1.0, 0.0],
                [0.0, 0.0, 1.0],
                [1.0, 1.0, 1.0],
            ],
            dtype=np.float64,
        )

        values = np.array([10.0, 20.0, 30.0, 40.0, 50.0], dtype=np.float64)

        tree.packing(coordinates, values)

        assert not tree.empty()
        assert tree.size() == 5

    def test_packing_3d_cartesian_float32(self) -> None:
        """Test packing 3D Cartesian points with float32."""
        tree = core.RTree3DFloat32()

        coordinates = np.array(
            [
                [0.0, 0.0, 0.0],
                [1.0, 0.0, 0.0],
                [0.0, 1.0, 0.0],
                [0.0, 0.0, 1.0],
                [1.0, 1.0, 1.0],
            ],
            dtype=np.float32,
        )

        values = np.array([10.0, 20.0, 30.0, 40.0, 50.0], dtype=np.float32)

        tree.packing(coordinates, values)

        assert not tree.empty()
        assert tree.size() == 5

    def test_packing_2d_float64(self) -> None:
        """Test packing 2D points (Z=0) with float64."""
        tree = core.RTree3DFloat64()

        coordinates = np.array(
            [[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0]], dtype=np.float64
        )

        values = np.array([10.0, 20.0, 30.0, 40.0], dtype=np.float64)

        tree.packing(coordinates, values)

        assert not tree.empty()
        assert tree.size() == 4

    def test_packing_2d_float32(self) -> None:
        """Test packing 2D points (Z=0) with float32."""
        tree = core.RTree3DFloat32()

        coordinates = np.array(
            [[0.0, 0.0], [1.0, 0.0], [1.0, 1.0], [0.0, 1.0]], dtype=np.float32
        )

        values = np.array([10.0, 20.0, 30.0, 40.0], dtype=np.float32)

        tree.packing(coordinates, values)

        assert not tree.empty()
        assert tree.size() == 4

    def test_packing_parametrized_3d(
        self,
        rtree_class: type[core.RTree3DFloat64 | core.RTree3DFloat32],
        float_dtype: type[np.float64 | np.float32],
    ) -> None:
        """Test packing 3D points for both Float64 and Float32."""
        tree = rtree_class()

        coordinates = np.array(
            [
                [0.0, 0.0, 0.0],
                [1.0, 0.0, 0.0],
                [0.0, 1.0, 0.0],
            ],
            dtype=float_dtype,
        )

        values = np.array([10.0, 20.0, 30.0], dtype=float_dtype)

        tree.packing(coordinates, values)  # type: ignore[arg-type]

        assert tree.size() == 3
        assert not tree.empty()

    def test_packing_geodetic_float64(self) -> None:
        """Test packing geodetic (lon, lat, alt) coordinates with float64."""
        spheroid = core.geodetic.Spheroid()
        tree = core.RTree3DFloat64(spheroid=spheroid)

        # Sample points around Paris
        coordinates = np.array(
            [
                [2.35, 48.85, 0.0],  # Paris
                [2.45, 48.90, 100.0],  # Northeast
                [2.25, 48.80, 50.0],  # Southwest
            ],
            dtype=np.float64,
        )

        values = np.array([100.0, 150.0, 120.0], dtype=np.float64)

        tree.packing(coordinates, values)

        assert tree.size() == 3
        assert tree.spheroid is not None

    def test_packing_geodetic_float32(self) -> None:
        """Test packing geodetic (lon, lat, alt) coordinates with float32."""
        spheroid = core.geodetic.Spheroid()
        tree = core.RTree3DFloat32(spheroid=spheroid)

        coordinates = np.array(
            [
                [2.35, 48.85, 0.0],
                [2.45, 48.90, 100.0],
                [2.25, 48.80, 50.0],
            ],
            dtype=np.float32,
        )

        values = np.array([100.0, 150.0, 120.0], dtype=np.float32)

        tree.packing(coordinates, values)

        assert tree.size() == 3
        assert tree.spheroid is not None

    def test_packing_geodetic_parametrized(
        self,
        rtree_class: type[core.RTree3DFloat64 | core.RTree3DFloat32],
        float_dtype: type[np.float64 | np.float32],
    ) -> None:
        """Test packing geodetic coordinates for both Float64 and Float32."""
        spheroid = core.geodetic.Spheroid()
        tree = rtree_class(spheroid=spheroid)

        coordinates = np.array(
            [
                [2.35, 48.85, 0.0],
                [2.45, 48.90, 100.0],
                [2.25, 48.80, 50.0],
            ],
            dtype=float_dtype,
        )

        values = np.array([100.0, 150.0, 120.0], dtype=float_dtype)

        tree.packing(coordinates, values)  # type: ignore[arg-type]

        assert tree.size() == 3
        assert tree.spheroid is not None


class TestRTree3DInsertion:
    """Tests for RTree3D insertion operations."""

    def test_insert_float64(self) -> None:
        """Test incremental insertion with float64."""
        tree = core.RTree3DFloat64()

        coords1 = np.array([[0.0, 0.0, 0.0], [1.0, 0.0, 0.0]], dtype=np.float64)
        values1 = np.array([10.0, 20.0], dtype=np.float64)
        tree.insert(coords1, values1)

        assert tree.size() == 2

        coords2 = np.array([[0.0, 1.0, 0.0]], dtype=np.float64)
        values2 = np.array([30.0], dtype=np.float64)
        tree.insert(coords2, values2)

        assert tree.size() == 3

    def test_insert_float32(self) -> None:
        """Test incremental insertion with float32."""
        tree = core.RTree3DFloat32()

        coords1 = np.array([[0.0, 0.0, 0.0], [1.0, 0.0, 0.0]], dtype=np.float32)
        values1 = np.array([10.0, 20.0], dtype=np.float32)
        tree.insert(coords1, values1)

        assert tree.size() == 2

        coords2 = np.array([[0.0, 1.0, 0.0]], dtype=np.float32)
        values2 = np.array([30.0], dtype=np.float32)
        tree.insert(coords2, values2)

        assert tree.size() == 3

    def test_insert_parametrized(
        self,
        rtree_class: type[core.RTree3DFloat64 | core.RTree3DFloat32],
        float_dtype: type[np.float64 | np.float32],
    ) -> None:
        """Test insertion for both Float64 and Float32."""
        tree = rtree_class()

        coordinates = np.array(
            [[0.0, 0.0, 0.0], [1.0, 0.0, 0.0], [0.0, 1.0, 0.0]],
            dtype=float_dtype,
        )
        values = np.array([10.0, 20.0, 30.0], dtype=float_dtype)

        tree.insert(coordinates, values)  # type: ignore[arg-type]

        assert tree.size() == 3


class TestRTree3DMaintenanceOperations:
    """Tests for RTree3D maintenance operations."""

    def test_clear_float64(self) -> None:
        """Test clearing tree with float64."""
        tree = core.RTree3DFloat64()

        coordinates = np.array(
            [[0.0, 0.0, 0.0], [1.0, 1.0, 1.0]], dtype=np.float64
        )
        values = np.array([10.0, 20.0], dtype=np.float64)

        tree.packing(coordinates, values)
        assert not tree.empty()
        assert tree.size() == 2

        tree.clear()
        assert tree.empty()
        assert tree.size() == 0

    def test_clear_float32(self) -> None:
        """Test clearing tree with float32."""
        tree = core.RTree3DFloat32()

        coordinates = np.array(
            [[0.0, 0.0, 0.0], [1.0, 1.0, 1.0]], dtype=np.float32
        )
        values = np.array([10.0, 20.0], dtype=np.float32)

        tree.packing(coordinates, values)
        assert not tree.empty()
        assert tree.size() == 2

        tree.clear()
        assert tree.empty()
        assert tree.size() == 0

    def test_clear_parametrized(
        self,
        rtree_class: type[core.RTree3DFloat64 | core.RTree3DFloat32],
        float_dtype: type[np.float64 | np.float32],
    ) -> None:
        """Test clearing for both Float64 and Float32."""
        tree = rtree_class()

        coordinates = np.array(
            [[0.0, 0.0, 0.0], [1.0, 1.0, 1.0]], dtype=float_dtype
        )
        values = np.array([10.0, 20.0], dtype=float_dtype)

        tree.packing(coordinates, values)  # type: ignore[arg-type]
        assert tree.size() == 2

        tree.clear()
        assert tree.empty()

    def test_bounds_empty_float64(self) -> None:
        """Test bounds on empty tree with float64."""
        tree = core.RTree3DFloat64()
        assert tree.bounds() is None

    def test_bounds_empty_float32(self) -> None:
        """Test bounds on empty tree with float32."""
        tree = core.RTree3DFloat32()
        assert tree.bounds() is None

    def test_bounds_with_points_float64(self) -> None:
        """Test bounds calculation with float64."""
        tree = core.RTree3DFloat64()

        coordinates = np.array(
            [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0], [2.5, 3.5, 4.5]],
            dtype=np.float64,
        )
        values = np.array([10.0, 20.0, 30.0], dtype=np.float64)

        tree.packing(coordinates, values)
        bounds = tree.bounds()

        assert bounds is not None
        min_point, max_point = bounds
        assert len(min_point) == 3
        assert len(max_point) == 3

    def test_bounds_with_points_float32(self) -> None:
        """Test bounds calculation with float32."""
        tree = core.RTree3DFloat32()

        coordinates = np.array(
            [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0], [2.5, 3.5, 4.5]],
            dtype=np.float32,
        )
        values = np.array([10.0, 20.0, 30.0], dtype=np.float32)

        tree.packing(coordinates, values)
        bounds = tree.bounds()

        assert bounds is not None
        min_point, max_point = bounds
        assert len(min_point) == 3
        assert len(max_point) == 3

    def test_bounds_parametrized(
        self,
        rtree_class: type[core.RTree3DFloat64 | core.RTree3DFloat32],
        float_dtype: type[np.float64 | np.float32],
    ) -> None:
        """Test bounds for both Float64 and Float32."""
        tree = rtree_class()

        coordinates = np.array(
            [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]], dtype=float_dtype
        )
        values = np.array([10.0, 20.0], dtype=float_dtype)

        tree.packing(coordinates, values)  # type: ignore[arg-type]
        bounds = tree.bounds()

        assert bounds is not None


class TestRTree3DPickling:
    """Tests for RTree3D serialization."""

    def test_pickle_float64(self) -> None:
        """Test pickling and unpickling with float64."""
        tree = core.RTree3DFloat64()

        coordinates = np.array(
            [[0.0, 0.0, 0.0], [1.0, 1.0, 1.0], [2.0, 2.0, 2.0]],
            dtype=np.float64,
        )
        values = np.array([10.0, 20.0, 30.0], dtype=np.float64)

        tree.packing(coordinates, values)

        tree_bytes = pickle.dumps(tree)
        tree2 = pickle.loads(tree_bytes)

        assert tree2.size() == 3
        assert not tree2.empty()

    def test_pickle_float32(self) -> None:
        """Test pickling and unpickling with float32."""
        tree = core.RTree3DFloat32()

        coordinates = np.array(
            [[0.0, 0.0, 0.0], [1.0, 1.0, 1.0], [2.0, 2.0, 2.0]],
            dtype=np.float32,
        )
        values = np.array([10.0, 20.0, 30.0], dtype=np.float32)

        tree.packing(coordinates, values)

        tree_bytes = pickle.dumps(tree)
        tree2 = pickle.loads(tree_bytes)

        assert tree2.size() == 3
        assert not tree2.empty()

    def test_pickle_parametrized(
        self,
        rtree_class: type[core.RTree3DFloat64 | core.RTree3DFloat32],
        float_dtype: type[np.float64 | np.float32],
    ) -> None:
        """Test pickling for both Float64 and Float32."""
        tree = rtree_class()

        coordinates = np.array(
            [[0.0, 0.0, 0.0], [1.0, 1.0, 1.0]], dtype=float_dtype
        )
        values = np.array([10.0, 20.0], dtype=float_dtype)

        tree.packing(coordinates, values)  # type: ignore[arg-type]

        tree_copy = pickle.loads(pickle.dumps(tree))

        assert tree_copy.size() == 2

    def test_pickle_geodetic_float64(self) -> None:
        """Test pickling with geodetic spheroid float64."""
        spheroid = core.geodetic.Spheroid()
        tree = core.RTree3DFloat64(spheroid=spheroid)

        coordinates = np.array(
            [[2.35, 48.85, 0.0], [2.45, 48.90, 100.0]], dtype=np.float64
        )
        values = np.array([100.0, 150.0], dtype=np.float64)

        tree.packing(coordinates, values)

        tree_copy = pickle.loads(pickle.dumps(tree))

        assert tree_copy.size() == 2
        assert tree_copy.spheroid is not None


class TestRTree3DEdgeCases:
    """Test edge cases and error conditions."""

    def test_single_point_float64(self) -> None:
        """Test tree with a single point with float64."""
        tree = core.RTree3DFloat64()

        coordinates = np.array([[0.0, 0.0, 0.0]], dtype=np.float64)
        values = np.array([42.0], dtype=np.float64)

        tree.packing(coordinates, values)

        assert tree.size() == 1

    def test_single_point_float32(self) -> None:
        """Test tree with a single point with float32."""
        tree = core.RTree3DFloat32()

        coordinates = np.array([[0.0, 0.0, 0.0]], dtype=np.float32)
        values = np.array([42.0], dtype=np.float32)

        tree.packing(coordinates, values)

        assert tree.size() == 1

    def test_duplicate_coordinates_float64(self) -> None:
        """Test inserting duplicate coordinates with float64."""
        tree = core.RTree3DFloat64()

        coordinates = np.array(
            [[0.0, 0.0, 0.0], [0.0, 0.0, 0.0], [1.0, 1.0, 1.0]],
            dtype=np.float64,
        )
        values = np.array([10.0, 20.0, 30.0], dtype=np.float64)

        tree.packing(coordinates, values)

        assert tree.size() == 3

    def test_duplicate_coordinates_float32(self) -> None:
        """Test inserting duplicate coordinates with float32."""
        tree = core.RTree3DFloat32()

        coordinates = np.array(
            [[0.0, 0.0, 0.0], [0.0, 0.0, 0.0], [1.0, 1.0, 1.0]],
            dtype=np.float32,
        )
        values = np.array([10.0, 20.0, 30.0], dtype=np.float32)

        tree.packing(coordinates, values)

        assert tree.size() == 3

    def test_large_values_float64(self) -> None:
        """Test with large coordinate values (ECEF) with float64."""
        tree = core.RTree3DFloat64()

        coordinates = np.array(
            [
                [6378137.0, 0.0, 0.0],
                [0.0, 6378137.0, 0.0],
                [0.0, 0.0, 6356752.0],
            ],
            dtype=np.float64,
        )
        values = np.array([10.0, 20.0, 30.0], dtype=np.float64)

        tree.packing(coordinates, values)

        assert tree.size() == 3

    def test_large_values_float32(self) -> None:
        """Test with large coordinate values (ECEF) with float32."""
        tree = core.RTree3DFloat32()

        coordinates = np.array(
            [
                [6378137.0, 0.0, 0.0],
                [0.0, 6378137.0, 0.0],
                [0.0, 0.0, 6356752.0],
            ],
            dtype=np.float32,
        )
        values = np.array([10.0, 20.0, 30.0], dtype=np.float32)

        tree.packing(coordinates, values)

        assert tree.size() == 3

    def test_small_values_float64(self) -> None:
        """Test with very small coordinate values with float64."""
        tree = core.RTree3DFloat64()

        coordinates = np.array(
            [
                [1e-10, 1e-10, 1e-10],
                [2e-10, 2e-10, 2e-10],
                [3e-10, 3e-10, 3e-10],
            ],
            dtype=np.float64,
        )
        values = np.array([10.0, 20.0, 30.0], dtype=np.float64)

        tree.packing(coordinates, values)

        assert tree.size() == 3

    def test_small_values_float32(self) -> None:
        """Test with very small coordinate values with float32."""
        tree = core.RTree3DFloat32()

        coordinates = np.array(
            [
                [1e-10, 1e-10, 1e-10],
                [2e-10, 2e-10, 2e-10],
                [3e-10, 3e-10, 3e-10],
            ],
            dtype=np.float32,
        )
        values = np.array([10.0, 20.0, 30.0], dtype=np.float32)

        tree.packing(coordinates, values)

        assert tree.size() == 3

    def test_negative_coordinates_float64(self) -> None:
        """Test with negative coordinates with float64."""
        tree = core.RTree3DFloat64()

        coordinates = np.array(
            [[-1.0, -1.0, -1.0], [1.0, 1.0, 1.0], [-1.0, 1.0, -1.0]],
            dtype=np.float64,
        )
        values = np.array([10.0, 20.0, 30.0], dtype=np.float64)

        tree.packing(coordinates, values)

        assert tree.size() == 3

    def test_negative_coordinates_float32(self) -> None:
        """Test with negative coordinates with float32."""
        tree = core.RTree3DFloat32()

        coordinates = np.array(
            [[-1.0, -1.0, -1.0], [1.0, 1.0, 1.0], [-1.0, 1.0, -1.0]],
            dtype=np.float32,
        )
        values = np.array([10.0, 20.0, 30.0], dtype=np.float32)

        tree.packing(coordinates, values)

        assert tree.size() == 3

    def test_mismatched_dimensions_float64(self) -> None:
        """Test error handling for mismatched dimensions with float64."""
        tree = core.RTree3DFloat64()

        coordinates = np.array(
            [[0.0, 0.0, 0.0], [1.0, 1.0, 1.0]], dtype=np.float64
        )
        values = np.array([10.0, 20.0, 30.0], dtype=np.float64)

        with pytest.raises(
            ValueError,
            match="Number of coordinates must match number of values",
        ):
            tree.packing(coordinates, values)

    def test_mismatched_dimensions_float32(self) -> None:
        """Test error handling for mismatched dimensions with float32."""
        tree = core.RTree3DFloat32()

        coordinates = np.array(
            [[0.0, 0.0, 0.0], [1.0, 1.0, 1.0]], dtype=np.float32
        )
        values = np.array([10.0, 20.0, 30.0], dtype=np.float32)

        with pytest.raises(
            ValueError,
            match="Number of coordinates must match number of values",
        ):
            tree.packing(coordinates, values)

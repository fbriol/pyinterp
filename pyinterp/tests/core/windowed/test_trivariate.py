# Copyright (c) 2025 CNES
#
# All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE file.
"""Unit tests for windowed trivariate interpolation."""
from __future__ import annotations

import numpy as np
import pytest

from pyinterp import core
from pyinterp.core.config import windowed

from ... import load_grid3d


class TestTrivariateWindowed:
    """Test windowed trivariate interpolation."""

    @staticmethod
    def create_analytical_grid3d(
        dtype: type[np.float32] | type[np.float64]
    ) -> core.Grid3DFloat64 | core.Grid3DFloat32:
        """
        Create a 3D grid with an analytical field.

        f(x, y, z) = sin(x) * cos(y) * exp(-z/10)

        This provides a smooth, continuous field with known values.
        """
        x_vals = np.linspace(0, 2 * np.pi, 15)
        y_vals = np.linspace(0, np.pi, 12)
        z_vals = np.linspace(0, 10, 8)

        x_axis = core.Axis(x_vals, period=None)
        y_axis = core.Axis(y_vals)
        z_axis = core.Axis(z_vals)

        x_grid, y_grid, z_grid = np.meshgrid(x_vals,
                                             y_vals,
                                             z_vals,
                                             indexing='ij')

        # Create analytical field: f(x, y, z) = sin(x) * cos(y) * exp(-z/10)
        data = (np.sin(x_grid) * np.cos(y_grid) *
                np.exp(-z_grid / 10)).astype(dtype)
        # Ensure C-contiguous for grid creation
        data = np.ascontiguousarray(data)

        class_name = (core.Grid3DFloat32
                      if dtype == np.float32 else core.Grid3DFloat64)
        return class_name(x_axis, y_axis, z_axis, data)

    def test_single_point_bilinear(self) -> None:
        """Test windowed trivariate interpolation at a single point with bilinear method."""
        grid = self.create_analytical_grid3d(np.float64)

        # Test point: (π, π/2, 0)
        # Expected: sin(π) * cos(π/2) * exp(0) = 0 * 0 * 1 = 0
        x = np.array([np.pi])
        y = np.array([np.pi / 2])
        z = np.array([0.0])

        config = windowed.Trivariate.bilinear()
        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (1, )
        # Should be close to 0 (within interpolation error)
        assert np.abs(result[0]) < 0.05

    def test_multiple_points_bilinear(self) -> None:
        """Test windowed trivariate interpolation at multiple points."""
        grid = self.create_analytical_grid3d(np.float64)

        # Multiple test points
        x = np.array([np.pi / 4, np.pi / 2, 3 * np.pi / 4])
        y = np.array([np.pi / 4, np.pi / 2, 3 * np.pi / 4])
        z = np.array([0.0, 5.0, 10.0])

        config = windowed.Trivariate.bilinear()
        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (3, )
        assert np.all(np.isfinite(result))

    def test_linear_method(self) -> None:
        """Test windowed trivariate interpolation with linear method."""
        grid = self.create_analytical_grid3d(np.float64)

        x = np.array([np.pi / 2])
        y = np.array([np.pi / 4])
        z = np.array([2.5])

        config = windowed.Trivariate.linear()
        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (1, )
        assert np.isfinite(result[0])

    def test_cubic_method(self) -> None:
        """Test windowed trivariate interpolation with cubic method."""
        grid = self.create_analytical_grid3d(np.float64)

        x = np.array([np.pi / 3])
        y = np.array([np.pi / 3])
        z = np.array([5.0])

        config = windowed.Trivariate.bicubic()
        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (1, )
        assert np.isfinite(result[0])

    def test_spline_method(self) -> None:
        """Test windowed trivariate interpolation with spline method."""
        grid = self.create_analytical_grid3d(np.float64)

        x = np.array([np.pi / 2])
        y = np.array([np.pi / 2])
        z = np.array([3.0])

        config = windowed.Trivariate.c_spline()
        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (1, )
        assert np.isfinite(result[0])

    def test_bounds_error(self) -> None:
        """Test bounds_error parameter with windowed."""
        grid = self.create_analytical_grid3d(np.float64)

        # Point outside grid bounds
        x = np.array([3 * np.pi])
        y = np.array([0.0])
        z = np.array([0.0])

        # With bounds_error=False, should return NaN
        config = windowed.Trivariate.bilinear()
        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (1, )
        assert np.isnan(result[0])

        # With bounds_error=True, should raise an error
        config = windowed.Trivariate.bilinear().bounds_error(True)
        with pytest.raises(ValueError, match='out of bounds'):
            core.trivariate(grid, x, y, z, config)

    def test_window_size_configuration(self) -> None:
        """Test windowed trivariate with different window sizes."""
        grid = self.create_analytical_grid3d(np.float64)

        x = np.array([np.pi / 2, np.pi / 2])
        y = np.array([np.pi / 3, np.pi / 3])
        z = np.array([3.0, 3.0])

        # Test with small window
        config_small = windowed.Trivariate.bilinear().with_window_size_x(
            3).with_window_size_y(3)
        result_small = core.trivariate(grid, x[:1], y[:1], z[:1], config_small)

        # Test with larger window
        config_large = windowed.Trivariate.bilinear().with_window_size_x(
            7).with_window_size_y(5)
        result_large = core.trivariate(grid, x[1:2], y[1:2], z[1:2],
                                       config_large)

        assert result_small.shape == (1, )
        assert result_large.shape == (1, )
        assert np.isfinite(result_small[0])
        assert np.isfinite(result_large[0])

    def test_boundary_mode_expand(self) -> None:
        """Test boundary mode EXPAND."""
        grid = self.create_analytical_grid3d(np.float64)

        x = np.array([np.pi / 2])
        y = np.array([np.pi / 2])
        z = np.array([5.0])

        config = windowed.Trivariate.bilinear().with_boundary_mode(
            windowed.Boundary.EXPAND).with_window_size_x(5).with_window_size_y(
                5)
        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (1, )
        assert np.isfinite(result[0])

    def test_boundary_mode_sym(self) -> None:
        """Test boundary mode SYM."""
        grid = self.create_analytical_grid3d(np.float64)

        x = np.array([np.pi / 2])
        y = np.array([np.pi / 2])
        z = np.array([5.0])

        config = windowed.Trivariate.bilinear().with_boundary_mode(
            windowed.Boundary.SYM).with_window_size_x(5).with_window_size_y(5)
        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (1, )
        assert np.isfinite(result[0])

    def test_boundary_mode_wrap(self) -> None:
        """Test boundary mode WRAP."""
        grid = self.create_analytical_grid3d(np.float64)

        x = np.array([np.pi / 2])
        y = np.array([np.pi / 2])
        z = np.array([5.0])

        config = windowed.Trivariate.bilinear().with_boundary_mode(
            windowed.Boundary.WRAP).with_window_size_x(5).with_window_size_y(5)
        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (1, )
        assert np.isfinite(result[0])

    def test_with_real_data(self) -> None:
        """Test windowed trivariate interpolation with real grid data."""
        grid_data = load_grid3d()
        x_axis = core.Axis(grid_data.longitude.values, period=360.0)
        y_axis = core.Axis(grid_data.latitude.values)
        z_axis = core.Axis(grid_data.time.values.astype('float64'))

        matrix = np.ascontiguousarray(grid_data.tcw.values.transpose())
        grid = core.Grid3DFloat64(x_axis, y_axis, z_axis, matrix)

        # Test points within bounds
        x = np.array([10.0, 20.0, 30.0])
        y = np.array([-10.0, 0.0, 10.0])
        z = np.array([
            grid_data.time.values[0].astype('float64'),
            grid_data.time.values[1].astype('float64'),
            grid_data.time.values[-1].astype('float64')
        ])

        config = windowed.Trivariate.bilinear().with_window_size_x(
            5).with_window_size_y(5)
        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (3, )
        # At least some values should be finite (not all NaNs)
        assert np.any(np.isfinite(result))

    def test_dtype_float32(self) -> None:
        """Test windowed trivariate interpolation with float32 data."""
        grid = self.create_analytical_grid3d(np.float32)

        x = np.array([np.pi / 4])
        y = np.array([np.pi / 4])
        z = np.array([5.0])

        config = windowed.Trivariate.bilinear()
        result = core.trivariate(grid, x, y, z, config)

        assert result.dtype == np.float32
        assert np.isfinite(result[0])

    def test_num_threads(self) -> None:
        """Test windowed trivariate interpolation with different thread counts."""
        grid = self.create_analytical_grid3d(np.float64)

        x = np.array([np.pi / 4, np.pi / 2, 3 * np.pi / 4])
        y = np.array([np.pi / 4, np.pi / 2, 3 * np.pi / 4])
        z = np.array([1.0, 5.0, 9.0])

        # Test with 1 thread
        config_single = windowed.Trivariate.bilinear().num_threads(1)
        result_single = core.trivariate(grid, x, y, z, config_single)

        # Test with multiple threads
        config_multi = windowed.Trivariate.bilinear().num_threads(4)
        result_multi = core.trivariate(grid, x, y, z, config_multi)

        # Results should be identical or very close
        np.testing.assert_array_almost_equal(result_single, result_multi)

    def test_continuity(self) -> None:
        """Test that windowed interpolation is continuous (smooth)."""
        grid = self.create_analytical_grid3d(np.float64)

        # Create two close points
        x1 = np.array([1.0])
        y1 = np.array([1.0])
        z1 = np.array([2.0])

        x2 = np.array([1.05])
        y2 = np.array([1.05])
        z2 = np.array([2.05])

        config = windowed.Trivariate.bilinear().with_window_size_x(
            5).with_window_size_y(5)
        result1 = core.trivariate(grid, x1, y1, z1, config)
        result2 = core.trivariate(grid, x2, y2, z2, config)

        # Results should be close (continuity)
        assert np.abs(result1[0] - result2[0]) < 0.1

    def test_corner_point(self) -> None:
        """Test windowed interpolation at grid corner."""
        grid = self.create_analytical_grid3d(np.float64)

        # Get actual corner values from grid
        x_val = grid.x[0]
        y_val = grid.y[0]
        z_val = grid.z[0]

        x = np.array([x_val])
        y = np.array([y_val])
        z = np.array([z_val])

        config = windowed.Trivariate.bilinear()
        result = core.trivariate(grid, x, y, z, config)

        expected = grid.array[0, 0, 0]

        assert np.isfinite(result[0])
        # Windowed may have slightly more error at boundaries
        assert np.abs(result[0] - expected) < 0.1

    def test_analytical_accuracy(self) -> None:
        """Test windowed interpolation accuracy against analytical function."""
        grid = self.create_analytical_grid3d(np.float64)

        # Define the analytical function
        def analytical_func(x: float, y: float, z: float) -> float:
            return float(np.sin(x) * np.cos(y) * np.exp(-z / 10))

        # Test at interior points
        x = np.array([0.5, 1.0, 1.5, 2.0])
        y = np.array([0.5, 1.0, 1.5, 2.0])
        z = np.array([1.0, 2.0, 3.0, 4.0])

        config = windowed.Trivariate.bilinear().with_window_size_x(
            5).with_window_size_y(5)
        result = core.trivariate(grid, x, y, z, config)

        # Compare with analytical values
        expected = np.array(
            [analytical_func(x[i], y[i], z[i]) for i in range(len(x))])

        # Allow some tolerance for interpolation error
        np.testing.assert_allclose(result, expected, rtol=0.1)

    def test_large_array(self) -> None:
        """Test windowed trivariate interpolation with large arrays."""
        grid = self.create_analytical_grid3d(np.float64)

        # Create large arrays of points
        n_points = 500
        x = np.random.uniform(0.1, 2 * np.pi - 0.1, n_points)
        y = np.random.uniform(0.1, np.pi - 0.1, n_points)
        z = np.random.uniform(0.1, 9.9, n_points)

        config = windowed.Trivariate.bilinear().with_window_size_x(
            5).with_window_size_y(5)
        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (n_points, )
        # Most values should be finite
        assert np.sum(np.isfinite(result)) > n_points * 0.99

    def test_method_chaining(self) -> None:
        """Test that windowed methods can be chained."""
        config = (windowed.Trivariate.bicubic().with_num_threads(
            4).with_bounds_error(True).with_boundary_mode(
                windowed.Boundary.WRAP).with_window_size_x(
                    10).with_window_size_y(8))

        assert isinstance(config, windowed.Trivariate)

    def test_all_interpolation_methods(self) -> None:
        """Test all available windowed trivariate methods."""
        grid = self.create_analytical_grid3d(np.float64)

        methods = [
            'akima',
            'akima_periodic',
            'bicubic',
            'bilinear',
            'c_spline',
            'c_spline_not_a_knot',
            'c_spline_periodic',
            'linear',
            'polynomial',
            'steffen',
        ]

        x = np.array([np.pi / 2])
        y = np.array([np.pi / 2])
        z = np.array([5.0])

        for method in methods:
            config = getattr(windowed.Trivariate, method)()
            result = core.trivariate(grid, x, y, z, config)

            assert result.shape == (1, )
            assert np.isfinite(result[0]), f"Method {method} produced NaN"

    def test_polynomial_method(self) -> None:
        """Test windowed trivariate interpolation with polynomial method."""
        grid = self.create_analytical_grid3d(np.float64)

        x = np.array([np.pi / 2])
        y = np.array([np.pi / 3])
        z = np.array([4.0])

        config = windowed.Trivariate.polynomial().with_window_size_x(
            5).with_window_size_y(5)
        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (1, )
        assert np.isfinite(result[0])

    def test_steffen_method(self) -> None:
        """Test windowed trivariate interpolation with steffen method."""
        grid = self.create_analytical_grid3d(np.float64)

        x = np.array([np.pi / 2])
        y = np.array([np.pi / 2])
        z = np.array([5.0])

        config = windowed.Trivariate.steffen().with_window_size_x(
            5).with_window_size_y(5)
        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (1, )
        assert np.isfinite(result[0])

    def test_akima_method(self) -> None:
        """Test windowed trivariate interpolation with akima method."""
        grid = self.create_analytical_grid3d(np.float64)

        x = np.array([np.pi / 2])
        y = np.array([np.pi / 2])
        z = np.array([5.0])

        config = windowed.Trivariate.akima().with_window_size_x(
            5).with_window_size_y(5)
        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (1, )
        assert np.isfinite(result[0])

    def test_third_axis_configuration(self) -> None:
        """Test trivariate with explicit third axis configuration."""
        grid = self.create_analytical_grid3d(np.float64)

        x = np.array([np.pi / 2])
        y = np.array([np.pi / 2])
        z = np.array([5.0])

        # Configure third axis
        axis_config = windowed.AxisConfig.linear()
        config = windowed.Trivariate.bilinear().with_third_axis(axis_config)

        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (1, )
        assert np.isfinite(result[0])

    def test_third_axis_nearest(self) -> None:
        """Test trivariate with third axis as nearest neighbor."""
        grid = self.create_analytical_grid3d(np.float64)

        x = np.array([np.pi / 2])
        y = np.array([np.pi / 2])
        z = np.array([5.0])

        # Configure third axis as nearest
        axis_config = windowed.AxisConfig.nearest()
        config = windowed.Trivariate.bilinear().with_third_axis(axis_config)

        result = core.trivariate(grid, x, y, z, config)

        assert result.shape == (1, )
        assert np.isfinite(result[0])

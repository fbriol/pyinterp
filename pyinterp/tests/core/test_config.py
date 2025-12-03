# Copyright (c) 2025 CNES
#
# All rights reserved. Use of this source code is governed by a
# BSD-style license that can be found in the LICENSE file.
"""Unit tests for configuration objects."""
from __future__ import annotations

from pyinterp.core.config import geometric, windowed


class TestGeometric:
    """Test geometric interpolation configurations."""

    def test_bivariate_class_methods(self) -> None:
        """Test Bivariate class methods return instances."""
        bilinear = geometric.Bivariate.bilinear()
        assert isinstance(bilinear, geometric.Bivariate)

        idw = geometric.Bivariate.idw()
        assert isinstance(idw, geometric.Bivariate)

        nearest = geometric.Bivariate.nearest()
        assert isinstance(nearest, geometric.Bivariate)

    def test_bivariate_instance_methods(self) -> None:
        """Test Bivariate instance methods."""
        config = geometric.Bivariate.bilinear()

        # Test bounds_error
        config_bounds = config.bounds_error(True)
        assert isinstance(config_bounds, geometric.Bivariate)
        assert config_bounds is not config

        config_bounds_false = config.bounds_error(False)
        assert isinstance(config_bounds_false, geometric.Bivariate)

        # Test num_threads
        config_threads = config.num_threads(4)
        assert isinstance(config_threads, geometric.Bivariate)
        assert config_threads is not config

    def test_trivariate_class_methods(self) -> None:
        """Test Trivariate class methods return instances."""
        bilinear = geometric.Trivariate.bilinear()
        assert isinstance(bilinear, geometric.Trivariate)

        idw = geometric.Trivariate.idw()
        assert isinstance(idw, geometric.Trivariate)

        nearest = geometric.Trivariate.nearest()
        assert isinstance(nearest, geometric.Trivariate)

    def test_trivariate_instance_methods(self) -> None:
        """Test Trivariate instance methods."""
        config = geometric.Trivariate.bilinear()

        config_bounds = config.bounds_error(True)
        assert isinstance(config_bounds, geometric.Trivariate)
        assert config_bounds is not config

        config_threads = config.num_threads(8)
        assert isinstance(config_threads, geometric.Trivariate)
        assert config_threads is not config

    def test_quadrivariate_class_methods(self) -> None:
        """Test Quadrivariate class methods return instances."""
        bilinear = geometric.Quadrivariate.bilinear()
        assert isinstance(bilinear, geometric.Quadrivariate)

        idw = geometric.Quadrivariate.idw()
        assert isinstance(idw, geometric.Quadrivariate)

        nearest = geometric.Quadrivariate.nearest()
        assert isinstance(nearest, geometric.Quadrivariate)

    def test_quadrivariate_instance_methods(self) -> None:
        """Test Quadrivariate instance methods."""
        config = geometric.Quadrivariate.bilinear()

        config_bounds = config.bounds_error(True)
        assert isinstance(config_bounds, geometric.Quadrivariate)
        assert config_bounds is not config

        config_threads = config.num_threads(2)
        assert isinstance(config_threads, geometric.Quadrivariate)
        assert config_threads is not config


class TestWindowed:
    """Test windowed interpolation configurations."""

    def test_boundary_enum(self) -> None:
        """Test Boundary enum values."""
        assert windowed.Boundary.EXPAND
        assert windowed.Boundary.SYM
        assert windowed.Boundary.UNDEF
        assert windowed.Boundary.WRAP

        # Test that enum values are unique
        values = [
            windowed.Boundary.EXPAND,
            windowed.Boundary.SYM,
            windowed.Boundary.UNDEF,
            windowed.Boundary.WRAP,
        ]
        assert len(set(values)) == 4

    def test_axis_config_class_methods(self) -> None:
        """Test AxisConfig class methods."""
        linear = windowed.AxisConfig.linear()
        assert isinstance(linear, windowed.AxisConfig)

        nearest = windowed.AxisConfig.nearest()
        assert isinstance(nearest, windowed.AxisConfig)

    def test_bivariate_class_methods(self) -> None:
        """Test Bivariate class methods return instances."""
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

        for method in methods:
            config = getattr(windowed.Bivariate, method)()
            assert isinstance(config, windowed.Bivariate)

    def test_bivariate_instance_methods(self) -> None:
        """Test Bivariate instance methods."""
        config = windowed.Bivariate.bilinear()

        # Test bounds_error
        config_bounds = config.bounds_error(True)
        assert isinstance(config_bounds, windowed.Bivariate)
        assert config_bounds is not config

        # Test num_threads
        config_threads = config.num_threads(4)
        assert isinstance(config_threads, windowed.Bivariate)
        assert config_threads is not config

        # Test with_boundary_mode
        config_boundary = config.with_boundary_mode(windowed.Boundary.WRAP)
        assert isinstance(config_boundary, windowed.Bivariate)
        assert config_boundary is not config

        # Test with_bounds_error
        config_bounds2 = config.with_bounds_error(False)
        assert isinstance(config_bounds2, windowed.Bivariate)
        assert config_bounds2 is not config

        # Test with_num_threads
        config_threads2 = config.with_num_threads(8)
        assert isinstance(config_threads2, windowed.Bivariate)
        assert config_threads2 is not config

        # Test with_window_size_x
        config_wsx = config.with_window_size_x(5)
        assert isinstance(config_wsx, windowed.Bivariate)
        assert config_wsx is not config

        # Test with_window_size_y
        config_wsy = config.with_window_size_y(3)
        assert isinstance(config_wsy, windowed.Bivariate)
        assert config_wsy is not config

    def test_trivariate_class_methods(self) -> None:
        """Test Trivariate class methods return instances."""
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

        for method in methods:
            config = getattr(windowed.Trivariate, method)()
            assert isinstance(config, windowed.Trivariate)

    def test_trivariate_instance_methods(self) -> None:
        """Test Trivariate instance methods."""
        config = windowed.Trivariate.bilinear()

        config_bounds = config.bounds_error(True)
        assert isinstance(config_bounds, windowed.Trivariate)
        assert config_bounds is not config

        config_threads = config.num_threads(4)
        assert isinstance(config_threads, windowed.Trivariate)
        assert config_threads is not config

        config_boundary = config.with_boundary_mode(windowed.Boundary.SYM)
        assert isinstance(config_boundary, windowed.Trivariate)
        assert config_boundary is not config

        config_bounds2 = config.with_bounds_error(False)
        assert isinstance(config_bounds2, windowed.Trivariate)
        assert config_bounds2 is not config

        config_threads2 = config.with_num_threads(2)
        assert isinstance(config_threads2, windowed.Trivariate)
        assert config_threads2 is not config

        config_wsx = config.with_window_size_x(7)
        assert isinstance(config_wsx, windowed.Trivariate)
        assert config_wsx is not config

        config_wsy = config.with_window_size_y(5)
        assert isinstance(config_wsy, windowed.Trivariate)
        assert config_wsy is not config

        # Test with_third_axis
        axis_config = windowed.AxisConfig.linear()
        config_axis = config.with_third_axis(axis_config)
        assert isinstance(config_axis, windowed.Trivariate)
        assert config_axis is not config

    def test_quadrivariate_class_methods(self) -> None:
        """Test Quadrivariate class methods return instances."""
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

        for method in methods:
            config = getattr(windowed.Quadrivariate, method)()
            assert isinstance(config, windowed.Quadrivariate)

    def test_quadrivariate_instance_methods(self) -> None:
        """Test Quadrivariate instance methods."""
        config = windowed.Quadrivariate.bilinear()

        config_bounds = config.bounds_error(True)
        assert isinstance(config_bounds, windowed.Quadrivariate)
        assert config_bounds is not config

        config_threads = config.num_threads(4)
        assert isinstance(config_threads, windowed.Quadrivariate)
        assert config_threads is not config

        config_boundary = config.with_boundary_mode(windowed.Boundary.EXPAND)
        assert isinstance(config_boundary, windowed.Quadrivariate)
        assert config_boundary is not config

        config_bounds2 = config.with_bounds_error(False)
        assert isinstance(config_bounds2, windowed.Quadrivariate)
        assert config_bounds2 is not config

        config_threads2 = config.with_num_threads(6)
        assert isinstance(config_threads2, windowed.Quadrivariate)
        assert config_threads2 is not config

        config_wsx = config.with_window_size_x(9)
        assert isinstance(config_wsx, windowed.Quadrivariate)
        assert config_wsx is not config

        config_wsy = config.with_window_size_y(7)
        assert isinstance(config_wsy, windowed.Quadrivariate)
        assert config_wsy is not config

        # Test with_third_axis and with_fourth_axis
        axis_config = windowed.AxisConfig.nearest()
        config_axis3 = config.with_third_axis(axis_config)
        assert isinstance(config_axis3, windowed.Quadrivariate)
        assert config_axis3 is not config

        config_axis4 = config.with_fourth_axis(axis_config)
        assert isinstance(config_axis4, windowed.Quadrivariate)
        assert config_axis4 is not config

    def test_method_chaining(self) -> None:
        """Test that methods can be chained."""
        config = (windowed.Bivariate.bicubic().with_num_threads(
            4).with_bounds_error(True).with_boundary_mode(
                windowed.Boundary.WRAP).with_window_size_x(
                    10).with_window_size_y(8))
        assert isinstance(config, windowed.Bivariate)

    def test_equality(self) -> None:
        """Test configuration equality (basic checks)."""
        config1 = geometric.Bivariate.bilinear().num_threads(4)
        config2 = geometric.Bivariate.bilinear().num_threads(4)
        config3 = geometric.Bivariate.bilinear().num_threads(8)

        assert isinstance(config1, geometric.Bivariate)
        assert isinstance(config2, geometric.Bivariate)
        assert isinstance(config3, geometric.Bivariate)

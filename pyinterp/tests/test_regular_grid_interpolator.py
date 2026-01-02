"""Tests for the regular grid interpolator module."""

import pytest

from pyinterp.core.config import geometric, windowed
from pyinterp.regular_grid_interpolator import (
    _make_geometric_config,
    _make_windowed_config,
    _validate_no_windowed_options,
)


class TestMakeGeometricConfig:
    """Test _make_geometric_config function."""

    def test_bilinear_config(self) -> None:
        """Test creating bilinear geometric config."""
        config = _make_geometric_config(
            "bilinear",
            geometric.Bivariate,
            bounds_error=False,
            num_threads=0,
        )
        assert isinstance(config, geometric.Bivariate)

    def test_nearest_config(self) -> None:
        """Test creating nearest neighbor geometric config."""
        config = _make_geometric_config(
            "nearest",
            geometric.Bivariate,
            bounds_error=False,
            num_threads=0,
        )
        assert isinstance(config, geometric.Bivariate)

    def test_idw_config(self) -> None:
        """Test creating IDW geometric config."""
        config = _make_geometric_config(
            "idw",
            geometric.Bivariate,
            bounds_error=False,
            num_threads=0,
        )
        assert isinstance(config, geometric.Bivariate)

    def test_invalid_method(self) -> None:
        """Test error for invalid method."""
        with pytest.raises(ValueError, match="Unknown geometric method"):
            _make_geometric_config(
                "invalid",  # type: ignore[arg-type]
                geometric.Bivariate,
                bounds_error=False,
                num_threads=0,
            )

    def test_trivariate_config(self) -> None:
        """Test creating trivariate config."""
        config = _make_geometric_config(
            "bilinear",
            geometric.Trivariate,
            bounds_error=True,
            num_threads=2,
        )
        assert isinstance(config, geometric.Trivariate)

    def test_quadrivariate_config(self) -> None:
        """Test creating quadrivariate config."""
        config = _make_geometric_config(
            "nearest",
            geometric.Quadrivariate,
            bounds_error=False,
            num_threads=4,
        )
        assert isinstance(config, geometric.Quadrivariate)


class TestMakeWindowedConfig:
    """Test _make_windowed_config function."""

    def test_bicubic_config(self) -> None:
        """Test creating bicubic windowed config."""
        config = _make_windowed_config(
            "bicubic",
            windowed.Bivariate,
            bounds_error=False,
            num_threads=0,
        )
        assert isinstance(config, windowed.Bivariate)

    def test_akima_config(self) -> None:
        """Test creating akima windowed config."""
        config = _make_windowed_config(
            "akima",
            windowed.Bivariate,
            bounds_error=False,
            num_threads=0,
        )
        assert isinstance(config, windowed.Bivariate)

    def test_linear_config(self) -> None:
        """Test creating linear windowed config."""
        config = _make_windowed_config(
            "linear",
            windowed.Bivariate,
            bounds_error=False,
            num_threads=0,
        )
        assert isinstance(config, windowed.Bivariate)

    def test_invalid_method(self) -> None:
        """Test error for invalid method."""
        with pytest.raises(ValueError, match="Unknown windowed method"):
            _make_windowed_config(
                "invalid",  # type: ignore[arg-type]
                windowed.Bivariate,
                bounds_error=False,
                num_threads=0,
            )

    def test_window_size_x(self) -> None:
        """Test config with window size X."""
        config = _make_windowed_config(
            "bicubic",
            windowed.Bivariate,
            bounds_error=False,
            num_threads=0,
            window_size_x=10,
        )
        assert isinstance(config, windowed.Bivariate)

    def test_window_size_y(self) -> None:
        """Test config with window size Y."""
        config = _make_windowed_config(
            "bicubic",
            windowed.Bivariate,
            bounds_error=False,
            num_threads=0,
            window_size_y=12,
        )
        assert isinstance(config, windowed.Bivariate)

    def test_boundary_mode(self) -> None:
        """Test config with boundary mode."""
        config = _make_windowed_config(
            "bicubic",
            windowed.Bivariate,
            bounds_error=False,
            num_threads=0,
            boundary_mode="symmetric",
        )
        assert isinstance(config, windowed.Bivariate)

    def test_boundary_mode_wrap(self) -> None:
        """Test config with wrap boundary mode."""
        config = _make_windowed_config(
            "bicubic",
            windowed.Bivariate,
            bounds_error=False,
            num_threads=0,
            boundary_mode="wrap",
        )
        assert isinstance(config, windowed.Bivariate)

    def test_trivariate_with_third_axis(self) -> None:
        """Test trivariate config with third axis."""
        config = _make_windowed_config(
            "bicubic",
            windowed.Trivariate,
            bounds_error=False,
            num_threads=0,
            third_axis="linear",
        )
        assert isinstance(config, windowed.Trivariate)

    def test_quadrivariate_with_axes(self) -> None:
        """Test quadrivariate config with axes."""
        config = _make_windowed_config(
            "bicubic",
            windowed.Quadrivariate,
            bounds_error=False,
            num_threads=0,
            third_axis="nearest",
            fourth_axis="linear",
        )
        assert isinstance(config, windowed.Quadrivariate)


class TestValidateNoWindowedOptions:
    """Test _validate_no_windowed_options function."""

    def test_no_options(self) -> None:
        """Test validation passes with no windowed options."""
        # Should not raise
        _validate_no_windowed_options(
            "bilinear",
            window_size_x=None,
            window_size_y=None,
            boundary_mode=None,
        )

    def test_window_size_x_error(self) -> None:
        """Test error for window_size_x with geometric method."""
        with pytest.raises(TypeError, match="window_size_x"):
            _validate_no_windowed_options(
                "bilinear",
                window_size_x=10,
                window_size_y=None,
                boundary_mode=None,
            )

    def test_window_size_y_error(self) -> None:
        """Test error for window_size_y with geometric method."""
        with pytest.raises(TypeError, match="window_size_y"):
            _validate_no_windowed_options(
                "nearest",
                window_size_x=None,
                window_size_y=12,
                boundary_mode=None,
            )

    def test_boundary_mode_error(self) -> None:
        """Test error for boundary_mode with geometric method."""
        with pytest.raises(TypeError, match="boundary_mode"):
            _validate_no_windowed_options(
                "idw",
                window_size_x=None,
                window_size_y=None,
                boundary_mode="symmetric",
            )

    def test_third_axis_error(self) -> None:
        """Test error for third_axis with geometric method."""
        with pytest.raises(TypeError, match="third_axis"):
            _validate_no_windowed_options(
                "bilinear",
                window_size_x=None,
                window_size_y=None,
                boundary_mode=None,
                third_axis="linear",
            )

    def test_fourth_axis_error(self) -> None:
        """Test error for fourth_axis with geometric method."""
        with pytest.raises(TypeError, match="fourth_axis"):
            _validate_no_windowed_options(
                "nearest",
                window_size_x=None,
                window_size_y=None,
                boundary_mode=None,
                fourth_axis="nearest",
            )

    def test_multiple_errors(self) -> None:
        """Test error message with multiple invalid options."""
        with pytest.raises(TypeError) as exc_info:
            _validate_no_windowed_options(
                "bilinear",
                window_size_x=10,
                window_size_y=12,
                boundary_mode="wrap",
            )
        error_msg = str(exc_info.value)
        assert "window_size_x" in error_msg
        assert "window_size_y" in error_msg
        assert "boundary_mode" in error_msg

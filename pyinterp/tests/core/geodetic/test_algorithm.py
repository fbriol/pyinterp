"""Tests for geodetic algorithm functions."""

import numpy as np
import pytest

from pyinterp.core.geodetic import (
    Box,
    MultiPolygon,
    Point,
    Polygon,
    Ring,
    Spheroid,
)
from pyinterp.core.geodetic.algorithms import area, Strategy


def test_strategy_enum() -> None:
    """Test Strategy enum values."""
    assert hasattr(Strategy, "ANDOYER")
    assert hasattr(Strategy, "KARNEY")
    assert hasattr(Strategy, "THOMAS")
    assert hasattr(Strategy, "VINCENTY")

    # Check enum values
    assert Strategy.ANDOYER is not None
    assert Strategy.KARNEY is not None
    assert Strategy.THOMAS is not None
    assert Strategy.VINCENTY is not None

    # Enum should be comparable
    assert Strategy.VINCENTY == Strategy.VINCENTY
    assert Strategy.ANDOYER != Strategy.KARNEY


def test_area_point() -> None:
    """Test area of a point (always 0)."""
    point = Point(10.0, 20.0)
    result = area(point)

    # Points have zero area
    assert result == 0.0

    # All strategies should return 0 for points
    assert area(point, strategy=Strategy.ANDOYER) == 0.0
    assert area(point, strategy=Strategy.KARNEY) == 0.0
    assert area(point, strategy=Strategy.THOMAS) == 0.0
    assert area(point, strategy=Strategy.VINCENTY) == 0.0


def test_area_box() -> None:
    """Test area calculation for a box."""
    # Create a 1x1 degree box at equator
    box = Box((0.0, 0.0), (1.0, 1.0))
    result = area(box)

    # Area should be positive
    assert result > 0

    # Approximately 12,364 km² (1 degree x 1 degree at equator)
    # Convert to m²: ~12,364,000,000 m²
    assert 12.3e9 < result < 12.4e9

    # Larger box should have larger area
    large_box = Box((0.0, 0.0), (10.0, 10.0))
    large_area = area(large_box)
    assert (
        large_area > result * 90
    )  # Should be ~100x but accounting for curvature


def test_area_box_different_strategies() -> None:
    """Test that different strategies give similar but not identical results."""
    box = Box((0.0, 0.0), (1.0, 1.0))

    area_andoyer = area(box, strategy=Strategy.ANDOYER)
    area_karney = area(box, strategy=Strategy.KARNEY)
    area_thomas = area(box, strategy=Strategy.THOMAS)
    area_vincenty = area(box, strategy=Strategy.VINCENTY)

    # All should be positive
    assert area_andoyer > 0
    assert area_karney > 0
    assert area_thomas > 0
    assert area_vincenty > 0

    # All should be within 1% of each other for this simple case
    mean_area = (area_andoyer + area_karney + area_thomas + area_vincenty) / 4
    assert abs(area_andoyer - mean_area) / mean_area < 0.01
    assert abs(area_karney - mean_area) / mean_area < 0.01
    assert abs(area_thomas - mean_area) / mean_area < 0.01
    assert abs(area_vincenty - mean_area) / mean_area < 0.01


def test_area_ring() -> None:
    """Test area calculation for a ring."""
    # Create a square ring (5 points to close the ring)
    # Using counter-clockwise winding order (positive area)
    lon = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    lat = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    ring = Ring(lon, lat)

    result = area(ring)

    # Area should be positive (counter-clockwise winding)
    assert result > 0

    # Should be similar to the box area (same bounds)
    box = Box((0.0, 0.0), (1.0, 1.0))
    box_area = area(box)
    # Ring and box should have very similar areas
    assert abs(result - box_area) / box_area < 0.01


def test_area_polygon_simple() -> None:
    """Test area calculation for a simple polygon."""
    # Create a square polygon with counter-clockwise winding
    lon = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    lat = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    outer = Ring(lon, lat)
    polygon = Polygon(outer)

    result = area(polygon)

    # Area should be positive (counter-clockwise winding)
    assert result > 0

    # Should be similar to the box/ring area
    box_area = area(Box((0.0, 0.0), (1.0, 1.0)))
    assert abs(result - box_area) / box_area < 0.01


def test_area_polygon_with_hole() -> None:
    """Test area calculation for a polygon with a hole."""
    # Create outer ring (0-10 degrees square)
    outer_lon = np.array([0.0, 10.0, 10.0, 0.0, 0.0])
    outer_lat = np.array([0.0, 0.0, 10.0, 10.0, 0.0])
    outer = Ring(outer_lon, outer_lat)

    # Create inner ring (hole, 2-8 degrees square)
    inner_lon = np.array([2.0, 8.0, 8.0, 2.0, 2.0])
    inner_lat = np.array([2.0, 2.0, 8.0, 8.0, 2.0])
    inner = Ring(inner_lon, inner_lat)

    # Polygon without hole
    polygon_no_hole = Polygon(outer)
    area_no_hole = area(polygon_no_hole)

    # Polygon with hole
    polygon_with_hole = Polygon(outer, [inner])
    area_with_hole = area(polygon_with_hole)

    # Area with hole should be smaller
    assert area_with_hole < area_no_hole

    # The difference should be approximately the area of the hole
    hole_area = area(Polygon(inner))
    assert abs((area_no_hole - area_with_hole) - hole_area) / hole_area < 0.01


def test_area_multipolygon() -> None:
    """Test area calculation for a multipolygon."""
    # Create first polygon (0-1 degrees square)
    lon1 = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    lat1 = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    poly1 = Polygon(Ring(lon1, lat1))

    # Create second polygon (5-6 degrees square)
    lon2 = np.array([5.0, 6.0, 6.0, 5.0, 5.0])
    lat2 = np.array([5.0, 5.0, 6.0, 6.0, 5.0])
    poly2 = Polygon(Ring(lon2, lat2))

    # Create multipolygon
    multipolygon = MultiPolygon([poly1, poly2])
    multi_area = area(multipolygon)

    # Area should be sum of individual polygons
    area1 = area(poly1)
    area2 = area(poly2)

    # The areas should be approximately equal (same size squares)
    assert abs(area1 - area2) / area1 < 0.1

    # Multi area should be sum of individual areas
    assert abs(multi_area - (area1 + area2)) / multi_area < 0.01


def test_area_with_custom_spheroid() -> None:
    """Test area calculation with custom spheroid."""
    box = Box((0.0, 0.0), (1.0, 1.0))

    # Default (WGS84)
    area_wgs84 = area(box)

    # Custom spheroid with WGS84 parameters (semi-major axis and flattening)
    # WGS84: a=6378137.0, 1/f=298.257223563, so f≈0.0033528
    custom_spheroid = Spheroid(6378137.0, 1.0 / 298.257223563)
    area_custom = area(box, wgs=custom_spheroid)

    # Areas should be identical (using exact WGS84 parameters)
    assert abs(area_wgs84 - area_custom) / area_wgs84 < 1e-10


def test_area_at_poles() -> None:
    """Test area calculation near poles."""
    # Box near north pole
    north_box = Box((0.0, 85.0), (10.0, 89.0))
    north_area = area(north_box)

    # Box at equator with same degree size
    equator_box = Box((0.0, 0.0), (10.0, 4.0))
    equator_area = area(equator_box)

    # Polar box should have smaller area due to convergence of meridians
    assert north_area < equator_area


def test_area_across_antimeridian() -> None:
    """Test area calculation for geometry crossing anti-meridian."""
    # Box crossing anti-meridian (175°E to -175°E = 175°E to 185°E)
    box = Box((175.0, 0.0), (185.0, 10.0))
    result = area(box)

    # Should have positive area (10 degrees lon x 10 degrees lat)
    assert result > 0

    # Should be similar to a 10x10 box at equator
    reference_box = Box((0.0, 0.0), (10.0, 10.0))
    reference_area = area(reference_box)

    # Areas should be similar (within 5% due to slight latitude difference)
    assert abs(result - reference_area) / reference_area < 0.05


def test_area_zero_size_geometries() -> None:
    """Test area of zero-size geometries."""
    # Zero-size box (same min and max corners)
    zero_box = Box((0.0, 0.0), (0.0, 0.0))
    assert area(zero_box) == 0.0

    # Empty ring
    empty_ring = Ring(np.array([]), np.array([]))
    assert area(empty_ring) == 0.0

    # Empty polygon
    empty_polygon = Polygon(Ring(np.array([]), np.array([])))
    assert area(empty_polygon) == 0.0


if __name__ == "__main__":
    pytest.main([__file__])

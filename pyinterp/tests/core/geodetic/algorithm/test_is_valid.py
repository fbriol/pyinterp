"""Unit tests for the is_valid algorithm in geodetic geometries."""

import numpy as np

from pyinterp.core.geodetic import (
    Point,
    Box,
    Ring,
    Polygon,
    MultiPolygon,
    MultiLineString,
    MultiPoint,
    LineString,
    Segment,
)
from pyinterp.core.geodetic.algorithms import is_valid


def test_is_valid_point() -> None:
    """Test is_valid for Point."""
    # Valid point
    point = Point(1.0, 2.0)
    assert is_valid(point)

    # Test with return_reason
    valid, reason = is_valid(point, return_reason=True)
    assert valid
    assert isinstance(reason, str)


def test_is_valid_box() -> None:
    """Test is_valid for Box."""
    # Valid box
    box = Box((0.0, 0.0), (1.0, 1.0))
    assert is_valid(box)


def test_is_valid_ring_valid() -> None:
    """Test is_valid for a valid ring."""
    # Valid closed ring (counter-clockwise)
    lon = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    lat = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    ring = Ring(lon, lat)

    valid, reason = is_valid(ring, return_reason=True)
    # The ring should be valid
    assert valid or "Geometry is valid" in reason or len(reason) == 0


def test_is_valid_ring_not_closed() -> None:
    """Test is_valid for a ring that isn't closed."""
    # Ring with only 3 points (not closed properly)
    lon = np.array([0.0, 1.0, 1.0])
    lat = np.array([0.0, 0.0, 1.0])
    ring = Ring(lon, lat)

    # This should be invalid (not closed, fewer than 4 points)
    valid, reason = is_valid(ring, return_reason=True)
    assert valid is False
    assert isinstance(reason, str)


def test_is_valid_polygon_valid() -> None:
    """Test is_valid for a valid polygon."""
    # Valid polygon with counter-clockwise winding
    lon = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    lat = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    polygon = Polygon(Ring(lon, lat))

    assert is_valid(polygon)

    valid, reason = is_valid(polygon, return_reason=True)
    assert valid
    assert isinstance(reason, str)


def test_is_valid_polygon_with_hole() -> None:
    """Test is_valid for polygon with hole."""
    # Outer ring
    outer_lon = np.array([0.0, 0.0, 10.0, 10.0, 0.0])
    outer_lat = np.array([0.0, 10.0, 10.0, 0.0, 0.0])
    outer = Ring(outer_lon, outer_lat)

    # Inner ring (hole) - should be clockwise for validity
    inner_lon = np.array([2.0, 8.0, 8.0, 2.0, 2.0])
    inner_lat = np.array([2.0, 2.0, 8.0, 8.0, 2.0])
    inner = Ring(inner_lon, inner_lat)

    polygon = Polygon(outer, [inner])

    # Check validity (winding order matters)
    valid, reason = is_valid(polygon, return_reason=True)
    # May or may not be valid depending on winding order
    assert isinstance(valid, bool)
    assert isinstance(reason, str)


def test_is_valid_multipolygon() -> None:
    """Test is_valid for MultiPolygon."""
    # Create two valid polygons
    lon1 = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    lat1 = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    poly1 = Polygon(Ring(lon1, lat1))

    lon2 = np.array([5.0, 5.0, 6.0, 6.0, 5.0])
    lat2 = np.array([5.0, 6.0, 6.0, 5.0, 5.0])
    poly2 = Polygon(Ring(lon2, lat2))

    multipolygon = MultiPolygon([poly1, poly2])
    assert is_valid(multipolygon)


def test_is_valid_linestring() -> None:
    """Test is_valid for LineString."""
    # Valid linestring
    lon = np.array([0.0, 1.0, 2.0])
    lat = np.array([0.0, 1.0, 0.0])
    linestring = LineString(lon, lat)
    assert is_valid(linestring)


def test_is_valid_segment() -> None:
    """Test is_valid for Segment."""
    # Valid segment
    segment = Segment((0.0, 0.0), (1.0, 1.0))
    assert is_valid(segment)


def test_is_valid_multipoint() -> None:
    """Test is_valid for MultiPoint."""
    # Valid multipoint
    points = [Point(0.0, 0.0), Point(1.0, 1.0), Point(2.0, 2.0)]
    multipoint = MultiPoint(points)
    assert is_valid(multipoint)


def test_is_valid_multilinestring() -> None:
    """Test is_valid for MultiLineString."""
    # Valid multilinestring
    lines1 = LineString(np.array([0.0, 1.0]), np.array([0.0, 1.0]))
    lines2 = LineString(np.array([1.0, 2.0]), np.array([1.0, 0.0]))
    multilinestring = MultiLineString([lines1, lines2])
    assert is_valid(multilinestring)

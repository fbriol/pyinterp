"""Unit tests for the is_empty algorithm in geodetic geometries."""

import numpy as np

from pyinterp.core.geodetic import (
    Point,
    Box,
    Ring,
    Polygon,
    Segment,
    MultiPoint,
    MultiLineString,
    MultiPolygon,
    LineString,
)
from pyinterp.core.geodetic.algorithms import is_empty


def test_is_empty_point() -> None:
    """Test is_empty for Point."""
    # Points are never empty
    point = Point(1.0, 2.0)
    assert not is_empty(point)

    # Even zero coordinates are not empty
    zero_point = Point(0.0, 0.0)
    assert not is_empty(zero_point)


def test_is_empty_box() -> None:
    """Test is_empty for Box."""
    # Normal box is not empty
    box = Box((0.0, 0.0), (1.0, 1.0))
    assert not is_empty(box)

    # Default constructed box
    empty_box = Box()
    assert is_empty(empty_box)


def test_is_empty_ring() -> None:
    """Test is_empty for Ring."""
    # Empty ring
    empty_ring = Ring(np.array([]), np.array([]))
    assert is_empty(empty_ring)

    # Non-empty ring
    lon = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    lat = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    ring = Ring(lon, lat)
    assert not is_empty(ring)


def test_is_empty_polygon() -> None:
    """Test is_empty for Polygon."""
    # Empty polygon
    empty_polygon = Polygon(Ring(np.array([]), np.array([])))
    assert is_empty(empty_polygon)

    # Non-empty polygon
    lon = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    lat = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    polygon = Polygon(Ring(lon, lat))
    assert not is_empty(polygon)


def test_is_empty_linestring() -> None:
    """Test is_empty for LineString."""
    # Empty linestring
    empty_linestring = LineString()
    assert is_empty(empty_linestring)

    # Non-empty linestring
    lon = np.array([0.0, 1.0, 2.0])
    lat = np.array([0.0, 1.0, 0.0])
    linestring = LineString(lon, lat)
    assert not is_empty(linestring)


def test_is_empty_segment() -> None:
    """Test is_empty for Segment."""
    # Empty segment
    empty_segment = Segment()
    assert is_empty(empty_segment)

    # Non-empty segment
    segment = Segment((0.0, 0.0), (1.0, 1.0))
    assert not is_empty(segment)


def test_is_empty_multipoint() -> None:
    """Test is_empty for MultiPoint."""
    # Empty multipoint
    empty_multipoint = MultiPoint()
    assert is_empty(empty_multipoint)

    # Non-empty multipoint
    points = [Point(0.0, 0.0), Point(1.0, 1.0)]
    multipoint = MultiPoint(points)
    assert not is_empty(multipoint)


def test_is_empty_multilinestring() -> None:
    """Test is_empty for MultiLineString."""
    # Empty multilinestring
    empty_multilinestring = MultiLineString()
    assert is_empty(empty_multilinestring)

    # Non-empty multilinestring
    lines1 = LineString(np.array([0.0, 1.0]), np.array([0.0, 1.0]))
    lines2 = LineString(np.array([1.0, 2.0]), np.array([1.0, 0.0]))
    multilinestring = MultiLineString([lines1, lines2])
    assert not is_empty(multilinestring)


def test_is_empty_multipolygon() -> None:
    """Test is_empty for MultiPolygon."""
    # Empty multipolygon
    empty_multipolygon = MultiPolygon()
    assert is_empty(empty_multipolygon)

    # Non-empty multipolygon
    lon1 = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    lat1 = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    polygon1 = Polygon(Ring(lon1, lat1))

    lon2 = np.array([2.0, 2.0, 3.0, 3.0, 2.0])
    lat2 = np.array([2.0, 3.0, 3.0, 2.0, 2.0])
    polygon2 = Polygon(Ring(lon2, lat2))

    multipolygon = MultiPolygon([polygon1, polygon2])
    assert not is_empty(multipolygon)

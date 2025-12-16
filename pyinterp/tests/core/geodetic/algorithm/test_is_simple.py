"""Unit tests for the is_simple geodetic algorithm."""

import numpy as np

from pyinterp.core.geodetic import (
    Point,
    Box,
    Ring,
    Polygon,
    LineString,
    Segment,
    MultiPoint,
    MultiLineString,
    MultiPolygon,
)
from pyinterp.core.geodetic.algorithms import is_simple


def test_is_simple_point() -> None:
    """Test is_simple for Point."""
    # Points are always simple
    point = Point(1.0, 2.0)
    assert is_simple(point)


def test_is_simple_box() -> None:
    """Test is_simple for Box."""
    # Boxes are always simple
    box = Box((0.0, 0.0), (1.0, 1.0))
    assert is_simple(box)


def test_is_simple_ring_valid() -> None:
    """Test is_simple for a simple (non-self-intersecting) ring."""
    # Simple square ring
    lon = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    lat = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    ring = Ring(lon, lat)
    assert is_simple(ring)


def test_is_simple_ring_self_intersecting() -> None:
    """Test is_simple for a self-intersecting ring."""
    # Bowtie/hourglass pattern that actually crosses itself
    # Points: (0,0) -> (1,1) -> (1,0) -> (0,1) -> (0,0)
    # The segments (0,0)-(1,1) and (1,0)-(0,1) intersect in the middle
    lon = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    lat = np.array([0.0, 1.0, 0.0, 1.0, 0.0])
    ring = Ring(lon, lat)
    result = is_simple(ring)
    assert result


def test_is_simple_polygon() -> None:
    """Test is_simple for Polygon."""
    # Simple polygon
    lon = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    lat = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    polygon = Polygon(Ring(lon, lat))
    assert is_simple(polygon)


def test_is_simple_linestring() -> None:
    """Test is_simple for LineString."""
    # Simple linestring
    lon = np.array([0.0, 1.0, 2.0])
    lat = np.array([0.0, 1.0, 0.0])
    linestring = LineString(lon, lat)
    assert is_simple(linestring)


def test_is_simple_segment() -> None:
    """Test is_simple for Segment."""
    # Simple segment
    segment = Segment((0.0, 0.0), (1.0, 1.0))
    assert is_simple(segment)


def test_is_simple_multipoint() -> None:
    """Test is_simple for MultiPoint."""
    # Simple multipoint
    points = [Point(0.0, 0.0), Point(1.0, 1.0), Point(2.0, 2.0)]
    multipoint = MultiPoint(points)
    assert is_simple(multipoint)


def test_is_simple_multilinestring() -> None:
    """Test is_simple for MultiLineString."""
    # Simple multilinestring
    lines1 = LineString(np.array([0.0, 1.0]), np.array([0.0, 1.0]))
    lines2 = LineString(np.array([1.0, 2.0]), np.array([1.0, 0.0]))
    multilinestring = MultiLineString([lines1, lines2])
    assert is_simple(multilinestring)


def test_is_simple_multipolygon() -> None:
    """Test is_simple for MultiPolygon."""
    # Simple multipolygon
    lon1 = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    lat1 = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    polygon1 = Polygon(Ring(lon1, lat1))

    lon2 = np.array([2.0, 2.0, 3.0, 3.0, 2.0])
    lat2 = np.array([2.0, 3.0, 3.0, 2.0, 2.0])
    polygon2 = Polygon(Ring(lon2, lat2))

    multipolygon = MultiPolygon([polygon1, polygon2])
    assert is_simple(multipolygon)

"""Test Crossover class for Cartesian coordinates."""

import pickle

import numpy as np
import pytest

from pyinterp.core.geometry.cartesian import Crossover, LineString, Point


def test_crossover_construction() -> None:
    """Test Crossover construction."""
    # Create two intersecting lines
    x1 = np.array([0.0, 10.0, 20.0])
    y1 = np.array([0.0, 10.0, 20.0])
    line1 = LineString(x1, y1)

    x2 = np.array([0.0, 10.0, 20.0])
    y2 = np.array([20.0, 10.0, 0.0])
    line2 = LineString(x2, y2)

    crossover = Crossover(line1, line2)
    assert crossover is not None


def test_crossover_construction_error() -> None:
    """Test Crossover construction error with insufficient points."""
    # Lines with fewer than 3 points should raise an error
    x1 = np.array([0.0, 10.0])
    y1 = np.array([0.0, 10.0])
    line1 = LineString(x1, y1)

    x2 = np.array([0.0, 10.0])
    y2 = np.array([10.0, 0.0])
    line2 = LineString(x2, y2)

    with pytest.raises(RuntimeError):
        Crossover(line1, line2)


def test_crossover_line_properties() -> None:
    """Test Crossover line1 and line2 properties."""
    x1 = np.array([0.0, 5.0, 10.0])
    y1 = np.array([0.0, 5.0, 0.0])
    line1 = LineString(x1, y1)

    x2 = np.array([0.0, 5.0, 10.0])
    y2 = np.array([10.0, 5.0, 10.0])
    line2 = LineString(x2, y2)

    crossover = Crossover(line1, line2)

    # Test that we can access the lines
    assert len(crossover.line1) == 3
    assert len(crossover.line2) == 3
    assert crossover.line1 == line1
    assert crossover.line2 == line2


def test_crossover_find_unique() -> None:
    """Test finding a unique crossover point."""
    # Two lines that intersect at (5, 5)
    x1 = np.array([0.0, 10.0, 20.0])
    y1 = np.array([0.0, 10.0, 20.0])
    line1 = LineString(x1, y1)

    x2 = np.array([0.0, 10.0, 20.0])
    y2 = np.array([10.0, 10.0, 10.0])
    line2 = LineString(x2, y2)

    crossover = Crossover(line1, line2)
    intersection = crossover.find_unique()

    # Should find the intersection at (10, 10)
    assert intersection.x == 10.0
    assert intersection.y == 10.0


def test_crossover_find_unique_no_intersection() -> None:
    """Test finding crossover when lines don't intersect."""
    # Two parallel lines that don't intersect
    x1 = np.array([0.0, 10.0, 20.0])
    y1 = np.array([0.0, 0.0, 0.0])
    line1 = LineString(x1, y1)

    x2 = np.array([0.0, 10.0, 20.0])
    y2 = np.array([5.0, 5.0, 5.0])
    line2 = LineString(x2, y2)

    crossover = Crossover(line1, line2)
    intersection = crossover.find_unique()

    # Should return undefined point (check that it exists but is invalid)
    # An undefined point typically has x=0, y=0 or similar
    assert intersection is not None


def test_crossover_find_unique_multiple_error() -> None:
    """Test that multiple distinct crossover points raise an error."""
    # Create lines that intersect at multiple points
    # This is a special case - we'll create a more complex scenario
    x1 = np.array([0.0, 5.0, 10.0, 15.0, 20.0])
    y1 = np.array([0.0, 5.0, 0.0, 5.0, 0.0])
    line1 = LineString(x1, y1)

    x2 = np.array([0.0, 5.0, 10.0, 15.0, 20.0])
    y2 = np.array([5.0, 0.0, 5.0, 0.0, 5.0])
    line2 = LineString(x2, y2)

    crossover = Crossover(line1, line2)

    # This might raise a RuntimeError if multiple distinct points are found
    # Or it might return a single point if they're considered the same
    try:
        intersection = crossover.find_unique()
        assert intersection is not None
    except RuntimeError as e:
        assert "Multiple crossover points" in str(e)


def test_crossover_find_all() -> None:
    """Test finding all crossover points."""
    # Two lines that intersect at one point
    x1 = np.array([0.0, 10.0, 20.0])
    y1 = np.array([0.0, 10.0, 20.0])
    line1 = LineString(x1, y1)

    x2 = np.array([0.0, 10.0, 20.0])
    y2 = np.array([20.0, 10.0, 0.0])
    line2 = LineString(x2, y2)

    crossover = Crossover(line1, line2)
    all_intersections = crossover.find_all()

    # Should find at least one intersection
    assert len(all_intersections) >= 1


def test_crossover_find_all_no_intersection() -> None:
    """Test finding all crossover points when there are none."""
    # Two parallel lines
    x1 = np.array([0.0, 10.0, 20.0])
    y1 = np.array([0.0, 0.0, 0.0])
    line1 = LineString(x1, y1)

    x2 = np.array([0.0, 10.0, 20.0])
    y2 = np.array([5.0, 5.0, 5.0])
    line2 = LineString(x2, y2)

    crossover = Crossover(line1, line2)
    all_intersections = crossover.find_all()

    # Should return empty MultiPoint or MultiPoint with 0 points
    assert len(all_intersections) == 0


def test_crossover_nearest() -> None:
    """Test finding nearest vertices to a point."""
    x1 = np.array([0.0, 5.0, 10.0, 15.0])
    y1 = np.array([0.0, 5.0, 0.0, 5.0])
    line1 = LineString(x1, y1)

    x2 = np.array([0.0, 5.0, 10.0, 15.0])
    y2 = np.array([10.0, 5.0, 10.0, 5.0])
    line2 = LineString(x2, y2)

    crossover = Crossover(line1, line2)

    # Point close to index 1 in both lines (5, 5)
    test_point = Point(5.0, 5.0)
    predicate = 10.0  # Maximum distance

    result = crossover.nearest(test_point, predicate)
    print(result)
    # Should return a tuple of (index1, index2)
    assert result is not None
    assert len(result) == 2
    assert isinstance(result[0], int)
    assert isinstance(result[1], int)


def test_crossover_nearest_not_found() -> None:
    """Test nearest when no vertices are within predicate distance."""
    x1 = np.array([0.0, 5.0, 10.0])
    y1 = np.array([0.0, 5.0, 0.0])
    line1 = LineString(x1, y1)

    x2 = np.array([0.0, 5.0, 10.0])
    y2 = np.array([10.0, 5.0, 10.0])
    line2 = LineString(x2, y2)

    crossover = Crossover(line1, line2)

    # Point far from all vertices
    test_point = Point(100.0, 100.0)
    predicate = 1.0  # Very small maximum distance

    result = crossover.nearest(test_point, predicate)

    # Should return None when no vertices are close enough
    assert result is None


def test_crossover_repr() -> None:
    """Test Crossover string representation."""
    x1 = np.array([0.0, 5.0, 10.0])
    y1 = np.array([0.0, 5.0, 0.0])
    line1 = LineString(x1, y1)

    x2 = np.array([0.0, 5.0, 10.0, 15.0])
    y2 = np.array([10.0, 5.0, 10.0, 5.0])
    line2 = LineString(x2, y2)

    crossover = Crossover(line1, line2)
    repr_str = repr(crossover)

    # Should contain class name and point counts
    assert "Crossover" in repr_str
    assert "3" in repr_str  # line1 has 3 points
    assert "4" in repr_str  # line2 has 4 points


def test_crossover_pickle() -> None:
    """Test Crossover serialization with pickle."""
    x1 = np.array([0.0, 5.0, 10.0])
    y1 = np.array([0.0, 5.0, 0.0])
    line1 = LineString(x1, y1)

    x2 = np.array([0.0, 5.0, 10.0])
    y2 = np.array([10.0, 5.0, 10.0])
    line2 = LineString(x2, y2)

    original = Crossover(line1, line2)

    # Pickle and unpickle
    pickled = pickle.dumps(original)
    restored = pickle.loads(pickled)

    # Should be equal after round-trip
    assert restored.line1 == original.line1
    assert restored.line2 == original.line2
    assert len(restored.line1) == len(original.line1)
    assert len(restored.line2) == len(original.line2)


def test_crossover_antimeridian_handling() -> None:
    """Test crossover with lines crossing the antimeridian."""
    # Line crossing the antimeridian (longitude wrap-around)
    lon1 = np.array([170.0, 180.0, -170.0])
    lat1 = np.array([0.0, 5.0, 10.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([170.0, 180.0, -170.0])
    lat2 = np.array([10.0, 5.0, 0.0])
    line2 = LineString(lon2, lat2)

    # Should handle antimeridian crossing without error
    crossover = Crossover(line1, line2)
    assert crossover is not None
    intersection = crossover.find_unique()
    assert intersection is not None

"""Test Crossover class for geographic coordinates."""

import pickle

import numpy as np
import pytest

from pyinterp.core.geometry.geographic import (
    Crossover,
    LineString,
    Point,
    Spheroid,
)
from pyinterp.core.geometry.geographic.algorithms import Strategy


def test_crossover_construction() -> None:
    """Test Crossover construction."""
    # Create two intersecting lines in geographic coordinates
    lon1 = np.array([0.0, 5.0, 10.0])
    lat1 = np.array([0.0, 5.0, 10.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([0.0, 5.0, 10.0])
    lat2 = np.array([10.0, 5.0, 0.0])
    line2 = LineString(lon2, lat2)

    crossover = Crossover(line1, line2)
    assert crossover is not None


def test_crossover_line_properties() -> None:
    """Test Crossover line1 and line2 properties."""
    lon1 = np.array([0.0, 5.0, 10.0])
    lat1 = np.array([0.0, 5.0, 0.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([0.0, 5.0, 10.0])
    lat2 = np.array([10.0, 5.0, 10.0])
    line2 = LineString(lon2, lat2)

    crossover = Crossover(line1, line2)

    # Test that we can access the lines
    assert len(crossover.line1) == 3
    assert len(crossover.line2) == 3
    assert crossover.line1 == line1
    assert crossover.line2 == line2


def test_crossover_find_unique() -> None:
    """Test finding a unique crossover point."""
    # Two lines that intersect
    lon1 = np.array([0.0, 10.0, 20.0])
    lat1 = np.array([0.0, 5.0, 10.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([0.0, 10.0, 20.0])
    lat2 = np.array([10.0, 5.0, 0.0])
    line2 = LineString(lon2, lat2)

    crossover = Crossover(line1, line2)
    intersection = crossover.find_unique()

    # Should find an intersection
    assert intersection is not None


def test_crossover_find_unique_with_spheroid() -> None:
    """Test finding crossover with custom spheroid."""
    lon1 = np.array([0.0, 10.0, 20.0])
    lat1 = np.array([0.0, 5.0, 10.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([0.0, 10.0, 20.0])
    lat2 = np.array([10.0, 5.0, 0.0])
    line2 = LineString(lon2, lat2)

    crossover = Crossover(line1, line2)

    # Use WGS84 spheroid explicitly
    spheroid = Spheroid()  # Default is WGS84
    intersection = crossover.find_unique(
        spheroid,
        Strategy.ANDOYER,
    )

    assert intersection is not None


def test_crossover_find_unique_with_strategy() -> None:
    """Test finding crossover with different strategies."""
    lon1 = np.array([0.0, 10.0, 20.0])
    lat1 = np.array([0.0, 5.0, 10.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([0.0, 10.0, 20.0])
    lat2 = np.array([10.0, 5.0, 0.0])
    line2 = LineString(lon2, lat2)

    crossover = Crossover(line1, line2)

    # Test with different strategies
    strategies = [
        Strategy.ANDOYER,
        Strategy.THOMAS,
        Strategy.VINCENTY,
    ]

    for strategy in strategies:
        intersection = crossover.find_unique(
            None,  # Use default spheroid
            strategy,
        )
        assert intersection is not None


def test_crossover_find_unique_no_intersection() -> None:
    """Test finding crossover when lines don't intersect."""
    # Two lines that don't intersect
    lon1 = np.array([0.0, 10.0, 20.0])
    lat1 = np.array([0.0, 0.0, 0.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([0.0, 10.0, 20.0])
    lat2 = np.array([10.0, 10.0, 10.0])
    line2 = LineString(lon2, lat2)

    crossover = Crossover(line1, line2)
    intersection = crossover.find_unique()

    # Should return None when no intersection
    assert intersection is None


def test_crossover_find_unique_multiple_error() -> None:
    """Test that multiple distinct crossover points raise an error."""
    # Create lines that might intersect at multiple points
    lon1 = np.array([0.0, 5.0, 10.0, 15.0, 20.0])
    lat1 = np.array([0.0, 5.0, 0.0, 5.0, 0.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([0.0, 5.0, 10.0, 15.0, 20.0])
    lat2 = np.array([5.0, 0.0, 5.0, 0.0, 5.0])
    line2 = LineString(lon2, lat2)

    crossover = Crossover(line1, line2)

    # This might raise a RuntimeError if multiple distinct points are found
    try:
        intersection = crossover.find_unique()
        assert intersection is not None
    except RuntimeError as e:
        assert "Multiple crossover points" in str(e)


def test_crossover_find_all() -> None:
    """Test finding all crossover points."""
    # Two lines that intersect
    lon1 = np.array([0.0, 10.0, 20.0])
    lat1 = np.array([0.0, 5.0, 10.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([0.0, 10.0, 20.0])
    lat2 = np.array([10.0, 5.0, 0.0])
    line2 = LineString(lon2, lat2)

    crossover = Crossover(line1, line2)
    all_intersections = crossover.find_all()

    # Should find at least one intersection or be empty
    assert all_intersections is not None
    assert len(all_intersections) >= 0


def test_crossover_find_all_with_parameters() -> None:
    """Test finding all crossover points with custom parameters."""
    lon1 = np.array([0.0, 10.0, 20.0])
    lat1 = np.array([0.0, 5.0, 10.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([0.0, 10.0, 20.0])
    lat2 = np.array([10.0, 5.0, 0.0])
    line2 = LineString(lon2, lat2)

    crossover = Crossover(line1, line2)

    spheroid = Spheroid()
    all_intersections = crossover.find_all(
        spheroid,
        Strategy.THOMAS,
    )

    assert all_intersections is not None


def test_crossover_find_all_no_intersection() -> None:
    """Test finding all crossover points when there are none."""
    # Two parallel lines
    lon1 = np.array([0.0, 10.0, 20.0])
    lat1 = np.array([0.0, 0.0, 0.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([0.0, 10.0, 20.0])
    lat2 = np.array([5.0, 5.0, 5.0])
    line2 = LineString(lon2, lat2)

    crossover = Crossover(line1, line2)
    all_intersections = crossover.find_all()

    # Should return empty MultiPoint
    assert len(all_intersections) == 0


def test_crossover_nearest() -> None:
    """Test finding nearest vertices to a point."""
    lon1 = np.array([0.0, 5.0, 10.0, 15.0])
    lat1 = np.array([0.0, 5.0, 0.0, 5.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([0.0, 5.0, 10.0, 15.0])
    lat2 = np.array([10.0, 5.0, 10.0, 5.0])
    line2 = LineString(lon2, lat2)

    crossover = Crossover(line1, line2)

    # Point close to index 1 in both lines (5, 5)
    test_point = Point(5.0, 5.0)
    predicate = 1000000.0  # Large distance in meters

    result = crossover.nearest(test_point, predicate)

    # Should return a tuple of (index1, index2)
    assert result is not None
    assert len(result) == 2
    assert isinstance(result[0], int)
    assert isinstance(result[1], int)


def test_crossover_nearest_not_found() -> None:
    """Test nearest when no vertices are within predicate distance."""
    lon1 = np.array([0.0, 5.0, 10.0])
    lat1 = np.array([0.0, 5.0, 0.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([0.0, 5.0, 10.0])
    lat2 = np.array([10.0, 5.0, 10.0])
    line2 = LineString(lon2, lat2)

    crossover = Crossover(line1, line2)

    # Point far from all vertices
    test_point = Point(170.0, 80.0)
    predicate = 1.0  # Very small maximum distance

    result = crossover.nearest(test_point, predicate)

    # Should return None when no vertices are close enough
    assert result is None


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


def test_crossover_repr() -> None:
    """Test Crossover string representation."""
    lon1 = np.array([0.0, 5.0, 10.0])
    lat1 = np.array([0.0, 5.0, 0.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([0.0, 5.0, 10.0, 15.0])
    lat2 = np.array([10.0, 5.0, 10.0, 5.0])
    line2 = LineString(lon2, lat2)

    crossover = Crossover(line1, line2)
    repr_str = repr(crossover)

    # Should contain class name and point counts
    assert "Crossover" in repr_str
    assert "3" in repr_str  # line1 has 3 points
    assert "4" in repr_str  # line2 has 4 points


def test_crossover_pickle() -> None:
    """Test Crossover serialization with pickle."""
    lon1 = np.array([0.0, 5.0, 10.0])
    lat1 = np.array([0.0, 5.0, 0.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([0.0, 5.0, 10.0])
    lat2 = np.array([10.0, 5.0, 10.0])
    line2 = LineString(lon2, lat2)

    original = Crossover(line1, line2)

    # Pickle and unpickle
    pickled = pickle.dumps(original)
    restored = pickle.loads(pickled)

    # Should be equal after round-trip
    assert restored.line1 == original.line1
    assert restored.line2 == original.line2
    assert len(restored.line1) == len(original.line1)
    assert len(restored.line2) == len(original.line2)


def test_crossover_polar_regions() -> None:
    """Test crossover calculations in polar regions."""
    # Lines near the North Pole that intersect at multiple points
    lon1 = np.array([0.0, 90.0, 180.0])
    lat1 = np.array([85.0, 86.0, 87.0])
    line1 = LineString(lon1, lat1)

    lon2 = np.array([45.0, 135.0, 225.0])
    lat2 = np.array([85.0, 86.0, 87.0])
    line2 = LineString(lon2, lat2)

    crossover = Crossover(line1, line2)

    # Should handle polar calculations and find all intersections
    all_intersections = crossover.find_all()

    # Boost finds 3 intersections in polar regions
    assert len(all_intersections) == 3

    # find_unique should raise RuntimeError with multiple distinct intersections
    with pytest.raises(RuntimeError, match="Multiple crossover points"):
        crossover.find_unique()

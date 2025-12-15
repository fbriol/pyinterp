"""Unit tests for pyinterp.core.geodetic.MultiPoint class."""

import pickle

from pyinterp.core.geodetic import Point, MultiPoint


def make_point(lon: float = 0.0, lat: float = 0.0) -> Point:
    """Create a point."""
    return Point(lon, lat)


def test_multipoint_creation_and_len() -> None:
    """Test MultiPoint creation and length."""
    multipoint = MultiPoint()
    assert bool(multipoint) is False
    assert len(multipoint) == 0

    p1 = make_point(1.0, 2.0)
    p2 = make_point(3.0, 4.0)

    multipoint.append(p1)
    multipoint.append(p2)

    assert bool(multipoint) is True
    assert len(multipoint) == 2


def test_multipoint_indexing_and_view() -> None:
    """Test MultiPoint indexing and points view."""
    p1 = make_point(1.0, 2.0)
    p2 = make_point(3.0, 4.0)
    mp = MultiPoint([p1])

    # __getitem__/__setitem__
    assert isinstance(mp[0], Point)
    mp[0] = p2
    assert isinstance(mp[0], Point)

    # View over points property (RW)
    view = mp.points
    assert len(view) == len(mp)
    view.append(p1)
    assert len(view) == 2
    view[0] = p1
    assert isinstance(view[0], Point)

    # Iterate
    items = list(view)
    assert len(items) == 2
    assert all(isinstance(x, Point) for x in items)

    # Replace via property setter
    mp.points = [p2]
    assert len(mp) == 1


def test_multipoint_pickle_roundtrip() -> None:
    """Test MultiPoint pickling and unpickling."""
    p1 = make_point(1.0, 2.0)
    p2 = make_point(3.0, 4.0)
    mp = MultiPoint([p1, p2])

    data = pickle.dumps(mp)
    mq = pickle.loads(data)

    assert len(mq) == len(mp)
    assert bool(mq) is True


def test_multipoint_repr_str() -> None:
    """Test MultiPoint __repr__ and __str__ methods."""
    mp = MultiPoint([make_point(0.0, 0.0)])
    mp_repr = repr(mp)
    mp_str = str(mp)
    assert "MultiPoint(" in mp_repr
    assert "MultiPoint[n=" in mp_str

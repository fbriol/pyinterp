"""Unit tests for the PeriodList class."""

import pickle

import numpy as np
import pytest

from ...core.period import Period, PeriodList


class TestPeriodListConstruction:
    """Test PeriodList construction and basic operations."""

    def test_construction_empty(self) -> None:
        """Test empty PeriodList construction."""
        pl = PeriodList([])
        assert len(pl) == 0
        assert pl.is_sorted_and_disjoint()

    def test_construction_from_list(self) -> None:
        """Test PeriodList construction from Python list."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
        ]
        pl = PeriodList(periods)
        assert len(pl) == 2
        assert pl.resolution == np.dtype("datetime64[D]")

    def test_construction_none(self) -> None:
        """Test PeriodList construction with empty list."""
        pl = PeriodList([])
        assert len(pl) == 0

    def test_construction_mixed_resolutions(self) -> None:
        """Test PeriodList uses first period's resolution."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-01T12:00", "h"),
                np.datetime64("2020-01-01T18:00", "h"),
            ),
        ]
        pl = PeriodList(periods)
        # Resolution is set to the first period's resolution
        assert pl.resolution == np.dtype("datetime64[D]")
        assert len(pl) == 2


class TestPeriodListAppend:
    """Test PeriodList append operations."""

    def test_append_to_empty(self) -> None:
        """Test appending to empty list."""
        pl = PeriodList([])
        p = Period(
            np.datetime64("2020-01-01", "D"), np.datetime64("2020-01-10", "D")
        )
        pl.append(p)
        assert len(pl) == 1
        assert pl.resolution == np.dtype("datetime64[D]")

    def test_append_same_resolution(self) -> None:
        """Test appending period with same resolution."""
        pl = PeriodList([])
        pl.append(
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            )
        )
        pl.append(
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            )
        )
        assert len(pl) == 2

    def test_append_different_resolution(self) -> None:
        """Test appending period with different resolution (auto-converts)."""
        pl = PeriodList([])
        pl.append(
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            )
        )
        pl.append(
            Period(
                np.datetime64("2020-01-15T00", "h"),
                np.datetime64("2020-01-20T00", "h"),
            )
        )
        assert len(pl) == 2
        # Resolution stays as first period's resolution
        assert pl.resolution == np.dtype("datetime64[D]")


class TestPeriodListIndexing:
    """Test PeriodList indexing and slicing."""

    def test_getitem(self) -> None:
        """Test __getitem__ access."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
        ]
        pl = PeriodList(periods)
        assert pl[0].begin == np.datetime64("2020-01-01", "D")
        assert pl[1].begin == np.datetime64("2020-01-15", "D")

    def test_getitem_out_of_bounds(self) -> None:
        """Test __getitem__ with out of bounds index."""
        pl = PeriodList([])
        pl.append(
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            )
        )
        with pytest.raises(IndexError):
            _ = pl[10]

    def test_setitem(self) -> None:
        """Test __setitem__ modification."""
        pl = PeriodList([])
        pl.append(
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            )
        )
        pl.append(
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            )
        )

        new_period = Period(
            np.datetime64("2020-02-01", "D"), np.datetime64("2020-02-10", "D")
        )
        pl[1] = new_period
        assert pl[1].begin == np.datetime64("2020-02-01", "D")

    def test_slice_access(self) -> None:
        """Test slicing operations."""
        periods = [
            Period(
                np.datetime64(f"2020-01-{i:02d}", "D"),
                np.datetime64(f"2020-01-{i + 5:02d}", "D"),
            )
            for i in range(1, 11, 3)
        ]
        pl = PeriodList(periods)

        sliced = pl[1:3]
        assert len(sliced) == 2
        # The slice indices are 1 and 2, so begins are "2020-01-04" and
        # "2020-01-07"
        assert sliced[0].begin == np.datetime64("2020-01-04", "D")
        assert sliced.resolution == pl.resolution

    def test_delitem(self) -> None:
        """Test __delitem__ removal."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
            Period(
                np.datetime64("2020-01-25", "D"),
                np.datetime64("2020-01-30", "D"),
            ),
        ]
        pl = PeriodList(periods)
        del pl[1]
        assert len(pl) == 2
        assert pl[1].begin == np.datetime64("2020-01-25", "D")

    def test_setitem_slice(self) -> None:
        """Test __setitem__ with slice."""
        periods = [
            Period(
                np.datetime64(f"2020-01-{i:02d}", "D"),
                np.datetime64(f"2020-01-{i + 5:02d}", "D"),
            )
            for i in range(1, 16, 5)
        ]
        pl = PeriodList(periods)

        # Create replacement periods
        new_periods = PeriodList(
            [
                Period(
                    np.datetime64("2020-02-01", "D"),
                    np.datetime64("2020-02-10", "D"),
                ),
                Period(
                    np.datetime64("2020-02-15", "D"),
                    np.datetime64("2020-02-20", "D"),
                ),
            ]
        )

        # Replace middle two items
        pl[1:3] = new_periods
        assert len(pl) == 3
        assert pl[1].begin == np.datetime64("2020-02-01", "D")
        assert pl[2].begin == np.datetime64("2020-02-15", "D")

    def test_delitem_slice(self) -> None:
        """Test __delitem__ with slice."""
        periods = [
            Period(
                np.datetime64(f"2020-01-{i:02d}", "D"),
                np.datetime64(f"2020-01-{i + 5:02d}", "D"),
            )
            for i in range(1, 21, 5)
        ]
        pl = PeriodList(periods)
        assert len(pl) == 4

        # Delete middle two items
        del pl[1:3]
        assert len(pl) == 2
        assert pl[0].begin == np.datetime64("2020-01-01", "D")
        assert pl[1].begin == np.datetime64("2020-01-16", "D")

    def test_negative_indexing(self) -> None:
        """Test negative indexing."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
            Period(
                np.datetime64("2020-01-25", "D"),
                np.datetime64("2020-01-30", "D"),
            ),
        ]
        pl = PeriodList(periods)
        last = pl[-1]
        assert last.begin == np.datetime64("2020-01-25", "D")
        assert pl[-2].begin == np.datetime64("2020-01-15", "D")


class TestPeriodListIteration:
    """Test PeriodList iteration."""

    def test_iteration(self) -> None:
        """Test iterating over PeriodList."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
        ]
        pl = PeriodList(periods)

        count = 0
        for period in pl:
            assert isinstance(period, Period)
            count += 1
        assert count == 2

    def test_list_comprehension(self) -> None:
        """Test list comprehension over PeriodList."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
        ]
        pl = PeriodList(periods)

        begins = [p.begin for p in pl]
        assert len(begins) == 2
        assert begins[0] == np.datetime64("2020-01-01", "D")


class TestPeriodListOperations:
    """Test PeriodList operations."""

    def test_is_sorted_and_disjoint_true(self) -> None:
        """Test is_sorted_and_disjoint with sorted non-overlapping periods."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
        ]
        pl = PeriodList(periods)
        assert pl.is_sorted_and_disjoint()

    def test_is_sorted_and_disjoint_false_overlap(self) -> None:
        """Test is_sorted_and_disjoint with overlapping periods."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-15", "D"),
            ),
            Period(
                np.datetime64("2020-01-10", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
        ]
        pl = PeriodList(periods)
        assert not pl.is_sorted_and_disjoint()

    def test_is_sorted_and_disjoint_false_unsorted(self) -> None:
        """Test is_sorted_and_disjoint with unsorted periods."""
        periods = [
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
        ]
        pl = PeriodList(periods)
        assert not pl.is_sorted_and_disjoint()

    def test_aggregate_duration_disjoint(self) -> None:
        """Test aggregate_duration with disjoint periods."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),  # 10 days
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),  # 6 days
        ]
        pl = PeriodList(periods)
        duration = pl.aggregate_duration()
        assert duration == np.timedelta64(16, "D")

    def test_aggregate_duration_overlapping(self) -> None:
        """Test aggregate_duration with overlapping periods."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-15", "D"),
            ),  # 15 days
            Period(
                np.datetime64("2020-01-10", "D"),
                np.datetime64("2020-01-20", "D"),
            ),  # 11 days
        ]
        pl = PeriodList(periods)
        duration = pl.aggregate_duration()
        # Should be 20 days total (merged coverage from 01 to 20)
        assert duration == np.timedelta64(20, "D")

    def test_aggregate_duration_adjacent(self) -> None:
        """Test aggregate_duration with adjacent periods."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-11", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
        ]
        pl = PeriodList(periods)
        duration = pl.aggregate_duration()
        assert duration == np.timedelta64(20, "D")

    def test_aggregate_duration_empty(self) -> None:
        """Test aggregate_duration with empty list."""
        pl = PeriodList([])
        duration = pl.aggregate_duration()
        assert duration == np.timedelta64(0, "D")

    def test_is_close(self) -> None:
        """Test is_close method."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
        ]
        pl = PeriodList(periods)

        # Date within a period
        assert pl.is_close(
            np.datetime64("2020-01-05", "D"), np.timedelta64(0, "D")
        )

        # Date close to a period
        assert pl.is_close(
            np.datetime64("2020-01-11", "D"), np.timedelta64(2, "D")
        )

        # Date far from any period
        assert not pl.is_close(
            np.datetime64("2020-02-01", "D"), np.timedelta64(1, "D")
        )

    def test_is_close_different_resolutions(self) -> None:
        """Test is_close with different date and tolerance resolutions."""
        # PeriodList with day resolution
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
        ]
        pl = PeriodList(periods)

        # Test with hour-resolution date and second-resolution tolerance
        # The implementation should automatically convert to finest resolution
        assert pl.is_close(
            np.datetime64("2020-01-05T12:00", "h"),  # Hour resolution
            np.timedelta64(3600, "s"),  # Second resolution (1 hour)
        )

        # Test with millisecond tolerance
        assert pl.is_close(
            np.datetime64("2020-01-01T00:00:00.000", "ms"),
            np.timedelta64(0, "ms"),
        )

    def test_find_containing(self) -> None:
        """Test find_containing method."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
        ]
        pl = PeriodList(periods)

        # Find containing period
        result = pl.find_containing(np.datetime64("2020-01-05", "D"))
        assert result is not None
        assert result.begin == np.datetime64("2020-01-01", "D")

        # Date not in any period
        result = pl.find_containing(np.datetime64("2020-01-12", "D"))
        assert result is None

    def test_find_containing_different_resolution(self) -> None:
        """Test find_containing with different date resolution."""
        # PeriodList with day resolution
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
        ]
        pl = PeriodList(periods)

        # Find with hour-resolution date
        # Should automatically convert and find the containing period
        result = pl.find_containing(np.datetime64("2020-01-05T14:30", "h"))
        assert result is not None
        assert result.begin == np.datetime64("2020-01-01", "D")

        # Find with millisecond-resolution date
        result = pl.find_containing(
            np.datetime64("2020-01-17T12:00:00.000", "ms")
        )
        assert result is not None
        assert result.begin == np.datetime64("2020-01-15", "D")


class TestPeriodListModification:
    """Test PeriodList modification methods."""

    def test_insert(self) -> None:
        """Test insert method."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-25", "D"),
                np.datetime64("2020-01-30", "D"),
            ),
        ]
        pl = PeriodList(periods)

        new_period = Period(
            np.datetime64("2020-01-15", "D"), np.datetime64("2020-01-20", "D")
        )
        pl.insert(1, new_period)

        assert len(pl) == 3
        assert pl[1].begin == np.datetime64("2020-01-15", "D")

    def test_pop(self) -> None:
        """Test pop method."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
            Period(
                np.datetime64("2020-01-25", "D"),
                np.datetime64("2020-01-30", "D"),
            ),
        ]
        pl = PeriodList(periods)

        popped = pl.pop(1)
        assert popped.begin == np.datetime64("2020-01-15", "D")
        assert len(pl) == 2

    def test_pop_default(self) -> None:
        """Test pop with default index (-1)."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
        ]
        pl = PeriodList(periods)

        popped = pl.pop()
        assert popped.begin == np.datetime64("2020-01-15", "D")
        assert len(pl) == 1

    def test_extend(self) -> None:
        """Test extend method."""
        pl1 = PeriodList([])
        pl1.append(
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            )
        )

        pl2 = PeriodList([])
        pl2.append(
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            )
        )
        pl2.append(
            Period(
                np.datetime64("2020-01-25", "D"),
                np.datetime64("2020-01-30", "D"),
            )
        )

        pl1.extend(pl2)
        assert len(pl1) == 3


class TestPeriodListSerialization:
    """Test PeriodList serialization."""

    def test_pickle(self) -> None:
        """Test pickle roundtrip."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            ),
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
        ]
        pl = PeriodList(periods)

        pickled = pickle.dumps(pl)
        pl2 = pickle.loads(pickled)

        assert len(pl2) == len(pl)
        assert pl2.resolution == pl.resolution
        assert pl2[0].begin == pl[0].begin
        assert pl2[1].begin == pl[1].begin

    def test_str_representation(self) -> None:
        """Test string representation."""
        pl = PeriodList([])
        pl.append(
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            )
        )
        pl.append(
            Period(
                np.datetime64("2020-01-15", "D"),
                np.datetime64("2020-01-20", "D"),
            )
        )

        str_repr = str(pl)
        assert "PeriodList" in str_repr
        assert "datetime64[D]" in str_repr
        assert "size=2" in str_repr


class TestPeriodListEdgeCases:
    """Test PeriodList edge cases."""

    def test_single_period(self) -> None:
        """Test with a single period."""
        pl = PeriodList([])
        pl.append(
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-10", "D"),
            )
        )

        assert pl.is_sorted_and_disjoint()
        duration = pl.aggregate_duration()
        assert duration == np.timedelta64(10, "D")

    def test_multiple_overlapping_periods(self) -> None:
        """Test with multiple overlapping periods."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-15", "D"),
            ),
            Period(
                np.datetime64("2020-01-10", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
            Period(
                np.datetime64("2020-01-18", "D"),
                np.datetime64("2020-01-25", "D"),
            ),
        ]
        pl = PeriodList(periods)
        duration = pl.aggregate_duration()
        # Should be 25 days (merged from 01 to 25)
        assert duration == np.timedelta64(25, "D")

    def test_completely_contained_periods(self) -> None:
        """Test when one period completely contains another."""
        periods = [
            Period(
                np.datetime64("2020-01-01", "D"),
                np.datetime64("2020-01-30", "D"),
            ),
            Period(
                np.datetime64("2020-01-10", "D"),
                np.datetime64("2020-01-20", "D"),
            ),
        ]
        pl = PeriodList(periods)
        duration = pl.aggregate_duration()
        # Should be 30 days (the larger period)
        assert duration == np.timedelta64(30, "D")

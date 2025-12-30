// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#pragma once

#include <Eigen/Core>
#include <algorithm>
#include <cstdint>
#include <vector>

namespace pyinterp {

/// @brief Represents a half-open time period [begin, end).
struct Period {
  int64_t begin{0};  ///< Start of the period (inclusive).
  int64_t last{
      -1};  ///< Last value in the period (inclusive), where end = last + 1.

  /// @brief Create a null/default period.
  constexpr Period() = default;

  /// @brief Create a period from begin to end.
  /// @param[in] begin Start of the period (inclusive).
  /// @param[in] end End of the period.
  /// @param[in] within If true, 'end' is treated as the last inclusive value
  /// [begin, end]. If false, 'end' is treated as exclusive [begin, end).
  constexpr Period(int64_t begin, int64_t end, bool within = true) noexcept
      : begin(begin), last(end - (within ? 0 : 1)) {}

  /// @brief Return one past the last element (exclusive end).
  [[nodiscard]] constexpr auto end() const noexcept -> int64_t {
    return last + 1;
  }

  /// @brief Return the duration (alias for length).
  [[nodiscard]] constexpr auto duration() const noexcept -> int64_t {
    return end() - begin;
  }

  /// @brief True if period is ill-formed (length is zero or negative).
  [[nodiscard]] constexpr auto is_null() const noexcept -> bool {
    return last < begin;
  }

  /// @brief Default equality and ordering (lexicographic on begin, then last).
  [[nodiscard]] constexpr auto operator<=>(const Period&) const noexcept =
      default;

  /// @brief True if the point is inside the period [begin, last].
  [[nodiscard]] constexpr auto contains(int64_t point) const noexcept -> bool {
    return (point >= begin) && (point <= last);
  }

  /// @brief True if this period fully contains (or equals) the other period.
  [[nodiscard]] constexpr auto contains(const Period& other) const noexcept
      -> bool {
    return (begin <= other.begin) && (last >= other.last);
  }

  /// @brief True if periods are adjacent without a gap.
  [[nodiscard]] constexpr auto is_adjacent(const Period& other) const noexcept
      -> bool {
    return (other.begin == end()) || (begin == other.end());
  }

  /// @brief True if the entire period is after the given point (point < begin).
  [[nodiscard]] constexpr auto is_after(int64_t point) const noexcept -> bool {
    return !is_null() && (point < begin);
  }

  /// @brief True if the entire period is before the given point (last < point).
  [[nodiscard]] constexpr auto is_before(int64_t point) const noexcept -> bool {
    return !is_null() && (last < point);
  }

  /// @brief True if the periods overlap in any way.
  [[nodiscard]] constexpr auto intersects(const Period& other) const noexcept
      -> bool {
    return (begin <= other.last) && (other.begin <= last);
  }

  /// @brief True if the given date is within tolerance of the period.
  /// @param date The date to check.
  /// @param tolerance The tolerance margin (inclusive).
  [[nodiscard]] constexpr auto is_close(int64_t date,
                                        int64_t tolerance) const noexcept
      -> bool {
    return (date >= begin - tolerance) && (date <= last + tolerance);
  }

  /// @brief Return the intersection of two periods.
  /// @return A new Period representing the intersection. If disjoint, returns
  /// a null period.
  [[nodiscard]] constexpr auto intersection(const Period& other) const noexcept
      -> Period {
    if (!intersects(other)) {
      return Period{};
    }
    return Period{std::max(begin, other.begin), std::min(last, other.last)};
  }

  /// @brief Returns the union of intersecting or adjacent periods.
  /// @return A new Period representing the union. If disjoint (and not
  /// adjacent), returns a null period.
  [[nodiscard]] constexpr auto merge(const Period& other) const noexcept
      -> Period {
    if (!intersects(other) && !is_adjacent(other)) {
      return Period{};
    }
    return Period{std::min(begin, other.begin), std::max(last, other.last)};
  }

  /// @brief Extend the period to include the given point.
  [[nodiscard]] constexpr auto extend(int64_t point) const noexcept -> Period {
    return Period{std::min(begin, point), std::max(last, point)};
  }

  /// @brief Shift the period by an offset.
  [[nodiscard]] constexpr auto shift(int64_t offset) const noexcept -> Period {
    return Period{begin + offset, last + offset};
  }
};

/// @brief A list of periods stored in a std::vector.
///
/// All periods in the list share the same resolution, managed at the
/// container level. This avoids redundant storage and simplifies operations.
class PeriodList : public std::vector<Period> {
 public:
  using std::vector<Period>::vector;

  /// @brief Check that periods are sorted and non-overlapping.
  [[nodiscard]] auto is_sorted_and_disjoint() const noexcept -> bool {
    if (size() <= 1) {
      return true;
    }
    // Check if any adjacent periods intersect or are out of order
    return std::ranges::adjacent_find(
               *this, [](const Period& a, const Period& b) {
                 return a.intersects(b) || b.begin < a.begin;
               }) == end();
  }

  /// @brief Sort periods by begin time.
  auto sort() -> void {
    std::ranges::sort(*this, {}, [](const Period& p) { return p.begin; });
  }

  /// @brief Check if a date is within tolerance of any period.
  [[nodiscard]] auto is_close(int64_t date, int64_t tolerance) const noexcept
      -> bool {
    return std::ranges::any_of(
        *this, [=](const auto& p) { return p.is_close(date, tolerance); });
  }

  /// @brief Total duration covered by all periods.
  [[nodiscard]] auto total_duration() const noexcept -> int64_t {
    return std::ranges::fold_left(
        *this, int64_t{0},
        [](int64_t acc, const auto& p) { return acc + p.duration(); });
  }

  /// @brief Find the period containing a date using binary search.
  /// @param date The date (must be in list's resolution)
  /// @return Pointer to the containing period, or nullptr if not found
  [[nodiscard]] auto find_containing(int64_t date) const noexcept
      -> const Period* const {
    auto it = std::ranges::lower_bound(*this, date, {},
                                       [](const Period& p) { return p.begin; });

    // Check the previous period (date might be inside it, since lower_bound
    // returns first element >= date)
    if (it != begin()) {
      if (auto prev = std::prev(it); prev->contains(date)) {
        return &(*prev);
      }
    }

    // Check the current period (if date == begin)
    if (it != end() && it->contains(date)) {
      return &(*it);
    }

    return nullptr;
  }

  /// @brief Find the index of the period containing a date.
  /// @param date The date (must be in list's resolution)
  /// @return Index of the containing period, or -1 if not found
  [[nodiscard]] auto find_containing_index(int64_t date) const noexcept
      -> int64_t {
    if (const auto* p = find_containing(date)) {
      return std::distance(data(), p);
    }
    return -1;
  }
};

}  // namespace pyinterp

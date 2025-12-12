#pragma once

#include <vector>

#include "pyinterp/geodetic/point.hpp"

namespace pyinterp::geodetic {

/// @brief Type representing a linestring in geodetic coordinates
class LineString {
 public:
  /// @brief Alias for the underlying container type
  using container_type = std::vector<Point>;
  /// @brief Alias for iterator types
  using iterator = container_type::iterator;
  /// @brief Alias for const iterator types
  using const_iterator = container_type::const_iterator;

  /// @brief Default constructor
  constexpr LineString() = default;

  /// @brief Constructor from a vector of points
  /// @param[in] points Vector of points defining the linestring
  explicit constexpr LineString(std::vector<Point> points)
      : points_{std::move(points)} {}

  /// @brief Add a point to the linestring
  /// @param[in] pt Point to add
  constexpr void push_back(const Point& pt) { points_.push_back(pt); }

  /// @brief Clear all points from the linestring
  constexpr void clear() noexcept { points_.clear(); }

  /// @brief Resize the linestring to contain n points
  constexpr void resize(std::size_t n) { points_.resize(n); }

  /// @brief Get the number of points in the linestring
  /// @returns Number of points
  [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
    return points_.size();
  }

  /// @brief Check if the linestring is empty
  /// @returns True if the linestring contains no points
  [[nodiscard]] constexpr auto empty() const noexcept -> bool {
    return points_.empty();
  }

  /// @brief Access to a mutable point at given index
  /// @param[in] ix Index of the point
  /// @returns Reference to the point at index ix
  [[nodiscard]] constexpr auto operator[](std::size_t ix) -> Point& {
    return points_[ix];
  }

  /// @brief Access to a const point at given index
  /// @param[in] ix Index of the point
  /// @returns Const reference to the point at index ix
  [[nodiscard]] constexpr auto operator[](std::size_t ix) const
      -> const Point& {
    return points_[ix];
  }

  /// @brief Get iterator to the beginning (mutable)
  /// @returns Iterator to the first point
  [[nodiscard]] constexpr auto begin() noexcept -> iterator {
    return points_.begin();
  }

  /// @brief Get iterator to the end (mutable)
  /// @returns Iterator to one past the last point
  [[nodiscard]] constexpr auto end() noexcept -> iterator {
    return points_.end();
  }

  /// @brief Get iterator to the beginning (const)
  /// @returns Const iterator to the first point
  [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator {
    return points_.begin();
  }
  /// @brief Get iterator to the end (const)
  /// @returns Const iterator to one past the last point
  [[nodiscard]] constexpr auto end() const noexcept -> const_iterator {
    return points_.end();
  }

 private:
  container_type points_;
};

}  // namespace pyinterp::geodetic

namespace boost::geometry::traits {

template <>
struct tag<pyinterp::geodetic::LineString> {
  using type = linestring_tag;
};

template <>
struct point_type<pyinterp::geodetic::LineString> {
  using type = pyinterp::geodetic::Point;
};

}  // namespace boost::geometry::traits
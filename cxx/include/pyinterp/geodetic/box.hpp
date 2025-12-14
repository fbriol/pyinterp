#pragma once

#include <boost/geometry.hpp>

#include "pyinterp/geodetic/algorithm/area.hpp"
#include "pyinterp/geodetic/point.hpp"
#include "pyinterp/geodetic/spheroid.hpp"
#include "pyinterp/math.hpp"

namespace pyinterp::geodetic {

/// @brief Type representing a bounding box in geodetic coordinates
class Box {
 public:
  /// @brief Default constructor creating an empty box
  constexpr Box() noexcept = default;

  /// @brief Constructor from corner points
  /// @param[in] min_corner Minimum corner point (lon, lat)
  /// @param[in] max_corner Maximum corner point (lon, lat)
  constexpr Box(const Point& min_corner, const Point& max_corner) noexcept
      : min_corner_(min_corner), max_corner_(max_corner) {}

  /// @brief Returns the global bounding box covering the entire Earth
  [[nodiscard]]
  static constexpr auto global_bounding_box() -> Box {
    return {{-180, -90}, {180, 90}};
  }

  /// @brief Get the min corner of this box (const)
  /// @return Minimum corner point (lon, lat)
  [[nodiscard]] constexpr auto min_corner() const noexcept -> Point {
    return min_corner_;
  }

  /// @brief Get the max corner of this box (const)
  /// @return Maximum corner point (lon, lat)
  [[nodiscard]] constexpr auto max_corner() const noexcept -> Point {
    return max_corner_;
  }

  /// @brief Get the min corner of this box (mutable)
  /// @return Reference to minimum corner point (lon, lat)
  [[nodiscard]] constexpr auto min_corner() noexcept -> Point& {
    return min_corner_;
  }

  /// @brief Get the max corner of this box (mutable)
  /// @return Reference to maximum corner point (lon, lat)
  [[nodiscard]] constexpr auto max_corner() noexcept -> Point& {
    return max_corner_;
  }

  /// @brief Returns the center of the box.
  /// @return Center point (lon, lat)
  [[nodiscard]] inline auto centroid() const -> Point {
    return boost::geometry::return_centroid<Point, Box>(*this);
  }

  /// @brief Returns the delta of the box in latitude and longitude.
  /// @param[in] round If true, rounds the delta to the nearest power of 10.
  /// @return A tuple containing the delta in longitude and latitude.
  [[nodiscard]] constexpr auto delta(bool round) const
      -> std::tuple<double, double> {
    auto x = max_corner_.lon() - min_corner_.lon();
    auto y = max_corner_.lat() - min_corner_.lat();
    if (round) {
      x = Box::max_decimal_power(x);
      y = Box::max_decimal_power(y);
    }
    return {x, y};
  }

  /// @brief Returns a point inside the box, making an effort to round to
  /// minimal precision.
  [[nodiscard]] constexpr auto round() const -> Point {
    const auto xy = delta(true);
    const auto x = std::get<0>(xy);
    const auto y = std::get<1>(xy);
    return {std::ceil(min_corner_.lon() / x) * x,
            std::ceil(min_corner_.lat() / y) * y};
  }

  /// @brief Check if two boxes are equal
  /// @param[in] other Other box to compare with
  /// @return True if the boxes are equal, false otherwise
  [[nodiscard]] constexpr auto operator==(const Box& other) const noexcept
      -> bool {
    return boost::geometry::equals(*this, other);
  }

 private:
  Point min_corner_{};
  Point max_corner_{};

  /// @brief Returns the maximum power of 10 from a number (x > 0)
  /// @param[in] x Input number
  /// @return Maximum power of 10 less than or equal to x
  [[nodiscard]]
  static constexpr auto max_decimal_power(const double x) -> double {
    auto m = static_cast<int32_t>(std::floor(std::log10(x)));
    return math::power10(m);
  }
};

}  // namespace pyinterp::geodetic

// Boost.Geometry traits
namespace boost::geometry::traits {

template <>
struct tag<pyinterp::geodetic::Box> {
  using type = box_tag;
};

template <>
struct point_type<pyinterp::geodetic::Box> {
  using type = pyinterp::geodetic::Point;
};

template <std::size_t I, std::size_t D>
struct indexed_access<pyinterp::geodetic::Box, I, D> {
  static auto get(const pyinterp::geodetic::Box& b) -> double {
    if constexpr (I == min_corner) {
      return b.min_corner().get<D>();
    } else {
      return b.max_corner().get<D>();
    }
  }
  static void set(pyinterp::geodetic::Box& b, double v) {
    if constexpr (I == min_corner) {
      b.min_corner().set<D>(v);
    } else {
      b.max_corner().set<D>(v);
    }
  };
};

}  // namespace boost::geometry::traits

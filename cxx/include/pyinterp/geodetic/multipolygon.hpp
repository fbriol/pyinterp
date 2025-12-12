#pragma once

#include <vector>

#include "pyinterp/geodetic/polygon.hpp"

namespace pyinterp::geodetic {

/// @brief MultiPolygon: collection of polygons.
///
/// A `MultiPolygon` represents a collection of polygons. It provides
/// basic container-like operations for constructing and iterating over
/// polygons.
class MultiPolygon {
 public:
  /// @brief Underlying container type for polygons.
  using container_type = std::vector<Polygon>;

  /// @brief Iterator over polygons.
  using iterator = container_type::iterator;

  /// @brief Const iterator over polygons.
  using const_iterator = container_type::const_iterator;

  /// @brief Default constructor. Creates an empty collection.
  constexpr MultiPolygon() = default;

  /// @brief Construct from a vector of polygons.
  /// @param[in] polygons Vector of polygons to move into the multipolygon.
  explicit constexpr MultiPolygon(std::vector<Polygon> polygons)
      : polygons_{std::move(polygons)} {}

  /// @brief Append a polygon to the collection.
  /// @param[in] pt Polygon to append.
  constexpr void push_back(const Polygon& pt) { polygons_.push_back(pt); }

  /// @brief Remove all polygons from the collection.
  constexpr void clear() noexcept { polygons_.clear(); }

  /// @brief Resize the collection.
  /// @param[in] n New number of polygons.
  constexpr void resize(std::size_t n) { polygons_.resize(n); }

  /// @brief Number of polygons in the collection.
  [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
    return polygons_.size();
  }

  /// @brief Check whether the collection is empty.
  [[nodiscard]] constexpr auto empty() const noexcept -> bool {
    return polygons_.empty();
  }

  /// @brief Access the i-th polygon (mutable).
  [[nodiscard]] constexpr auto operator[](std::size_t ix) -> Polygon& {
    return polygons_[ix];
  }

  /// @brief Access the i-th polygon (const).
  [[nodiscard]] constexpr auto operator[](std::size_t ix) const
      -> const Polygon& {
    return polygons_[ix];
  }

  /// @brief Return iterator to first polygon (mutable).
  [[nodiscard]] constexpr auto begin() noexcept -> iterator {
    return polygons_.begin();
  }

  /// @brief Return iterator to past-the-end (mutable).
  [[nodiscard]] constexpr auto end() noexcept -> iterator {
    return polygons_.end();
  }

  /// @brief Return iterator to first polygon (const).
  [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator {
    return polygons_.begin();
  }

  /// @brief Return iterator to past-the-end (const).
  [[nodiscard]] constexpr auto end() const noexcept -> const_iterator {
    return polygons_.end();
  }

 private:
  container_type polygons_;
};

}  // namespace pyinterp::geodetic

namespace boost::geometry::traits {

template <>
struct tag<pyinterp::geodetic::MultiPolygon> {
  using type = multi_polygon_tag;
};

}  // namespace boost::geometry::traits


#pragma once

#include <boost/geometry.hpp>

#include "pyinterp/geometry/geographic/algorithms/distance.hpp"
#include "pyinterp/geometry/geographic/algorithms/strategy.hpp"
#include "pyinterp/geometry/geographic/spheroid.hpp"

namespace pyinterp::geometry::geographic {

/// @brief Calculate the length using a compile-time strategy
/// @tparam Geometry Geometry type
/// @tparam Method Strategy method to use
/// @param[in] geometry Geometry object
/// @param[in] wgs Optional Spheroid for geodetic calculations
/// @return Calculated length
template <typename Geometry, StrategyMethod Method>
[[nodiscard]] inline auto length(const Geometry &geometry,
                                 const std::optional<Spheroid> &wgs) -> double {
  return boost::geometry::length(
      geometry,
      detail::make_distance_strategy<Method>(detail::make_spheroid(wgs)));
}

/// @brief Calculate the length using a runtime strategy
/// @tparam Geometry Geometry type
/// @param[in] geometry Geometry object
/// @param[in] spheroid Optional Spheroid for geodetic calculations
/// @param[in] strategy Strategy method to use
/// @return Calculated length
template <typename Geometry>
[[nodiscard]] inline auto length(const Geometry &geometry,
                                 const std::optional<Spheroid> &spheroid,
                                 const StrategyMethod strategy) -> double {
  using enum StrategyMethod;
  switch (strategy) {
    case kAndoyer:
      return length<Geometry, kAndoyer>(geometry, spheroid);
    case kKarney:
      return length<Geometry, kKarney>(geometry, spheroid);
    case kThomas:
      return length<Geometry, kThomas>(geometry, spheroid);
    case kVincenty:
      return length<Geometry, kVincenty>(geometry, spheroid);
  }
  std::unreachable();
}

}  // namespace pyinterp::geometry::geographic

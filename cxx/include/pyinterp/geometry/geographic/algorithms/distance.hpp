
#pragma once

#include <boost/geometry.hpp>

#include "pyinterp/geometry/geographic/algorithms/strategy.hpp"
#include "pyinterp/geometry/geographic/spheroid.hpp"

namespace pyinterp::geometry::geographic {

namespace detail {

/// @brief Create distance strategy based on the specified method
/// and spheroid
/// @tparam Method Strategy method to use
/// @param[in] spheroid Spheroid for geodetic calculations
/// @return distance strategy
template <StrategyMethod Method>
[[nodiscard]] constexpr auto make_distance_strategy(
    const boost::geometry::srs::spheroid<double> &spheroid) {
  if constexpr (Method == StrategyMethod::kAndoyer) {
    return boost::geometry::strategy::distance::geographic<
        boost::geometry::strategy::andoyer>(spheroid);
  } else if constexpr (Method == StrategyMethod::kKarney) {
    return boost::geometry::strategy::distance::geographic<
        boost::geometry::strategy::karney>(spheroid);
  } else if constexpr (Method == StrategyMethod::kThomas) {
    return boost::geometry::strategy::distance::geographic<
        boost::geometry::strategy::thomas>(spheroid);
  } else if constexpr (Method == StrategyMethod::kVincenty) {
    return boost::geometry::strategy::distance::geographic<
        boost::geometry::strategy::vincenty>(spheroid);
  } else {
    static_assert(false, "Unhandled StrategyMethod");
  }
}

}  // namespace detail

/// @brief Calculate the distance using a compile-time strategy
/// @tparam Geometry Geometry type
/// @tparam Method Strategy method to use
/// @param[in] geometry1 First geometric object
/// @param[in] geometry2 Second geometric object
/// @param[in] wgs Optional Spheroid for geodetic calculations
/// @return Calculated distance
template <typename Geometry, StrategyMethod Method>
[[nodiscard]] inline auto distance(const Geometry &geometry1,
                                   const Geometry &geometry2,
                                   const std::optional<Spheroid> &wgs)
    -> double {
  return boost::geometry::distance(
      geometry1, geometry2,
      detail::make_distance_strategy<Method>(detail::make_spheroid(wgs)));
}

/// @brief Calculate the distance using a runtime strategy
/// @tparam Geometry Geometry type
/// @param[in] geometry1 First geometric object
/// @param[in] geometry2 Second geometric object
/// @param[in] spheroid Optional Spheroid for geodetic calculations
/// @param[in] strategy Strategy method to use
/// @return Calculated distance
template <typename Geometry>
[[nodiscard]] inline auto distance(const Geometry &geometry1,
                                   const Geometry &geometry2,
                                   const std::optional<Spheroid> &spheroid,
                                   const StrategyMethod strategy) -> double {
  using enum StrategyMethod;
  switch (strategy) {
    case kAndoyer:
      return distance<Geometry, kAndoyer>(geometry1, geometry2, spheroid);
    case kKarney:
      return distance<Geometry, kKarney>(geometry1, geometry2, spheroid);
    case kThomas:
      return distance<Geometry, kThomas>(geometry1, geometry2, spheroid);
    case kVincenty:
      return distance<Geometry, kVincenty>(geometry1, geometry2, spheroid);
  }
  std::unreachable();
}

}  // namespace pyinterp::geometry::geographic


#pragma once

#include <boost/geometry.hpp>

#include "pyinterp/geometry/geographic/spheroid.hpp"

namespace pyinterp::geometry::geographic {

/// Strategy for geodetic calculations
enum class StrategyMethod : std::int8_t {
  kAndoyer,
  kKarney,
  kThomas,
  kVincenty
};

namespace detail {

/// @brief Create area strategy based on the specified method
/// and spheroid
/// @tparam Method Strategy method to use
/// @param[in] spheroid Spheroid for geodetic calculations
/// @return Area strategy
template <StrategyMethod Method>
[[nodiscard]] constexpr auto make_area_strategy(
    const boost::geometry::srs::spheroid<double> &spheroid) {
  if constexpr (Method == StrategyMethod::kAndoyer) {
    return boost::geometry::strategy::area::geographic<
        boost::geometry::strategy::andoyer>(spheroid);
  } else if constexpr (Method == StrategyMethod::kKarney) {
    return boost::geometry::strategy::area::geographic<
        boost::geometry::strategy::karney>(spheroid);
  } else if constexpr (Method == StrategyMethod::kThomas) {
    return boost::geometry::strategy::area::geographic<
        boost::geometry::strategy::thomas>(spheroid);
  } else if constexpr (Method == StrategyMethod::kVincenty) {
    return boost::geometry::strategy::area::geographic<
        boost::geometry::strategy::vincenty, 5>(spheroid);
  } else {
    static_assert(false, "Unhandled StrategyMethod");
  }
}

/// @brief Create spheroid from optional Spheroid
/// @param[in] wgs Optional Spheroid
/// @return Boost geometry spheroid
[[nodiscard]] inline auto make_spheroid(const std::optional<Spheroid> &wgs)
    -> boost::geometry::srs::spheroid<double> {
  return wgs.has_value() ? boost::geometry::srs::spheroid<double>(*wgs)
                         : boost::geometry::srs::spheroid<double>{};
}

}  // namespace detail

/// @brief Calculate the area using a compile-time strategy
/// @tparam Geometry Geometry type
/// @tparam Method Strategy method to use
/// @param[in] geometry Geometry object
/// @param[in] wgs Optional Spheroid for geodetic calculations
/// @return Calculated area
template <typename Geometry, StrategyMethod Method>
[[nodiscard]] inline auto area(const Geometry &geometry,
                               const std::optional<Spheroid> &wgs) -> double {
  return boost::geometry::area(
      geometry, detail::make_area_strategy<Method>(detail::make_spheroid(wgs)));
}

/// @brief Calculate the area using a runtime strategy
/// @tparam Geometry Geometry type
/// @param[in] geometry Geometry object
/// @param[in] wgs Optional Spheroid for geodetic calculations
/// @param[in] strategy Strategy method to use
/// @return Calculated area
template <typename Geometry>
[[nodiscard]] inline auto area(const Geometry &geometry,
                               const std::optional<Spheroid> &wgs,
                               const StrategyMethod strategy) -> double {
  using enum StrategyMethod;
  switch (strategy) {
    case kAndoyer:
      return area<Geometry, kAndoyer>(geometry, wgs);
    case kKarney:
      return area<Geometry, kKarney>(geometry, wgs);
    case kThomas:
      return area<Geometry, kThomas>(geometry, wgs);
    case kVincenty:
      return area<Geometry, kVincenty>(geometry, wgs);
  }
  std::unreachable();
}

}  // namespace pyinterp::geometry::geographic

// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/geometry/geographic/algorithms/distance.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;

namespace pyinterp::geometry::geographic::pybind {

constexpr auto kDistanceDoc = R"doc(
Calculate the distance between two geometric objects.

The distance is calculated on the surface of a spheroid (default: WGS84).
Different geodetic calculation strategies are available for accuracy/performance
trade-offs.

Args:
    geometry1: First geometric object.
    geometry2: Second geometric object.
    spheroid: Optional spheroid for geodetic calculations. If not provided, uses
        WGS84 ellipsoid.
    strategy: Calculation strategy.
Returns:
    Distance in meters.
)doc";

auto init_distance(nb::module_& m) -> void {
  auto distance_impl = [](const auto& geometry1, const auto& geometry2,
                          const std::optional<Spheroid>& wgs,
                          StrategyMethod strategy) -> double {
    nb::gil_scoped_release release;
    return distance(geometry1, geometry2, wgs, strategy);
  };

  ([&]<typename... Geometry>() {
    (..., m.def(
              "distance",
              [=](const Geometry& g1, const Geometry& g2,
                  const std::optional<Spheroid>& wgs, StrategyMethod strategy)
                  -> double { return distance_impl(g1, g2, wgs, strategy); },
              "geometry1"_a, "geometry2"_a, nb::kw_only(),
              "spheroid"_a = std::nullopt,
              "strategy"_a = StrategyMethod::kVincenty, kDistanceDoc));
  }).template operator()<GEOMETRY_TYPES(geographic)>();
}

}  // namespace pyinterp::geometry::geographic::pybind

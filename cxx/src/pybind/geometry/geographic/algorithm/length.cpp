// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/geometry/geographic/algorithms/length.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;

namespace pyinterp::geometry::geographic::pybind {

constexpr auto kLengthDoc = R"doc(
Calculate the length of a geometric object.

The length is calculated on the surface of a spheroid (default: WGS84).
Different geodetic calculation strategies are available for accuracy/performance
trade-offs.

Args:
    geometry: Geometric object.
    spheroid: Optional spheroid for geodetic calculations. If not provided, uses
        WGS84 ellipsoid.
    strategy: Calculation strategy.
Returns:
    Length in meters.
)doc";

auto init_length(nb::module_& m) -> void {
  auto length_impl = [](const auto& geometry,
                        const std::optional<Spheroid>& wgs,
                        StrategyMethod strategy) -> double {
    nb::gil_scoped_release release;
    return length(geometry, wgs, strategy);
  };

  ([&]<typename... Geometry>() {
    (..., m.def(
              "length",
              [=](const Geometry& g, const std::optional<Spheroid>& wgs,
                  StrategyMethod strategy) -> double {
                return length_impl(g, wgs, strategy);
              },
              "geometry"_a, nb::kw_only(), "spheroid"_a = std::nullopt,
              "strategy"_a = StrategyMethod::kVincenty, kLengthDoc));
  }).template operator()<GEOMETRY_TYPES(geographic)>();
}

}  // namespace pyinterp::geometry::geographic::pybind

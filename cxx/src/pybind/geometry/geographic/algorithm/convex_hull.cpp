// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/geometry/geographic/algorithms/convex_hull.hpp"

#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/geometry/geographic/algorithms/strategy.hpp"
#include "pyinterp/geometry/geographic/polygon.hpp"
#include "pyinterp/geometry/geographic/spheroid.hpp"
#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;
using pyinterp::geometry::pybind::GeometryNamespace;

namespace pyinterp::geometry::geographic::pybind {

constexpr auto kConvexHullDoc = R"doc(
Calculates the convex hull of a geometry.

The convex hull is the smallest convex geometry that contains all points
of the input geometry. It is always returned as a Polygon.

Args:
    geometry: Geometric object.
    spheroid: Optional spheroid for geodetic calculations. If not provided, uses
        WGS84 ellipsoid.
    strategy: Calculation strategy.

Returns:
    A Polygon representing the convex hull.
)doc";

auto init_convex_hull(nb::module_& m) -> void {
  auto convex_hull_impl = [](const auto& geometry,
                             const std::optional<Spheroid>& spheroid,
                             StrategyMethod strategy) -> Polygon {
    nb::gil_scoped_release release;
    return convex_hull(geometry, spheroid, strategy);
  };

  ([&]<typename... Geometry>() {
    (..., m.def(
              "area",
              [=](const Geometry& g, const std::optional<Spheroid>& wgs,
                  StrategyMethod strategy) -> Polygon {
                return convex_hull_impl(g, wgs, strategy);
              },
              "geometry"_a, nb::kw_only(), "spheroid"_a = std::nullopt,
              "strategy"_a = StrategyMethod::kVincenty, kConvexHullDoc));
  }).template operator()<GEOMETRY_TYPES(geographic)>();
}

}  // namespace pyinterp::geometry::geographic::pybind

// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/geometry/geographic/algorithms/area.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;

namespace pyinterp::geometry::geographic::pybind {

constexpr auto kAreaDoc = R"doc(
Calculate the area of a geometric object.

The area is calculated on the surface of a spheroid (default: WGS84).
Different geodetic calculation strategies are available for accuracy/performance
trade-offs.

Args:
    geometry: Geometric object (Box, Ring, Polygon, or MultiPolygon).
    wgs: Optional spheroid for geodetic calculations. If not provided, uses
        WGS84 ellipsoid.
    strategy: Calculation strategy.
Returns:
    Area in square meters.

Examples:
    >>> from pyinterp.geometry.geographic import Box
    >>> from pyinterp.geometry.geographic.algorithms import area
    >>> box = Box((0.0, 0.0), (1.0, 1.0))
    >>> area(box)  # Area in square meters
    12364036464.29887

Note:
    For Point, Segment, LineString, MultiPoint, and MultiLineString,
    the area is always 0.0.
)doc";

constexpr auto kStrategyDoc = R"doc(
Geodetic calculation strategy.

Available strategies:
    - ANDOYER: Andoyer method - fast but less accurate
    - KARNEY: Karney method - most accurate but slower
    - THOMAS: Thomas method - balanced accuracy and performance
    - VINCENTY: Vincenty method - good balance (default)
)doc";

auto init_area(nb::module_& m) -> void {
  // Strategy enum
  nb::enum_<StrategyMethod>(m, "Strategy", kStrategyDoc)
      .value("ANDOYER", StrategyMethod::kAndoyer, "Andoyer method")
      .value("KARNEY", StrategyMethod::kKarney, "Karney method")
      .value("THOMAS", StrategyMethod::kThomas, "Thomas method")
      .value("VINCENTY", StrategyMethod::kVincenty, "Vincenty method (default)")
      .export_values();

  auto area_impl = [](const auto& geometry, const std::optional<Spheroid>& wgs,
                      StrategyMethod strategy) -> double {
    using GeometryType = std::decay_t<decltype(geometry)>;

    if constexpr (std::is_same_v<GeometryType, Point> ||
                  std::is_same_v<GeometryType, Segment> ||
                  std::is_same_v<GeometryType, LineString> ||
                  std::is_same_v<GeometryType, MultiPoint> ||
                  std::is_same_v<GeometryType, MultiLineString>) {
      return 0.0;  // 0D/1D geometries
    } else {
      nb::gil_scoped_release release;
      return area(geometry, wgs, strategy);
    }
  };

  ([&]<typename... Geometry>() {
    (..., m.def(
              "area",
              [=](const Geometry& g, const std::optional<Spheroid>& wgs,
                  StrategyMethod strategy) -> double {
                return area_impl(g, wgs, strategy);
              },
              "geometry"_a, nb::kw_only(), "wgs"_a = std::nullopt,
              "strategy"_a = StrategyMethod::kVincenty, kAreaDoc));
  }).template operator()<GEOMETRY_TYPES(geographic)>();
}

}  // namespace pyinterp::geometry::geographic::pybind

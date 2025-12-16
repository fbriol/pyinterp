// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/geodetic/algorithms/area.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>

#include "pyinterp/geodetic/box.hpp"
#include "pyinterp/geodetic/linestring.hpp"
#include "pyinterp/geodetic/multi_linestring.hpp"
#include "pyinterp/geodetic/multi_point.hpp"
#include "pyinterp/geodetic/multi_polygon.hpp"
#include "pyinterp/geodetic/point.hpp"
#include "pyinterp/geodetic/polygon.hpp"
#include "pyinterp/geodetic/ring.hpp"
#include "pyinterp/geodetic/segment.hpp"
#include "pyinterp/geodetic/spheroid.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;

namespace pyinterp::geodetic::pybind {

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
    >>> from pyinterp.geodetic import Box, area
    >>> box = Box((0.0, 0.0), (1.0, 1.0))
    >>> area(box)  # Area in square meters
    12364036464.29887

Note:
    Point and LineString geometries have zero area.
)doc";

constexpr auto kStrategyDoc = R"doc(
Geodetic calculation strategy.

Available strategies:
    - ANDOYER: Andoyer method - fast but less accurate
    - KARNEY: Karney method - most accurate but slower
    - THOMAS: Thomas method - balanced accuracy and performance
    - VINCENTY: Vincenty method - good balance (default)
)doc";

auto init_algorithms(nb::module_& m) -> void {
  // Strategy enum
  nb::enum_<StrategyMethod>(m, "Strategy", kStrategyDoc)
      .value("ANDOYER", StrategyMethod::kAndoyer, "Andoyer method")
      .value("KARNEY", StrategyMethod::kKarney, "Karney method")
      .value("THOMAS", StrategyMethod::kThomas, "Thomas method")
      .value("VINCENTY", StrategyMethod::kVincenty, "Vincenty method (default)")
      .export_values();

  // Area function - Point (always returns 0)
  m.def(
      "area",
      [](const Point&, const std::optional<Spheroid>&,
         const StrategyMethod) -> double { return 0.0; },
      "geometry"_a, "wgs"_a = nb::none(),
      "strategy"_a = StrategyMethod::kVincenty, kAreaDoc);

  // Area function - Box
  m.def(
      "area",
      [](const Box& geometry, const std::optional<Spheroid>& wgs,
         const StrategyMethod strategy) -> double {
        return area(geometry, wgs, strategy);
      },
      "geometry"_a, "wgs"_a = nb::none(),
      "strategy"_a = StrategyMethod::kVincenty, kAreaDoc);

  // Area function - Ring
  m.def(
      "area",
      [](const Ring& geometry, const std::optional<Spheroid>& wgs,
         const StrategyMethod strategy) -> double {
        return area(geometry, wgs, strategy);
      },
      "geometry"_a, "wgs"_a = nb::none(),
      "strategy"_a = StrategyMethod::kVincenty, kAreaDoc);

  // Area function - Segment (always returns 0)
  m.def(
      "area",
      [](const Segment&, const std::optional<Spheroid>&,
         const StrategyMethod) -> double { return 0.0; },
      "geometry"_a, "wgs"_a = nb::none(),
      "strategy"_a = StrategyMethod::kVincenty, kAreaDoc);

  // Area function - LineString
  m.def(
      "area",
      [](const LineString& self, const std::optional<Spheroid>& wgs,
         const StrategyMethod strategy) -> double {
        return area(self, wgs, strategy);
      },
      "geometry"_a, "wgs"_a = nb::none(),
      "strategy"_a = StrategyMethod::kVincenty, kAreaDoc);

  // Area function - Polygon
  m.def(
      "area",
      [](const Polygon& geometry, const std::optional<Spheroid>& wgs,
         const StrategyMethod strategy) -> double {
        return area(geometry, wgs, strategy);
      },
      "geometry"_a, "wgs"_a = nb::none(),
      "strategy"_a = StrategyMethod::kVincenty, kAreaDoc);

  // Area function - MultiLineString (always returns 0)
  m.def(
      "area",
      [](const MultiLineString&, const std::optional<Spheroid>&,
         const StrategyMethod) -> double { return 0.0; },
      "geometry"_a, "wgs"_a = nb::none(),
      "strategy"_a = StrategyMethod::kVincenty, kAreaDoc);

  // Area function - MultiPoint (always returns 0)
  m.def(
      "area",
      [](const MultiPoint&, const std::optional<Spheroid>&,
         const StrategyMethod) -> double { return 0.0; },
      "geometry"_a, "wgs"_a = nb::none(),
      "strategy"_a = StrategyMethod::kVincenty, kAreaDoc);

  // Area function - MultiPolygon
  m.def(
      "area",
      [](const MultiPolygon& geometry, const std::optional<Spheroid>& wgs,
         const StrategyMethod strategy) -> double {
        return area(geometry, wgs, strategy);
      },
      "geometry"_a, "wgs"_a = nb::none(),
      "strategy"_a = StrategyMethod::kVincenty, kAreaDoc);
}

}  // namespace pyinterp::geodetic::pybind

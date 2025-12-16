// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/geometry/cartesian/box.hpp"
#include "pyinterp/geometry/cartesian/linestring.hpp"
#include "pyinterp/geometry/cartesian/multi_linestring.hpp"
#include "pyinterp/geometry/cartesian/multi_point.hpp"
#include "pyinterp/geometry/cartesian/multi_polygon.hpp"
#include "pyinterp/geometry/cartesian/point.hpp"
#include "pyinterp/geometry/cartesian/polygon.hpp"
#include "pyinterp/geometry/cartesian/ring.hpp"
#include "pyinterp/geometry/cartesian/segment.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;

namespace pyinterp::geometry::cartesian::pybind {

constexpr auto kAreaDoc = R"doc(
Calculate the area of a geometric object in Cartesian coordinates.

The area is calculated on a flat 2D plane using standard Euclidean geometry.

Args:
    geometry: Geometric object (Box, Ring, Polygon, or MultiPolygon).

Returns:
    Area in square units.

Examples:
    >>> from pyinterp.cartesian import Box
    >>> from pyinterp.cartesian.algorithms import area
    >>> box = Box((0.0, 0.0), (10.0, 10.0))
    >>> area(box)  # Area is 100.0
    100.0

Note:
    For Point, Segment, LineString, MultiPoint, and MultiLineString,
    the area is always 0.0.
)doc";

// Area function definition helper
template <typename Geometry>
auto define_area_method(nb::module_& m) -> void {
  if constexpr (std::is_same_v<Geometry, Point> ||
                std::is_same_v<Geometry, Segment> ||
                std::is_same_v<Geometry, LineString> ||
                std::is_same_v<Geometry, MultiPoint> ||
                std::is_same_v<Geometry, MultiLineString>) {
    m.def(
        "area", [](const Geometry&) -> double { return 0.0; }, "geometry"_a,
        kAreaDoc);
  } else {
    m.def(
        "area",
        [](const Geometry& geometry) -> double {
          return boost::geometry::area(geometry);
        },
        "geometry"_a, kAreaDoc);
  }
}

auto init_area(nb::module_& m) -> void {
  define_area_method<Point>(m);
  define_area_method<Box>(m);
  define_area_method<Segment>(m);
  define_area_method<Ring>(m);
  define_area_method<LineString>(m);
  define_area_method<Polygon>(m);
  define_area_method<MultiPoint>(m);
  define_area_method<MultiLineString>(m);
  define_area_method<MultiPolygon>(m);
}

}  // namespace pyinterp::geometry::cartesian::pybind

// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

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
    >>> from pyinterp.geometry.cartesian import Box
    >>> from pyinterp.geometry.cartesian.algorithms import area
    >>> box = Box((0.0, 0.0), (10.0, 10.0))
    >>> area(box)  # Area is 100.0
    100.0

Note:
    For Point, Segment, LineString, MultiPoint, and MultiLineString,
    the area is always 0.0.
)doc";

auto init_area(nb::module_& m) -> void {
  auto area_impl = [](const auto& geometry) -> double {
    using GeometryType = std::decay_t<decltype(geometry)>;

    if constexpr (std::is_same_v<GeometryType, Point> ||
                  std::is_same_v<GeometryType, Segment> ||
                  std::is_same_v<GeometryType, LineString> ||
                  std::is_same_v<GeometryType, MultiPoint> ||
                  std::is_same_v<GeometryType, MultiLineString>) {
      return 0.0;  // 0D/1D geometries
    } else {
      nb::gil_scoped_release release;
      return boost::geometry::area(geometry);
    }
  };

  ([&]<typename... Geometry>() {
    (..., m.def(
              "area", [=](const Geometry& g) -> double { return area_impl(g); },
              "geometry"_a, kAreaDoc));
  }).template operator()<GEOMETRY_TYPES(cartesian)>();
}

}  // namespace pyinterp::geometry::cartesian::pybind

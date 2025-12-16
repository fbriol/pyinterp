// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/geometry/geographic/algorithms/is_empty.hpp"

#include <nanobind/nanobind.h>

#include "pyinterp/geometry/geographic/box.hpp"
#include "pyinterp/geometry/geographic/linestring.hpp"
#include "pyinterp/geometry/geographic/multi_linestring.hpp"
#include "pyinterp/geometry/geographic/multi_point.hpp"
#include "pyinterp/geometry/geographic/multi_polygon.hpp"
#include "pyinterp/geometry/geographic/point.hpp"
#include "pyinterp/geometry/geographic/polygon.hpp"
#include "pyinterp/geometry/geographic/ring.hpp"
#include "pyinterp/geometry/geographic/segment.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;

namespace pyinterp::geometry::geographic::pybind {

constexpr auto kIsEmptyDoc = R"doc(
Check if a geometry is empty (contains no points).

Args:
    geometry: Geometric object to check.

Returns:
    True if the geometry is empty, false otherwise.

Examples:
    >>> from pyinterp.geodetic import Point, Ring
    >>> from pyinterp.geodetic.algorithms import is_empty
    >>> import numpy as np
    >>> # Empty ring
    >>> empty_ring = Ring(np.array([]), np.array([]))
    >>> is_empty(empty_ring)
    True
    >>> # Non-empty point
    >>> point = Point(1.0, 2.0)
    >>> is_empty(point)
    False
)doc";

// is_empty function definition helper
template <typename Geometry>
auto define_is_empty_method(nb::module_& m) -> void {
  m.def(
      "is_empty",
      [](const Geometry& geometry) -> bool { return is_empty(geometry); },
      "geometry"_a, kIsEmptyDoc);
}

auto init_is_empty(nb::module_& m) -> void {
  define_is_empty_method<Box>(m);
  define_is_empty_method<LineString>(m);
  define_is_empty_method<MultiLineString>(m);
  define_is_empty_method<MultiPoint>(m);
  define_is_empty_method<MultiPolygon>(m);
  define_is_empty_method<Point>(m);
  define_is_empty_method<Polygon>(m);
  define_is_empty_method<Ring>(m);
  define_is_empty_method<Segment>(m);
}

}  // namespace pyinterp::geometry::geographic::pybind

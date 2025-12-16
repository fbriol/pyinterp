// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/geometry/geographic/algorithms/is_simple.hpp"

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

constexpr auto kIsSimpleDoc = R"doc(
Check if a geometry is simple (has no self-intersections).

For linestrings and rings, a geometry is simple if it does not intersect itself
(except possibly at endpoints). For polygons, this checks both the outer ring
and inner rings.

Note: Points and boxes are always simple.

Args:
    geometry: Geometric object to check.

Returns:
    True if the geometry is simple, false otherwise.

Examples:
    >>> from pyinterp.geodetic import Ring
    >>> from pyinterp.geodetic.algorithms import is_simple
    >>> import numpy as np
    >>> # Simple square ring
    >>> lon = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    >>> lat = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    >>> ring = Ring(lon, lat)
    >>> is_simple(ring)
    True
    >>> # Self-intersecting ring (figure-8)
    >>> lon_bad = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    >>> lat_bad = np.array([0.0, 1.0, 0.0, 1.0, 0.0])
    >>> ring_bad = Ring(lon_bad, lat_bad)
    >>> is_simple(ring_bad)
    False
)doc";

// is_simple function definition helper
template <typename Geometry>
auto define_is_simple_method(nb::module_& m) -> void {
  m.def(
      "is_simple",
      [](const Geometry& geometry) -> bool { return is_simple(geometry); },
      "geometry"_a, kIsSimpleDoc);
}

auto init_is_simple(nb::module_& m) -> void {
  define_is_simple_method<Box>(m);
  define_is_simple_method<LineString>(m);
  define_is_simple_method<MultiLineString>(m);
  define_is_simple_method<MultiPoint>(m);
  define_is_simple_method<MultiPolygon>(m);
  define_is_simple_method<Point>(m);
  define_is_simple_method<Polygon>(m);
  define_is_simple_method<Ring>(m);
  define_is_simple_method<Segment>(m);
}

}  // namespace pyinterp::geometry::geographic::pybind

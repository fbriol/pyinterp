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

constexpr auto kIsSimpleDoc = R"doc(
Check if a geometric object is simple (no self-intersections).

Args:
    geometry: Geometric object to check.

Returns:
    True if the geometry is simple, False otherwise.
)doc";

template <typename Geometry>
auto define_is_simple_method(nb::module_& m) -> void {
  m.def(
      "is_simple",
      [](const Geometry& geometry) -> bool {
        return boost::geometry::is_simple(geometry);
      },
      "geometry"_a, kIsSimpleDoc);
}

auto init_is_simple(nb::module_& m) -> void {
  define_is_simple_method<Point>(m);
  define_is_simple_method<Box>(m);
  define_is_simple_method<Segment>(m);
  define_is_simple_method<Ring>(m);
  define_is_simple_method<LineString>(m);
  define_is_simple_method<Polygon>(m);
  define_is_simple_method<MultiPoint>(m);
  define_is_simple_method<MultiLineString>(m);
  define_is_simple_method<MultiPolygon>(m);
}

}  // namespace pyinterp::geometry::cartesian::pybind

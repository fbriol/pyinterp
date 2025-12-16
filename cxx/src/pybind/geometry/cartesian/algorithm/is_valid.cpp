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

constexpr auto kIsValidDoc = R"doc(
Check if a geometric object is valid according to OGC standards.

Args:
    geometry: Geometric object to check.

Returns:
    True if the geometry is valid, False otherwise.
)doc";

template <typename Geometry>
auto define_is_valid_method(nb::module_& m) -> void {
  m.def(
      "is_valid",
      [](const Geometry& geometry) -> bool {
        return boost::geometry::is_valid(geometry);
      },
      "geometry"_a, kIsValidDoc);
}

auto init_is_valid(nb::module_& m) -> void {
  define_is_valid_method<Point>(m);
  define_is_valid_method<Box>(m);
  define_is_valid_method<Segment>(m);
  define_is_valid_method<Ring>(m);
  define_is_valid_method<LineString>(m);
  define_is_valid_method<Polygon>(m);
  define_is_valid_method<MultiPoint>(m);
  define_is_valid_method<MultiLineString>(m);
  define_is_valid_method<MultiPolygon>(m);
}

}  // namespace pyinterp::geometry::cartesian::pybind

// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/geodetic/algorithms/is_valid.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>

#include <string>

#include "pyinterp/geodetic/box.hpp"
#include "pyinterp/geodetic/linestring.hpp"
#include "pyinterp/geodetic/multi_linestring.hpp"
#include "pyinterp/geodetic/multi_point.hpp"
#include "pyinterp/geodetic/multi_polygon.hpp"
#include "pyinterp/geodetic/point.hpp"
#include "pyinterp/geodetic/polygon.hpp"
#include "pyinterp/geodetic/ring.hpp"
#include "pyinterp/geodetic/segment.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;

namespace pyinterp::geodetic::pybind {

constexpr auto kIsValidDoc = R"doc(
Check if a geometry is valid according to OGC standards.

Validity rules (OGC Simple Features specification):
- Points: Always valid (unless coordinates are NaN/Inf)
- LineStrings: Must have at least 2 points
- Rings: Must be closed and have at least 4 points
- Polygons: Outer ring must be counter-clockwise, inner rings clockwise,
  rings must not cross each other or touch except at single points
- MultiPolygons: All constituent polygons must be valid

Args:
    geometry: Geometric object to check.

Returns:
    If called without return_reason:
        bool: True if the geometry is valid, false otherwise.
    If called with return_reason=True:
        tuple: (is_valid, reason) where reason is a string describing why
               the geometry is invalid (empty string if valid).

Examples:
    >>> from pyinterp.geodetic import Polygon, Ring
    >>> from pyinterp.geodetic.algorithms import is_valid
    >>> import numpy as np
    >>> # Valid polygon (counter-clockwise winding)
    >>> lon = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    >>> lat = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    >>> polygon = Polygon(Ring(lon, lat))
    >>> is_valid(polygon)
    True
    >>> # Check with reason
    >>> is_valid(polygon, return_reason=True)
    (True, '')
)doc";

// is_valid function definition helper
template <typename Geometry>
auto define_is_valid_method(nb::module_& m) -> void {
  m.def(
      "is_valid",
      [](const Geometry& geometry, bool return_reason) -> nb::object {
        if (return_reason) {
          std::string reason;
          bool valid = is_valid(geometry, reason);
          return nb::make_tuple(valid, reason);
        } else {
          return nb::cast(is_valid(geometry));
        }
      },
      "geometry"_a, "return_reason"_a = false, kIsValidDoc);
}

auto init_is_valid(nb::module_& m) -> void {
  define_is_valid_method<Box>(m);
  define_is_valid_method<LineString>(m);
  define_is_valid_method<MultiLineString>(m);
  define_is_valid_method<MultiPoint>(m);
  define_is_valid_method<MultiPolygon>(m);
  define_is_valid_method<Point>(m);
  define_is_valid_method<Polygon>(m);
  define_is_valid_method<Ring>(m);
  define_is_valid_method<Segment>(m);
}

}  // namespace pyinterp::geodetic::pybind

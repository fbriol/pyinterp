// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/pybind/geometry/algorithms/is_valid.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;
using pyinterp::geometry::pybind::GeometryNamespace;

namespace pyinterp::geometry::geographic::pybind {

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
    >>> from pyinterp.geometry.geographic import Polygon, Ring
    >>> from pyinterp.geometry.geographic.algorithms import is_valid
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

auto init_is_valid(nb::module_& m) -> void {
  pyinterp::geometry::pybind::init_is_valid<GeometryNamespace::kGeographic>(
      m, kIsValidDoc);
}

}  // namespace pyinterp::geometry::geographic::pybind

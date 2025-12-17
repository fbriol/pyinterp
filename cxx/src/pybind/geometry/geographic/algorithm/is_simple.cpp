// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/pybind/geometry/algorithms/is_simple.hpp"

#include <nanobind/nanobind.h>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;
using pyinterp::geometry::pybind::GeometryNamespace;

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
    >>> from pyinterp.geometry.geographic import Ring
    >>> from pyinterp.geometry.geographic.algorithms import is_simple
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

auto init_is_simple(nb::module_& m) -> void {
  geometry::pybind::init_is_simple<GeometryNamespace::kGeographic>(
      m, kIsSimpleDoc);
}

}  // namespace pyinterp::geometry::geographic::pybind

// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/pybind/geometry/algorithms/is_empty.hpp"

#include <nanobind/nanobind.h>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;
using pyinterp::geometry::pybind::GeometryNamespace;

namespace pyinterp::geometry::geographic::pybind {

constexpr auto kIsEmptyDoc = R"doc(
Check if a geometry is empty (contains no points).

Args:
    geometry: Geometric object to check.

Returns:
    True if the geometry is empty, false otherwise.

Examples:
    >>> from pyinterp.geometry.geographic import Point, Ring
    >>> from pyinterp.geometry.geographic.algorithms import is_empty
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

auto init_is_empty(nb::module_& m) -> void {
  geometry::pybind::init_is_empty<GeometryNamespace::kGeographic>(m,
                                                                  kIsEmptyDoc);
}

}  // namespace pyinterp::geometry::geographic::pybind

// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/pybind/geometry/algorithms/is_simple.hpp"

#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;
using pyinterp::geometry::pybind::GeometryNamespace;

namespace pyinterp::geometry::cartesian::pybind {

constexpr auto kIsSimpleDoc = R"doc(
Check if a geometric object is simple (no self-intersections).

Args:
    geometry: Geometric object to check.

Returns:
    True if the geometry is simple, False otherwise.
)doc";

auto init_is_simple(nb::module_& m) -> void {
  geometry::pybind::init_is_simple<GeometryNamespace::kCartesian>(m,
                                                                  kIsSimpleDoc);
}

}  // namespace pyinterp::geometry::cartesian::pybind

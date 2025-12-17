// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/pybind/geometry/algorithms/is_empty.hpp"

#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;
using pyinterp::geometry::pybind::GeometryNamespace;

namespace pyinterp::geometry::cartesian::pybind {

constexpr auto kIsEmptyDoc = R"doc(
Check if a geometric object is empty.

Args:
    geometry: Geometric object to check.

Returns:
    True if the geometry is empty, False otherwise.
)doc";

auto init_is_empty(nb::module_& m) -> void {
  geometry::pybind::init_is_empty<GeometryNamespace::kCartesian>(m,
                                                                 kIsEmptyDoc);
}

}  // namespace pyinterp::geometry::cartesian::pybind

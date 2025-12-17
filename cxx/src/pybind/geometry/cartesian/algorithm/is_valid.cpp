// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/pybind/geometry/algorithms/is_valid.hpp"

#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;
using pyinterp::geometry::pybind::GeometryNamespace;

namespace pyinterp::geometry::cartesian::pybind {

constexpr auto kIsValidDoc = R"doc(
Check if a geometric object is valid according to OGC standards.

Args:
    geometry: Geometric object to check.

Returns:
    True if the geometry is valid, False otherwise.
)doc";

auto init_is_valid(nb::module_& m) -> void {
  pyinterp::geometry::pybind::init_is_valid<GeometryNamespace::kCartesian>(
      m, kIsValidDoc);
}

}  // namespace pyinterp::geometry::cartesian::pybind

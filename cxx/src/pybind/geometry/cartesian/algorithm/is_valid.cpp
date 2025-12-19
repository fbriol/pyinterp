// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/pybind/geometry/algorithms/is_valid.hpp"

#include <nanobind/nanobind.h>

using pyinterp::geometry::pybind::GeometryNamespace;

namespace pyinterp::geometry::cartesian::pybind {

auto init_is_valid(nanobind::module_& m) -> void {
  pyinterp::geometry::pybind::init_is_valid<GeometryNamespace::kCartesian>(m);
}

}  // namespace pyinterp::geometry::cartesian::pybind

// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/pybind/geometry/algorithms/for_each_point.hpp"

#include <nanobind/eigen/dense.h>
#include <nanobind/nanobind.h>

namespace nb = nanobind;

namespace pyinterp::geometry::cartesian::pybind {

auto init_for_each_point(nb::module_& m) -> void {
  using namespace pyinterp::geometry::pybind;
  init_for_each_point_within<GeometryNamespace::kCartesian>(m);
  init_for_each_point_covered_by<GeometryNamespace::kCartesian>(m);
}

}  // namespace pyinterp::geometry::cartesian::pybind

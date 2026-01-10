// Copyright (c) 2026 CNES.
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.

#include "pyinterp/pybind/geometry/algorithms/within.hpp"

#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

using pyinterp::geometry::pybind::GeometryNamespace;

namespace pyinterp::geometry::geographic::pybind {

auto init_within(nanobind::module_& m) -> void {
  geometry::pybind::init_within<GeometryNamespace::kGeographic>(m);
}

}  // namespace pyinterp::geometry::geographic::pybind

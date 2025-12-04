#pragma once

#include <nanobind/nanobind.h>

namespace pyinterp::pybind::windowed {

auto init_bivariate(nanobind::module_& m) -> void;
auto init_trivariate(nanobind::module_& m) -> void;
auto init_quadrivariate(nanobind::module_& m) -> void;

}  // namespace pyinterp::pybind::windowed

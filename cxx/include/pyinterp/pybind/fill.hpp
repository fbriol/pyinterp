#pragma once

#include <nanobind/nanobind.h>

namespace pyinterp::pybind {

/// @brief Initialize fill module
/// @param[in,out] m The nanobind module
auto init_fill(nanobind::module_& m) -> void;

}  // namespace pyinterp::pybind

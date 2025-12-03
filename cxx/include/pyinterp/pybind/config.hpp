#pragma once

#include <nanobind/nanobind.h>

namespace pyinterp::pybind {

/// @brief Bind configuration for the config namespace
/// @param[in,out] m Python module
auto init_config(nanobind::module_& m) -> void;

}  // namespace pyinterp::pybind

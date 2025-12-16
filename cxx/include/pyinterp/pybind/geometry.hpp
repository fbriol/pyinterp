#pragma once

#include <nanobind/nanobind.h>

namespace pyinterp::pybind {

/// @brief Initialize geographic coordinate bindings
/// @param[in,out] m Python module
void init_geometry(nanobind::module_& m);

}  // namespace pyinterp::pybind

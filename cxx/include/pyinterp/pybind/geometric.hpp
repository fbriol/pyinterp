#pragma once

#include <nanobind/nanobind.h>

namespace pyinterp::pybind::geometric {

/// @brief Initialize bivariate bindings.
/// @param[in,out] m Nanobind module
auto init_bivariate(nanobind::module_& m) -> void;

/// @brief Initialize trivariate bindings.
/// @param[in,out] m Nanobind module
auto init_trivariate(nanobind::module_& m) -> void;

/// @brief Initialize quadrivariate bindings.
/// @param[in,out] m Nanobind module
auto init_quadrivariate(nanobind::module_& m) -> void;

}  // namespace pyinterp::pybind::geometric

#pragma once

#include <nanobind/nanobind.h>

namespace pyinterp {
namespace geodetic::pybind {

/// @brief Initialize Spheroid bindings
/// @param[in,out] m Python module
auto init_spheroid(nanobind::module_& m) -> void;

/// @brief Initialize coordinate transformation bindings
/// @param[in,out] m Python module
auto init_coordinates(nanobind::module_& m) -> void;

}  // namespace geodetic::pybind

namespace pybind {

/// @brief Initialize geodetic coordinate bindings
/// @param[in,out] m Python module
void init_geodetic(nanobind::module_& m);

}  // namespace pybind
}  // namespace pyinterp

#pragma once

#include <nanobind/nanobind.h>

namespace pyinterp {
namespace geodetic::pybind {

/// @brief Initialize Point bindings
/// @param[in,out] m Python module
auto init_point(nanobind::module_& m) -> void;

/// @brief Initialize Box bindings
/// @param[in,out] m Python module
auto init_box(nanobind::module_& m) -> void;

/// @brief Initialize Ring bindings
/// @param[in,out] m Python module
auto init_ring(nanobind::module_& m) -> void;

/// @brief Initialize LineString bindings
/// @param[in,out] m Python module
auto init_line_string(nanobind::module_& m) -> void;

/// @brief Initialize Polygon bindings
/// @param[in,out] m Python module
auto init_polygon(nanobind::module_& m) -> void;

/// @brief Initialize MultiPolygon bindings
/// @param[in,out] m Python module
auto init_multipolygon(nanobind::module_& m) -> void;

/// @brief Initialize Spheroid bindings
/// @param[in,out] m Python module
auto init_spheroid(nanobind::module_& m) -> void;

/// @brief Initialize coordinate transformation bindings
/// @param[in,out] m Python module
auto init_coordinates(nanobind::module_& m) -> void;

/// @brief Initialize algorithm bindings (area, etc.)
/// @param[in,out] m Python module
auto init_algorithms(nanobind::module_& m) -> void;

}  // namespace geodetic::pybind

namespace pybind {

/// @brief Initialize geodetic coordinate bindings
/// @param[in,out] m Python module
void init_geodetic(nanobind::module_& m);

}  // namespace pybind
}  // namespace pyinterp

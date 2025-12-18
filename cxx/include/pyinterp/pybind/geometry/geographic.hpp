#pragma once

#include <nanobind/nanobind.h>

namespace pyinterp::geometry::geographic::pybind {

/// @brief Initialize Strategy enum bindings
/// @param[in,out] m Nanobind module
auto init_strategy(nanobind::module_& m) -> void;

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
auto init_linestring(nanobind::module_& m) -> void;

/// @brief Initialize Segment bindings
/// @param[in,out] m Python module
auto init_segment(nanobind::module_& m) -> void;

/// @brief Initialize Polygon bindings
/// @param[in,out] m Python module
auto init_polygon(nanobind::module_& m) -> void;

/// @brief Initialize MultiPoint bindings
/// @param[in,out] m Python module
auto init_multipoint(nanobind::module_& m) -> void;

/// @brief Initialize MultiPolygon bindings
/// @param[in,out] m Python module
auto init_multipolygon(nanobind::module_& m) -> void;

/// @brief Initialize MultiLineString bindings
/// @param[in,out] m Python module
auto init_multilinestring(nanobind::module_& m) -> void;

/// @brief Initialize Spheroid bindings
/// @param[in,out] m Python module
auto init_spheroid(nanobind::module_& m) -> void;

/// @brief Initialize coordinate transformation bindings
/// @param[in,out] m Python module
auto init_coordinates(nanobind::module_& m) -> void;

/// @brief Initialize area algorithm bindings
/// @param[in,out] m Python module
auto init_area(nanobind::module_& m) -> void;

/// @brief Initialize azimuth algorithm bindings
/// @param[in,out] m Python module
auto init_azimuth(nanobind::module_& m) -> void;

/// @brief Initialize centroid algorithm bindings
/// @param[in,out] m Python module
auto init_centroid(nanobind::module_& m) -> void;

/// @brief Initialize closest_points algorithm bindings
/// @param[in,out] m Python module
auto init_closest_points(nanobind::module_& m) -> void;

/// @brief Initialize covered_by algorithm binding
/// @param[in,out] m Python module
auto init_covered_by(nanobind::module_& m) -> void;

/// @brief Initialize is_empty algorithm binding
/// @param[in,out] m Python module
auto init_is_empty(nanobind::module_& m) -> void;

/// @brief Initialize is_simple algorithm binding
/// @param[in,out] m Python module
auto init_is_simple(nanobind::module_& m) -> void;

/// @brief Initialize is_valid algorithm binding
/// @param[in,out] m Python module
auto init_is_valid(nanobind::module_& m) -> void;

/// @brief Initialize all algorithm bindings
/// @param[in,out] m Python module
inline void init_algorithms(nanobind::module_& m) {
  init_strategy(m);
  init_area(m);
  init_azimuth(m);
  init_centroid(m);
  init_closest_points(m);
  init_covered_by(m);
  init_is_empty(m);
  init_is_simple(m);
  init_is_valid(m);
}

}  // namespace pyinterp::geometry::geographic::pybind

namespace pyinterp::geometry::pybind {

/// @brief Initialize geographic coordinate bindings
/// @param[in,out] m Python module
void init_geographic(nanobind::module_& m);

/// @brief Initialize cartesian coordinate bindings
/// @param[in,out] m Python module
void init_cartesian(nanobind::module_& m);

}  // namespace pyinterp::geometry::pybind

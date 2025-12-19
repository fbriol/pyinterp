#pragma once
#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

constexpr auto kNumPointsDoc = R"doc(
Returns the number of points in a geometry.

For linestrings and rings, returns the number of points.
For polygons, returns the sum of points in all rings.
For multi-geometries, returns the sum of points in all components.

Args:
    geometry: Geometric object to count points in.

Returns:
    The number of points.

Examples:
    >>> ls = LineString(np.array([0.0, 1.0, 2.0]),
    ...                 np.array([0.0, 1.0, 0.0]))
    >>> num_points(ls)
    3
)doc";

/// @brief Initialize the num_points algorithm in the given module
/// @tparam NS Namespace of the geometries (cartesian or geographic)
/// @param[in,out] m Nanobind module
template <GeometryNamespace NS>
inline auto init_num_points(nanobind::module_& m) -> void {
  auto num_points_impl = [](const auto& g) -> std::size_t {
    nanobind::gil_scoped_release release;
    return boost::geometry::num_points(g);
  };
  if constexpr (NS == GeometryNamespace::kCartesian) {
    geometry::pybind::define_unary_predicate<decltype(num_points_impl),
                                             GEOMETRY_TYPES(cartesian)>(
        m, "num_points", kNumPointsDoc, std::move(num_points_impl));
  } else {
    geometry::pybind::define_unary_predicate<decltype(num_points_impl),
                                             GEOMETRY_TYPES(geographic)>(
        m, "num_points", kNumPointsDoc, std::move(num_points_impl));
  }
}

}  // namespace pyinterp::geometry::pybind

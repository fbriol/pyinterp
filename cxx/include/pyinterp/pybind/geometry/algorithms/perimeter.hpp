#pragma once
#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

constexpr auto kPerimeterDoc = R"doc(
Calculates the perimeter of an areal geometry.

For polygons and rings, this is the sum of the lengths of all rings (exterior and holes).
For other geometries, returns 0.

Args:
    geometry: Geometric object to compute perimeter for.

Returns:
    The perimeter of the geometry (in coordinate units for Cartesian,
    meters for geographic).

Examples:
    >>> # Square with side length 1
    >>> ring = Ring(np.array([0.0, 1.0, 1.0, 0.0, 0.0]),
    ...             np.array([0.0, 0.0, 1.0, 1.0, 0.0]))
    >>> perimeter(ring)
    4.0
)doc";

/// @brief Initialize the perimeter algorithm in the given module
/// @tparam NS Namespace of the geometries (cartesian or geographic)
/// @param[in,out] m Nanobind module
template <GeometryNamespace NS>
inline auto init_perimeter(nanobind::module_& m) -> void {
  auto perimeter_impl = [](const auto& g) -> double {
    nanobind::gil_scoped_release release;
    return boost::geometry::perimeter(g);
  };
  if constexpr (NS == GeometryNamespace::kCartesian) {
    geometry::pybind::define_unary_predicate<decltype(perimeter_impl),
                                             GEOMETRY_TYPES(cartesian)>(
        m, "perimeter", kPerimeterDoc, std::move(perimeter_impl));
  } else {
    geometry::pybind::define_unary_predicate<decltype(perimeter_impl),
                                             GEOMETRY_TYPES(geographic)>(
        m, "perimeter", kPerimeterDoc, std::move(perimeter_impl));
  }
}

}  // namespace pyinterp::geometry::pybind

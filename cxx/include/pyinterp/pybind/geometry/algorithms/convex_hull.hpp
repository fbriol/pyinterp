#pragma once
#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

constexpr auto kConvexHullDoc = R"doc(
Calculates the convex hull of a geometry.

The convex hull is the smallest convex geometry that contains all points
of the input geometry. It is always returned as a Polygon (or Ring for
single-component geometries).

Args:
    geometry: Geometric object to compute convex hull for.

Returns:
    A Polygon representing the convex hull.
)doc";

/// @brief Initialize the convex_hull algorithm in the given module
/// @tparam NS Namespace of the geometries (cartesian or geographic)
/// @param[in,out] m Nanobind module
template <GeometryNamespace NS>
inline auto init_convex_hull(nanobind::module_& m) -> void {
  if constexpr (NS == GeometryNamespace::kCartesian) {
    auto convex_hull_impl = [](const auto& g) -> cartesian::Polygon {
      nanobind::gil_scoped_release release;
      cartesian::Polygon result;
      boost::geometry::convex_hull(g, result);
      return result;
    };
    geometry::pybind::define_for_geometries<decltype(convex_hull_impl),
                                            GEOMETRY_TYPES(cartesian)>(
        m, "convex_hull", kConvexHullDoc, std::move(convex_hull_impl));
  } else {
    auto convex_hull_impl = [](const auto& g) -> geographic::Polygon {
      nanobind::gil_scoped_release release;
      geographic::Polygon result;
      boost::geometry::convex_hull(g, result);
      return result;
    };
    geometry::pybind::define_for_geometries<decltype(convex_hull_impl),
                                            GEOMETRY_TYPES(geographic)>(
        m, "convex_hull", kConvexHullDoc, std::move(convex_hull_impl));
  }
}

}  // namespace pyinterp::geometry::pybind

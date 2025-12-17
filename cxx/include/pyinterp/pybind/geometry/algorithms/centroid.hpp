#pragma once
#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

constexpr auto kCentroidDoc = R"doc(
Calculate the centroid of a geometry.

Args:
    geometry: Geometric object to calculate the centroid of.

Returns:
    The centroid of the geometry.

Examples:
    >>> # Centroid of a polygon
    >>> lon = np.array([0.0, 0.0, 2.0, 2.0, 0.0])
    >>> lat = np.array([0.0, 2.0, 2.0, 0.0, 0.0])
    >>> polygon = Polygon(Ring(lon, lat))
    >>> centroid(polygon)
)doc";

/// @brief Initialize the centroid algorithm in the given module
/// @tparam NS Namespace of the geometries (cartesian or geographic)
/// @param[in,out] m Nanobind module
template <GeometryNamespace NS>
inline auto init_centroid(nanobind::module_& m) -> void {
  auto centroid_impl = [](const auto& g) -> bool {
    nanobind::gil_scoped_release release;
    return boost::geometry::is_empty(g);
  };
  if constexpr (NS == GeometryNamespace::kCartesian) {
    geometry::pybind::define_for_geometries<decltype(centroid_impl),
                                            GEOMETRY_TYPES(cartesian)>(
        m, "centroid", kCentroidDoc, std::move(centroid_impl));
  } else {
    geometry::pybind::define_for_geometries<decltype(centroid_impl),
                                            GEOMETRY_TYPES(geographic)>(
        m, "centroid", kCentroidDoc, std::move(centroid_impl));
  }
}

}  // namespace pyinterp::geometry::pybind

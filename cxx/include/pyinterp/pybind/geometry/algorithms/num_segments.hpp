#pragma once
#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

constexpr auto kNumSegmentsDoc = R"doc(
Returns the number of segments in a geometry.

A segment is a line between two consecutive points. For closed geometries
like rings, the closing segment is counted.

Args:
    geometry: Geometric object to count segments in.

Returns:
    The number of segments.

Examples:
    >>> # Triangle (closed ring with 3 segments)
    >>> ring = Ring(np.array([0.0, 1.0, 0.5, 0.0]),
    ...             np.array([0.0, 0.0, 1.0, 0.0]))
    >>> num_segments(ring)
    3
)doc";

/// @brief Initialize the num_segments algorithm in the given module
/// @tparam NS Namespace of the geometries (cartesian or geographic)
/// @param[in,out] m Nanobind module
template <GeometryNamespace NS>
inline auto init_num_segments(nanobind::module_& m) -> void {
  auto num_segments_impl = [](const auto& g) -> std::size_t {
    nanobind::gil_scoped_release release;
    return boost::geometry::num_segments(g);
  };
  if constexpr (NS == GeometryNamespace::kCartesian) {
    geometry::pybind::define_unary_predicate<decltype(num_segments_impl),
                                             GEOMETRY_TYPES(cartesian)>(
        m, "num_segments", kNumSegmentsDoc, std::move(num_segments_impl));
  } else {
    geometry::pybind::define_unary_predicate<decltype(num_segments_impl),
                                             GEOMETRY_TYPES(geographic)>(
        m, "num_segments", kNumSegmentsDoc, std::move(num_segments_impl));
  }
}

}  // namespace pyinterp::geometry::pybind

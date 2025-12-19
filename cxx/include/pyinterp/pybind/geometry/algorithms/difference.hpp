#pragma once
#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>

#include <boost/geometry.hpp>
#include <vector>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

constexpr auto kDifferenceDoc = R"doc(
Computes the geometric difference between two geometries.

The difference operation returns the part of the first geometry that does not
intersect with the second geometry (A - B). The result is returned as a list
of polygons or linestrings, depending on the input geometry types.

Args:
    geometry1: First geometry (minuend).
    geometry2: Second geometry (subtrahend).

Returns:
    A list of geometries representing the difference. For polygon inputs,
    returns a list of Polygon objects. For linestring inputs, returns a list of
    LineString objects.
)doc";

/// @brief Macro for polygon geometry pairs that support difference
/// Note: Box does not support overlay operations in Boost.Geometry
#define DIFFERENCE_POLYGON_PAIRS(NS)                                         \
  std::pair<NS::Ring, NS::Ring>, std::pair<NS::Ring, NS::Polygon>,           \
      std::pair<NS::Polygon, NS::Ring>, std::pair<NS::Polygon, NS::Polygon>, \
      std::pair<NS::MultiPolygon, NS::Ring>,                                 \
      std::pair<NS::MultiPolygon, NS::Polygon>,                              \
      std::pair<NS::MultiPolygon, NS::MultiPolygon>,                         \
      std::pair<NS::Ring, NS::MultiPolygon>,                                 \
      std::pair<NS::Polygon, NS::MultiPolygon>

/// @brief Helper to define difference for polygon pairs
/// @tparam Polygon Output polygon type
/// @tparam GeometryPairs Pairs of geometry types
/// @param[in] m Python module
/// @param[in] doc Documentation string
template <typename Polygon, typename... GeometryPairs>
inline auto define_difference_for_polygon_pairs(nanobind::module_& m,
                                                const char* doc)
    -> std::vector<Polygon> {
  auto difference_impl = [](const auto& g1, const auto& g2) {
    nanobind::gil_scoped_release release;
    std::vector<Polygon> result;
    boost::geometry::difference(g1, g2, result);
    return result;
  };

  (..., m.def(
            "difference",
            [difference_impl](const typename GeometryPairs::first_type& g1,
                              const typename GeometryPairs::second_type& g2) {
              return difference_impl(g1, g2);
            },
            "geometry1"_a, "geometry2"_a, doc));
}

/// @brief Initialize the difference algorithm in the given module
/// @tparam NS Namespace of the geometries (cartesian or geographic)
/// @param[in,out] m Nanobind module
/// Note: Only polygon-polygon difference is supported by Boost.Geometry
template <GeometryNamespace NS>
inline auto init_difference(nanobind::module_& m) -> void {
  if constexpr (NS == GeometryNamespace::kCartesian) {
    // Polygon difference operations only
    define_difference_for_polygon_pairs<cartesian::Polygon,
                                        DIFFERENCE_POLYGON_PAIRS(cartesian)>(
        m, kDifferenceDoc);
  } else {
    // Polygon difference operations only
    define_difference_for_polygon_pairs<geographic::Polygon,
                                        DIFFERENCE_POLYGON_PAIRS(geographic)>(
        m, kDifferenceDoc);
  }
}

}  // namespace pyinterp::geometry::pybind

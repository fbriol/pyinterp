#pragma once
#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>

#include <boost/geometry.hpp>
#include <vector>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

constexpr auto kIntersectionDoc = R"doc(
Computes the geometric intersection between two geometries.

The intersection operation returns the part of the geometries that overlap.
The result is returned as a vector of polygons, linestrings, or points,
depending on the input geometry types and their spatial relationship.

Args:
    geometry1: First geometry.
    geometry2: Second geometry.

Returns:
    A list of geometries representing the intersection. The type depends on
    the input geometries:
    - Two polygons: returns list of Polygon objects
    - Polygon and linestring: returns list of LineString objects
    - Two linestrings: returns list of Point or LineString objects
)doc";

/// @brief Macro for polygon-polygon intersection pairs
#define INTERSECTION_POLYGON_PAIRS(NS)                                       \
  std::pair<NS::Ring, NS::Ring>, std::pair<NS::Ring, NS::Polygon>,           \
      std::pair<NS::Polygon, NS::Ring>, std::pair<NS::Polygon, NS::Polygon>, \
      std::pair<NS::MultiPolygon, NS::Ring>,                                 \
      std::pair<NS::MultiPolygon, NS::Polygon>,                              \
      std::pair<NS::MultiPolygon, NS::MultiPolygon>,                         \
      std::pair<NS::Ring, NS::MultiPolygon>,                                 \
      std::pair<NS::Polygon, NS::MultiPolygon>

/// @brief Macro for linestring-polygon intersection pairs (returns linestrings)
#define INTERSECTION_LINESTRING_POLYGON_PAIRS(NS)                              \
  std::pair<NS::LineString, NS::Ring>, std::pair<NS::LineString, NS::Polygon>, \
      std::pair<NS::Ring, NS::LineString>,                                     \
      std::pair<NS::Polygon, NS::LineString>

/// @brief Macro for linestring-linestring intersection pairs (returns points)
#define INTERSECTION_LINESTRING_PAIRS(NS) \
  std::pair<NS::LineString, NS::LineString>

/// @brief Helper to define intersection for polygon pairs
/// @tparam Polygon Output polygon type
/// @tparam GeometryPairs Pairs of geometry types
/// @param[in] m Python module
/// @param[in] doc Documentation string
template <typename Polygon, typename... GeometryPairs>
inline auto define_intersection_for_polygon_pairs(nanobind::module_& m,
                                                  const char* doc) -> void {
  auto intersection_impl = [](const auto& g1,
                              const auto& g2) -> std::vector<Polygon> {
    nanobind::gil_scoped_release release;
    std::vector<Polygon> result;
    boost::geometry::intersection(g1, g2, result);
    return result;
  };

  (..., m.def(
            "intersection",
            [intersection_impl](const typename GeometryPairs::first_type& g1,
                                const typename GeometryPairs::second_type& g2) {
              return intersection_impl(g1, g2);
            },
            "geometry1"_a, "geometry2"_a, doc));
}

/// @brief Helper to define intersection for linestring-polygon pairs
/// @tparam LineString Output linestring type
/// @tparam GeometryPairs Pairs of geometry types
/// @param[in] m Python module
/// @param[in] doc Documentation string
template <typename LineString, typename... GeometryPairs>
inline auto define_intersection_for_linestring_polygon_pairs(
    nanobind::module_& m, const char* doc) -> void {
  auto intersection_impl = [](const auto& g1,
                              const auto& g2) -> std::vector<LineString> {
    nanobind::gil_scoped_release release;
    std::vector<LineString> result;
    boost::geometry::intersection(g1, g2, result);
    return result;
  };

  (..., m.def(
            "intersection",
            [intersection_impl](const typename GeometryPairs::first_type& g1,
                                const typename GeometryPairs::second_type& g2) {
              return intersection_impl(g1, g2);
            },
            "geometry1"_a, "geometry2"_a, doc));
}

/// @brief Helper to define intersection for linestring-linestring pairs
/// @tparam Point Output point type
/// @tparam GeometryPairs Pairs of geometry types
/// @param[in] m Python module
/// @param[in] doc Documentation string
template <typename Point, typename... GeometryPairs>
inline auto define_intersection_for_linestring_pairs(nanobind::module_& m,
                                                     const char* doc) -> void {
  auto intersection_impl = [](const auto& g1,
                              const auto& g2) -> std::vector<Point> {
    nanobind::gil_scoped_release release;
    std::vector<Point> result;
    boost::geometry::intersection(g1, g2, result);
    return result;
  };

  (..., m.def(
            "intersection",
            [intersection_impl](const typename GeometryPairs::first_type& g1,
                                const typename GeometryPairs::second_type& g2) {
              return intersection_impl(g1, g2);
            },
            "geometry1"_a, "geometry2"_a, doc));
}

/// @brief Initialize the intersection algorithm in the given module
/// @tparam NS Namespace of the geometries (cartesian or geographic)
/// @param[in,out] m Nanobind module
template <GeometryNamespace NS>
inline auto init_intersection(nanobind::module_& m) -> void {
  if constexpr (NS == GeometryNamespace::kCartesian) {
    // Polygon-polygon intersection
    define_intersection_for_polygon_pairs<
        cartesian::Polygon, INTERSECTION_POLYGON_PAIRS(cartesian)>(
        m, kIntersectionDoc);

    // Linestring-polygon intersection
    define_intersection_for_linestring_polygon_pairs<
        cartesian::LineString,
        INTERSECTION_LINESTRING_POLYGON_PAIRS(cartesian)>(m, kIntersectionDoc);

    // Linestring-linestring intersection
    define_intersection_for_linestring_pairs<
        cartesian::Point, INTERSECTION_LINESTRING_PAIRS(cartesian)>(
        m, kIntersectionDoc);
  } else {
    // Polygon-polygon intersection
    define_intersection_for_polygon_pairs<
        geographic::Polygon, INTERSECTION_POLYGON_PAIRS(geographic)>(
        m, kIntersectionDoc);

    // Linestring-polygon intersection
    define_intersection_for_linestring_polygon_pairs<
        geographic::LineString,
        INTERSECTION_LINESTRING_POLYGON_PAIRS(geographic)>(m, kIntersectionDoc);

    // Linestring-linestring intersection
    define_intersection_for_linestring_pairs<
        geographic::Point, INTERSECTION_LINESTRING_PAIRS(geographic)>(
        m, kIntersectionDoc);
  }
}

}  // namespace pyinterp::geometry::pybind

#pragma once
#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>
#include <string>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

constexpr auto kRelateDoc = R"doc(
Computes the DE-9IM (Dimensionally Extended nine-Intersection Model) matrix
for two geometries.

The DE-9IM is a topological model that describes the spatial relationship between
two geometries using a 3x3 matrix. Each cell in the matrix describes the dimension
of the intersection between interior/boundary/exterior of the two geometries.

Args:
    geometry1: First geometry.
    geometry2: Second geometry.

Returns:
    A string representing the 9-character DE-9IM matrix pattern (e.g., "FF2F11212").
    Each character can be:
    - 'F' (false): empty set
    - '0': point intersection
    - '1': line intersection
    - '2': area intersection
    - '*': any dimension (don't care)

Examples:
    >>> poly1 = Polygon(...)
    >>> poly2 = Polygon(...)
    >>> de9im = relate(poly1, poly2)
    >>> # Returns string like "212101212" for overlapping polygons
    >>> # Use the pattern to determine topological relationship
)doc";

/// @brief Macro for geometry pairs that support relate
/// Note: Segment, Box pairs not supported by Boost.Geometry relate
#define RELATE_PAIRS(NS)                                                      \
  std::pair<NS::Point, NS::Point>, std::pair<NS::LineString, NS::LineString>, \
      std::pair<NS::Ring, NS::Ring>, std::pair<NS::Ring, NS::Polygon>,        \
      std::pair<NS::Polygon, NS::Ring>, std::pair<NS::Polygon, NS::Polygon>

/// @brief Helper to define relate for geometry pairs
/// @tparam GeometryPairs Pairs of geometry types
/// @param[in] m Python module
/// @param[in] doc Documentation string
template <typename... GeometryPairs>
inline auto define_relate_for_pairs(nanobind::module_& m, const char* doc)
    -> void {
  auto relate_impl = [](const auto& g1, const auto& g2) -> std::string {
    nanobind::gil_scoped_release release;
    auto de9im = boost::geometry::relation(g1, g2);
    nanobind::gil_scoped_acquire acquire;

    // Convert DE-9IM matrix to string representation
    return de9im.str();
  };

  (..., m.def(
            "relate",
            [relate_impl](const typename GeometryPairs::first_type& g1,
                          const typename GeometryPairs::second_type& g2) {
              return relate_impl(g1, g2);
            },
            "geometry1"_a, "geometry2"_a, doc));
}

/// @brief Initialize the relate algorithm in the given module
/// @tparam NS Namespace of the geometries (cartesian or geographic)
/// @param[in,out] m Nanobind module
template <GeometryNamespace NS>
inline auto init_relate(nanobind::module_& m) -> void {
  if constexpr (NS == GeometryNamespace::kCartesian) {
    define_relate_for_pairs<RELATE_PAIRS(cartesian)>(m, kRelateDoc);
  } else {
    define_relate_for_pairs<RELATE_PAIRS(geographic)>(m, kRelateDoc);
  }
}

}  // namespace pyinterp::geometry::pybind

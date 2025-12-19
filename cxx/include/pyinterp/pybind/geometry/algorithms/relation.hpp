#pragma once
#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

constexpr auto kRelationDoc = R"doc(
Returns the spatial relationship between two geometries.

The relation operation returns a value from the DE-9IM (Dimensionally Extended
nine-Intersection Model) indicating the topological relationship between two
geometries. The result is returned as an integer representing the relation type.

Args:
    geometry1: First geometry.
    geometry2: Second geometry.

Returns:
    Integer representing the relation type:
    - 0: disjoint
    - 1: intersects
    - 2: touches
    - 3: crosses
    - 4: within
    - 5: contains
    - 6: overlaps
    - 7: covers
    - 8: covered_by
    - 9: equals

Examples:
    >>> poly1 = Polygon(...)
    >>> poly2 = Polygon(...)
    >>> rel = relation(poly1, poly2)
    >>> # Returns integer indicating relationship
)doc";

/// @brief Macro for geometry pairs that support relation
/// Note: Segment, Box pairs not supported by Boost.Geometry relation
#define RELATION_PAIRS(NS)                                                    \
  std::pair<NS::Point, NS::Point>, std::pair<NS::LineString, NS::LineString>, \
      std::pair<NS::Ring, NS::Ring>, std::pair<NS::Ring, NS::Polygon>,        \
      std::pair<NS::Polygon, NS::Ring>, std::pair<NS::Polygon, NS::Polygon>

/// @brief Helper to define relation for geometry pairs
/// @tparam GeometryPairs Pairs of geometry types
/// @param[in] m Python module
/// @param[in] doc Documentation string
template <typename... GeometryPairs>
inline auto define_relation_for_pairs(nanobind::module_& m, const char* doc)
    -> void {
  auto relation_impl = [](const auto& g1, const auto& g2) -> int {
    nanobind::gil_scoped_release release;
    auto de9im = boost::geometry::relation(g1, g2);

    // Convert DE-9IM mask to simple relation type
    // Note: This is a simplified mapping for common cases
    if (boost::geometry::disjoint(g1, g2)) return 0;    // disjoint
    if (boost::geometry::equals(g1, g2)) return 9;      // equals
    if (boost::geometry::within(g1, g2)) return 4;      // within
    if (boost::geometry::covered_by(g1, g2)) return 8;  // covered_by
    if (boost::geometry::touches(g1, g2)) return 2;     // touches
    if (boost::geometry::crosses(g1, g2)) return 3;     // crosses
    if (boost::geometry::overlaps(g1, g2)) return 6;    // overlaps
    if (boost::geometry::intersects(g1, g2)) return 1;  // intersects

    return -1;  // unknown
  };

  (..., m.def(
            "relation",
            [relation_impl](const typename GeometryPairs::first_type& g1,
                            const typename GeometryPairs::second_type& g2) {
              return relation_impl(g1, g2);
            },
            "geometry1"_a, "geometry2"_a, doc));
}

/// @brief Initialize the relation algorithm in the given module
/// @tparam NS Namespace of the geometries (cartesian or geographic)
/// @param[in,out] m Nanobind module
template <GeometryNamespace NS>
inline auto init_relation(nanobind::module_& m) -> void {
  if constexpr (NS == GeometryNamespace::kCartesian) {
    define_relation_for_pairs<RELATION_PAIRS(cartesian)>(m, kRelationDoc);
  } else {
    define_relation_for_pairs<RELATION_PAIRS(geographic)>(m, kRelationDoc);
  }
}

}  // namespace pyinterp::geometry::pybind

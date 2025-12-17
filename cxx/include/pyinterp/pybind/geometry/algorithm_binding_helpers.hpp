#pragma once

#include <nanobind/nanobind.h>

#include <cstdint>

// IWYU pragma: begin_keep
// Helper functions for binding geometry algorithms with optional arguments
// using nanobind. IWYU pragmas below preserve geometry type includes that are
// required by template fold expressions but may not be detected automatically.
#include "pyinterp/geometry/cartesian/box.hpp"
#include "pyinterp/geometry/cartesian/linestring.hpp"
#include "pyinterp/geometry/cartesian/multi_linestring.hpp"
#include "pyinterp/geometry/cartesian/multi_point.hpp"
#include "pyinterp/geometry/cartesian/multi_polygon.hpp"
#include "pyinterp/geometry/cartesian/point.hpp"
#include "pyinterp/geometry/cartesian/polygon.hpp"
#include "pyinterp/geometry/cartesian/ring.hpp"
#include "pyinterp/geometry/cartesian/segment.hpp"
#include "pyinterp/geometry/geographic/box.hpp"
#include "pyinterp/geometry/geographic/linestring.hpp"
#include "pyinterp/geometry/geographic/multi_linestring.hpp"
#include "pyinterp/geometry/geographic/multi_point.hpp"
#include "pyinterp/geometry/geographic/multi_polygon.hpp"
#include "pyinterp/geometry/geographic/point.hpp"
#include "pyinterp/geometry/geographic/polygon.hpp"
#include "pyinterp/geometry/geographic/ring.hpp"
#include "pyinterp/geometry/geographic/segment.hpp"
// IWYU pragma: end_keep

namespace pyinterp::geometry::pybind {

/// @brief Enumeration to identify geometry namespace
enum class GeometryNamespace : int8_t { kGeographic, kCartesian };

namespace nb = nanobind;
using nb::literals::operator""_a;

/// @brief Helper to define a unary algorithm for multiple geometry types
/// @tparam Algorithm Algorithm functor
/// @tparam Geometries Geometry types to bind
/// @param[in] m Python module
/// @param[in] name Function name
/// @param[in] doc Documentation string
/// @param[in] alg Algorithm functor that takes a geometry
template <typename Algorithm, typename... Geometries>
inline auto define_for_geometries(nb::module_& m, const char* name,
                                  const char* doc, Algorithm&& alg) -> void {
  // Fold expression to define binding for each geometry type
  (...,
   m.def(
       name, [alg](const Geometries& g) { return alg(g); }, "geometry"_a, doc));
}

/// @brief Helper to define algorithm with optional arguments for multiple
/// geometries
///
/// Usage: Just pass the wrapper lambda directly in the fold expression
///
/// Example:
///   auto area_impl = [](const auto& g, std::optional<Spheroid> wgs, Strategy
///   s) {
///     return compute(g, wgs, s);
///   };
///
///   ([&]<typename... Gs>() {
///     (..., m.def("area",
///                 [=](const Gs& g, std::optional<Spheroid> w, Strategy s) {
///                   return area_impl(g, w, s);
///                 },
///                 "geometry"_a, nb::kw_only(),
///                 "wgs"_a = std::nullopt, "strategy"_a = Strategy::kDefault,
///                 kDoc));
///   }).template operator()<GEOMETRY_TYPES(cartesian)>();
///
/// Key: Lambda parameters must be EXPLICIT types (not auto), otherwise nanobind
/// can't determine which overload to use.

/// @brief Helper to define a mutable algorithm for multiple geometry types
/// @tparam Algorithm Algorithm functor that modifies geometry in place
/// @tparam Geometries Geometry types to bind
/// @param[in] m Python module
/// @param[in] name Function name
/// @param[in] doc Documentation string
/// @param[in] alg Algorithm functor that takes a mutable geometry reference
template <typename Algorithm, typename... Geometries>
inline auto define_mutable_for_geometries(nb::module_& m, const char* name,
                                          const char* doc, Algorithm&& alg)
    -> void {
  (..., m.def(name, [alg](Geometries& g) { alg(g); }, "geometry"_a, doc));
}

/// @brief Helper to define algorithm with optional bool argument for multiple
/// geometries
/// @tparam Algorithm Algorithm functor that takes (geometry, bool) and returns
/// nb::object
/// @tparam Geometries Geometry types to bind
/// @param[in] m Python module
/// @param[in] name Function name
/// @param[in] doc Documentation string
/// @param[in] arg_name Name of the optional boolean argument
/// @param[in] arg_default Default value for the optional boolean argument
/// @param[in] alg Algorithm functor
template <typename Algorithm, typename... Geometries>
inline auto define_with_optional_bool(nb::module_& m, const char* name,
                                      const char* doc, const char* arg_name,
                                      bool arg_default, Algorithm&& alg)
    -> void {
  (..., m.def(
            name,
            [alg](const Geometries& g, bool opt) -> nb::object {
              return alg(g, opt);
            },
            "geometry"_a, nb::kw_only(), nb::arg(arg_name) = arg_default, doc));
}

/// @brief Helper to define a binary predicate for geometry pairs
/// @tparam Predicate Binary predicate functor
/// @tparam GeometryPairs Tuple of std::pair<G1, G2> for each combination
/// @param[in] m Python module
/// @param[in] name Function name
/// @param[in] doc Documentation string
/// @param[in] pred Predicate functor that takes two geometries
template <typename Predicate, typename... GeometryPairs>
inline auto define_binary_predicate(nb::module_& m, const char* name,
                                    const char* doc, Predicate&& pred) -> void {
  // Helper to unpack std::pair and define binding
  auto define_pair = [&]<typename Pair>(Pair*) {
    using G1 = typename Pair::first_type;
    using G2 = typename Pair::second_type;
    m.def(
        name, [pred](const G1& g1, const G2& g2) { return pred(g1, g2); },
        "geometry1"_a, "geometry2"_a, doc);
  };

  // Fold expression to define binding for each pair
  (..., define_pair(static_cast<GeometryPairs*>(nullptr)));
}

/// @brief Macro to create a list of all standard geometry types for a
/// coordinate system
/// @param NS Namespace containing the geometry types (e.g., geographic,
/// cartesian)
#define GEOMETRY_TYPES(NS)                                                \
  NS::Point, NS::Box, NS::Segment, NS::LineString, NS::Ring, NS::Polygon, \
      NS::MultiPoint, NS::MultiLineString, NS::MultiPolygon

/// @brief Macro to create common binary geometry pairs
/// @param NS Namespace containing the geometry types
#define GEOMETRY_PAIRS(NS)                                                   \
  std::pair<NS::Point, NS::Point>, std::pair<NS::Point, NS::Box>,            \
      std::pair<NS::Point, NS::Polygon>,                                     \
      std::pair<NS::Point, NS::MultiPolygon>, std::pair<NS::Box, NS::Point>, \
      std::pair<NS::Box, NS::Box>, std::pair<NS::Box, NS::Polygon>,          \
      std::pair<NS::LineString, NS::LineString>,                             \
      std::pair<NS::LineString, NS::Polygon>,                                \
      std::pair<NS::LineString, NS::Box>, std::pair<NS::Polygon, NS::Point>, \
      std::pair<NS::Polygon, NS::Box>, std::pair<NS::Polygon, NS::Polygon>,  \
      std::pair<NS::Polygon, NS::MultiPolygon>,                              \
      std::pair<NS::MultiPolygon, NS::Point>,                                \
      std::pair<NS::MultiPolygon, NS::Polygon>,                              \
      std::pair<NS::MultiPolygon, NS::MultiPolygon>

}  // namespace pyinterp::geometry::pybind

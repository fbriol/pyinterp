#pragma once
#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>
#include <sstream>
#include <string>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

constexpr auto kToWktDoc = R"doc(
Converts a geometry to Well-Known Text (WKT) representation.

WKT is a text markup language for representing vector geometry objects. It provides
a human-readable format for geometry data interchange.

Args:
    geometry: The geometry to convert.

Returns:
    A string containing the WKT representation (e.g., "POINT(1 2)").

Examples:
    >>> point = Point(1.0, 2.0)
    >>> wkt = to_wkt(point)
    >>> # Returns "POINT(1 2)"

    >>> polygon = Polygon(...)
    >>> wkt = to_wkt(polygon)
    >>> # Returns "POLYGON((...))"
)doc";

constexpr auto kFromWktDoc = R"doc(
Creates a geometry from Well-Known Text (WKT) representation.

Parses a WKT string and constructs the corresponding geometry object.

Args:
    wkt: WKT string representation of the geometry.

Returns:
    The geometry object constructed from the WKT string.

Examples:
    >>> point = from_wkt_point("POINT(1 2)")
    >>> # Returns Point(1.0, 2.0)

    >>> polygon = from_wkt_polygon("POLYGON((0 0, 1 0, 1 1, 0 1, 0 0))")
    >>> # Returns Polygon object
)doc";

/// @brief Macro for all geometry types supporting WKT
#define WKT_TYPES(NS)                                               \
  NS::Point, NS::LineString, NS::Ring, NS::Polygon, NS::MultiPoint, \
      NS::MultiLineString, NS::MultiPolygon

/// @brief Helper to define to_wkt for geometry types
/// @tparam Geometries Geometry types
/// @param[in] m Python module
/// @param[in] doc Documentation string
template <typename... Geometries>
inline auto define_to_wkt(nanobind::module_& m, const char* doc) -> void {
  auto to_wkt_impl = [](const auto& g) -> std::string {
    nanobind::gil_scoped_release release;
    std::ostringstream oss;
    oss << boost::geometry::wkt(g);
    return oss.str();
  };

  (...,
   m.def(
       "to_wkt", [to_wkt_impl](const Geometries& g) { return to_wkt_impl(g); },
       "geometry"_a, doc));
}

/// @brief Helper to define from_wkt for a specific geometry type
/// @tparam Geometry Geometry type
/// @param[in] m Python module
/// @param[in] name Function name (e.g., "from_wkt_point")
/// @param[in] doc Documentation string
template <typename Geometry>
inline auto define_from_wkt(nanobind::module_& m, const char* name,
                            const char* doc) -> void {
  m.def(
      name,
      [](const std::string& wkt) -> Geometry {
        nanobind::gil_scoped_release release;
        Geometry geom;
        boost::geometry::read_wkt(wkt, geom);
        return geom;
      },
      "wkt"_a, doc);
}

/// @brief Initialize WKT algorithms in the given module
/// @tparam NS Namespace of the geometries (cartesian or geographic)
/// @param[in,out] m Nanobind module
template <GeometryNamespace NS>
inline auto init_wkt(nanobind::module_& m) -> void {
  if constexpr (NS == GeometryNamespace::kCartesian) {
    // to_wkt for all geometry types
    define_to_wkt<WKT_TYPES(cartesian)>(m, kToWktDoc);

    // from_wkt for each geometry type
    define_from_wkt<cartesian::Point>(m, "from_wkt_point", kFromWktDoc);
    define_from_wkt<cartesian::LineString>(m, "from_wkt_linestring",
                                           kFromWktDoc);
    define_from_wkt<cartesian::Ring>(m, "from_wkt_ring", kFromWktDoc);
    define_from_wkt<cartesian::Polygon>(m, "from_wkt_polygon", kFromWktDoc);
    define_from_wkt<cartesian::MultiPoint>(m, "from_wkt_multipoint",
                                           kFromWktDoc);
    define_from_wkt<cartesian::MultiLineString>(m, "from_wkt_multilinestring",
                                                kFromWktDoc);
    define_from_wkt<cartesian::MultiPolygon>(m, "from_wkt_multipolygon",
                                             kFromWktDoc);
  } else {
    // to_wkt for all geometry types
    define_to_wkt<WKT_TYPES(geographic)>(m, kToWktDoc);

    // from_wkt for each geometry type
    define_from_wkt<geographic::Point>(m, "from_wkt_point", kFromWktDoc);
    define_from_wkt<geographic::LineString>(m, "from_wkt_linestring",
                                            kFromWktDoc);
    define_from_wkt<geographic::Ring>(m, "from_wkt_ring", kFromWktDoc);
    define_from_wkt<geographic::Polygon>(m, "from_wkt_polygon", kFromWktDoc);
    define_from_wkt<geographic::MultiPoint>(m, "from_wkt_multipoint",
                                            kFromWktDoc);
    define_from_wkt<geographic::MultiLineString>(m, "from_wkt_multilinestring",
                                                 kFromWktDoc);
    define_from_wkt<geographic::MultiPolygon>(m, "from_wkt_multipolygon",
                                              kFromWktDoc);
  }
}

}  // namespace pyinterp::geometry::pybind

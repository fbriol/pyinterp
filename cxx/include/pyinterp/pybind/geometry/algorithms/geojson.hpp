#pragma once
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <boost/geometry.hpp>
#include <iomanip>
#include <sstream>
#include <string>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

constexpr auto kToGeojsonDoc = R"doc(
Converts a geometry to GeoJSON representation.

GeoJSON is a format for encoding geographic data structures using JSON.
This function converts Boost.Geometry objects to their GeoJSON equivalent.

Args:
    geometry: The geometry to convert.

Returns:
    A JSON string representing the geometry in GeoJSON format.

Examples:
    >>> point = Point(1.0, 2.0)
    >>> geojson = to_geojson(point)
    >>> # Returns '{"type":"Point","coordinates":[1.0,2.0]}'
)doc";

constexpr auto kFromGeojsonDoc = R"doc(
Creates a geometry from GeoJSON representation.

Parses a GeoJSON string and constructs the corresponding geometry object.

Args:
    geojson: GeoJSON string representation of the geometry.

Returns:
    The geometry object constructed from the GeoJSON string.

Examples:
    >>> geojson_str = '{"type":"Point","coordinates":[1.0,2.0]}'
    >>> point = from_geojson_point(geojson_str)
    >>> # Returns Point object
)doc";

/// @brief Helper to convert a Point to GeoJSON coordinates array
template <typename Point>
inline auto point_to_coords(const Point& pt) -> std::string {
  std::ostringstream oss;
  oss << std::setprecision(17) << "[" << boost::geometry::get<0>(pt) << ","
      << boost::geometry::get<1>(pt) << "]";
  return oss.str();
}

/// @brief Helper to convert LineString to GeoJSON coordinates array
template <typename LineString>
inline auto linestring_to_coords(const LineString& ls) -> std::string {
  std::ostringstream oss;
  oss << "[";
  bool first = true;
  for (const auto& pt : ls) {
    if (!first) oss << ",";
    oss << point_to_coords(pt);
    first = false;
  }
  oss << "]";
  return oss.str();
}

/// @brief Helper to convert Ring to GeoJSON coordinates array
template <typename Ring>
inline auto ring_to_coords(const Ring& ring) -> std::string {
  std::ostringstream oss;
  oss << "[";
  bool first = true;
  for (const auto& pt : ring) {
    if (!first) oss << ",";
    oss << point_to_coords(pt);
    first = false;
  }
  oss << "]";
  return oss.str();
}

/// @brief Helper to convert Polygon to GeoJSON coordinates array
template <typename Polygon>
inline auto polygon_to_coords(const Polygon& poly) -> std::string {
  std::ostringstream oss;
  oss << "[";
  // Outer ring
  oss << ring_to_coords(poly.outer());
  // Inner rings (holes)
  for (const auto& inner : poly.inners()) {
    oss << "," << ring_to_coords(inner);
  }
  oss << "]";
  return oss.str();
}

/// @brief Convert Point to GeoJSON
template <typename Point>
inline auto point_to_geojson(const Point& pt) -> std::string {
  return "{\"type\":\"Point\",\"coordinates\":" + point_to_coords(pt) + "}";
}

/// @brief Convert LineString to GeoJSON
template <typename LineString>
inline auto linestring_to_geojson(const LineString& ls) -> std::string {
  return "{\"type\":\"LineString\",\"coordinates\":" +
         linestring_to_coords(ls) + "}";
}

/// @brief Convert Ring to GeoJSON (as LineString)
template <typename Ring>
inline auto ring_to_geojson(const Ring& ring) -> std::string {
  return "{\"type\":\"LineString\",\"coordinates\":" + ring_to_coords(ring) +
         "}";
}

/// @brief Convert Polygon to GeoJSON
template <typename Polygon>
inline auto polygon_to_geojson(const Polygon& poly) -> std::string {
  return "{\"type\":\"Polygon\",\"coordinates\":" + polygon_to_coords(poly) +
         "}";
}

/// @brief Convert MultiPoint to GeoJSON
template <typename MultiPoint>
inline auto multipoint_to_geojson(const MultiPoint& mp) -> std::string {
  std::ostringstream oss;
  oss << "{\"type\":\"MultiPoint\",\"coordinates\":[";
  bool first = true;
  for (const auto& pt : mp) {
    if (!first) oss << ",";
    oss << point_to_coords(pt);
    first = false;
  }
  oss << "]}";
  return oss.str();
}

/// @brief Convert MultiLineString to GeoJSON
template <typename MultiLineString>
inline auto multilinestring_to_geojson(const MultiLineString& mls)
    -> std::string {
  std::ostringstream oss;
  oss << "{\"type\":\"MultiLineString\",\"coordinates\":[";
  bool first = true;
  for (const auto& ls : mls) {
    if (!first) oss << ",";
    oss << linestring_to_coords(ls);
    first = false;
  }
  oss << "]}";
  return oss.str();
}

/// @brief Convert MultiPolygon to GeoJSON
template <typename MultiPolygon>
inline auto multipolygon_to_geojson(const MultiPolygon& mpoly) -> std::string {
  std::ostringstream oss;
  oss << "{\"type\":\"MultiPolygon\",\"coordinates\":[";
  bool first = true;
  for (const auto& poly : mpoly) {
    if (!first) oss << ",";
    oss << polygon_to_coords(poly);
    first = false;
  }
  oss << "]}";
  return oss.str();
}

/// @brief Macro for all geometry types supporting GeoJSON
#define GEOJSON_TYPES(NS)                                           \
  NS::Point, NS::LineString, NS::Ring, NS::Polygon, NS::MultiPoint, \
      NS::MultiLineString, NS::MultiPolygon

/// @brief Helper to define to_geojson for geometry types
/// @tparam Geometries Geometry types
/// @param[in] m Python module
/// @param[in] doc Documentation string
template <typename... Geometries>
inline auto define_to_geojson(nanobind::module_& m, const char* doc) -> void {
  (...,
   m.def(
       "to_geojson",
       [](const Geometries& g) -> std::string {
         nanobind::gil_scoped_release release;
         // Dispatch based on geometry type tag
         using Tag = typename boost::geometry::traits::tag<Geometries>::type;
         if constexpr (std::is_same_v<Tag, boost::geometry::point_tag>) {
           return point_to_geojson(g);
         } else if constexpr (std::is_same_v<Tag,
                                             boost::geometry::linestring_tag>) {
           return linestring_to_geojson(g);
         } else if constexpr (std::is_same_v<Tag, boost::geometry::ring_tag>) {
           return ring_to_geojson(g);
         } else if constexpr (std::is_same_v<Tag,
                                             boost::geometry::polygon_tag>) {
           return polygon_to_geojson(g);
         } else if constexpr (std::is_same_v<
                                  Tag, boost::geometry::multi_point_tag>) {
           return multipoint_to_geojson(g);
         } else if constexpr (std::is_same_v<
                                  Tag, boost::geometry::multi_linestring_tag>) {
           return multilinestring_to_geojson(g);
         } else if constexpr (std::is_same_v<
                                  Tag, boost::geometry::multi_polygon_tag>) {
           return multipolygon_to_geojson(g);
         }
         return "{}";
       },
       "geometry"_a, doc));
}

/// @brief Simple JSON parser helpers for GeoJSON
namespace json {

/// @brief Skip whitespace in JSON string
inline auto skip_whitespace(const std::string& str, size_t& pos) -> void {
  while (pos < str.size() && (str[pos] == ' ' || str[pos] == '\t' ||
                              str[pos] == '\n' || str[pos] == '\r')) {
    ++pos;
  }
}

/// @brief Parse a number from JSON
inline auto parse_number(const std::string& str, size_t& pos) -> double {
  size_t start = pos;
  if (str[pos] == '-') ++pos;
  while (pos < str.size() &&
         (std::isdigit(str[pos]) || str[pos] == '.' || str[pos] == 'e' ||
          str[pos] == 'E' || str[pos] == '+' || str[pos] == '-')) {
    ++pos;
  }
  return std::stod(str.substr(start, pos - start));
}

/// @brief Parse a string value from JSON
inline auto parse_string(const std::string& str, size_t& pos) -> std::string {
  if (str[pos] != '"') throw std::runtime_error("Expected '\"'");
  ++pos;
  size_t start = pos;
  while (pos < str.size() && str[pos] != '"') {
    if (str[pos] == '\\') ++pos;  // Skip escaped character
    ++pos;
  }
  std::string result = str.substr(start, pos - start);
  ++pos;  // Skip closing quote
  return result;
}

/// @brief Expect a specific character
inline auto expect_char(const std::string& str, size_t& pos, char c) -> void {
  skip_whitespace(str, pos);
  if (pos >= str.size() || str[pos] != c) {
    throw std::runtime_error(std::string("Expected '") + c + "'");
  }
  ++pos;
}

/// @brief Parse point coordinates [x, y]
template <typename Point>
inline auto parse_point_coords(const std::string& str, size_t& pos) -> Point {
  Point pt;
  expect_char(str, pos, '[');
  skip_whitespace(str, pos);
  double x = parse_number(str, pos);
  skip_whitespace(str, pos);
  expect_char(str, pos, ',');
  skip_whitespace(str, pos);
  double y = parse_number(str, pos);
  skip_whitespace(str, pos);
  expect_char(str, pos, ']');

  boost::geometry::set<0>(pt, x);
  boost::geometry::set<1>(pt, y);
  return pt;
}

/// @brief Parse array of point coordinates
template <typename Point>
inline auto parse_point_array(const std::string& str, size_t& pos)
    -> std::vector<Point> {
  std::vector<Point> points;
  expect_char(str, pos, '[');
  skip_whitespace(str, pos);

  while (pos < str.size() && str[pos] != ']') {
    points.push_back(parse_point_coords<Point>(str, pos));
    skip_whitespace(str, pos);
    if (str[pos] == ',') {
      ++pos;
      skip_whitespace(str, pos);
    }
  }

  expect_char(str, pos, ']');
  return points;
}

/// @brief Parse array of linestring coordinates
template <typename Point>
inline auto parse_linestring_array(const std::string& str, size_t& pos)
    -> std::vector<std::vector<Point>> {
  std::vector<std::vector<Point>> linestrings;
  expect_char(str, pos, '[');
  skip_whitespace(str, pos);

  while (pos < str.size() && str[pos] != ']') {
    linestrings.push_back(parse_point_array<Point>(str, pos));
    skip_whitespace(str, pos);
    if (str[pos] == ',') {
      ++pos;
      skip_whitespace(str, pos);
    }
  }

  expect_char(str, pos, ']');
  return linestrings;
}

/// @brief Parse array of polygon coordinates
template <typename Point>
inline auto parse_polygon_array(const std::string& str, size_t& pos)
    -> std::vector<std::vector<std::vector<Point>>> {
  std::vector<std::vector<std::vector<Point>>> polygons;
  expect_char(str, pos, '[');
  skip_whitespace(str, pos);

  while (pos < str.size() && str[pos] != ']') {
    polygons.push_back(parse_linestring_array<Point>(str, pos));
    skip_whitespace(str, pos);
    if (str[pos] == ',') {
      ++pos;
      skip_whitespace(str, pos);
    }
  }

  expect_char(str, pos, ']');
  return polygons;
}

}  // namespace json

/// @brief Helper to define from_geojson for a specific geometry type
/// @tparam Geometry Geometry type
/// @param[in] m Python module
/// @param[in] name Function name (e.g., "from_geojson_point")
/// @param[in] doc Documentation string
template <typename Geometry>
inline auto define_from_geojson(nanobind::module_& m, const char* name,
                                const char* doc) -> void {
  m.def(
      name,
      [](const std::string& geojson_str) -> Geometry {
        nanobind::gil_scoped_release release;
        Geometry geom;

        size_t pos = 0;
        json::skip_whitespace(geojson_str, pos);
        json::expect_char(geojson_str, pos, '{');

        // Parse GeoJSON object to find coordinates
        std::string coords_str;
        while (pos < geojson_str.size() && geojson_str[pos] != '}') {
          json::skip_whitespace(geojson_str, pos);
          if (geojson_str[pos] == '"') {
            std::string key = json::parse_string(geojson_str, pos);
            json::skip_whitespace(geojson_str, pos);
            json::expect_char(geojson_str, pos, ':');
            json::skip_whitespace(geojson_str, pos);

            if (key == "coordinates") {
              // Found coordinates, parse based on geometry type
              using Tag = typename boost::geometry::traits::tag<Geometry>::type;
              using Point =
                  typename boost::geometry::point_type<Geometry>::type;

              if constexpr (std::is_same_v<Tag, boost::geometry::point_tag>) {
                geom = json::parse_point_coords<Geometry>(geojson_str, pos);
              } else if constexpr (std::is_same_v<
                                       Tag, boost::geometry::linestring_tag> ||
                                   std::is_same_v<Tag,
                                                  boost::geometry::ring_tag>) {
                auto points = json::parse_point_array<Point>(geojson_str, pos);
                boost::geometry::assign_points(geom, points);
              } else if constexpr (std::is_same_v<
                                       Tag, boost::geometry::polygon_tag>) {
                auto rings =
                    json::parse_linestring_array<Point>(geojson_str, pos);
                if (!rings.empty()) {
                  boost::geometry::assign_points(geom.outer(), rings[0]);
                  for (size_t i = 1; i < rings.size(); ++i) {
                    typename Geometry::ring_type inner;
                    boost::geometry::assign_points(inner, rings[i]);
                    geom.inners().push_back(inner);
                  }
                }
              } else if constexpr (std::is_same_v<
                                       Tag, boost::geometry::multi_point_tag>) {
                auto points = json::parse_point_array<Point>(geojson_str, pos);
                for (const auto& pt : points) {
                  geom.push_back(pt);
                }
              } else if constexpr (std::is_same_v<
                                       Tag,
                                       boost::geometry::multi_linestring_tag>) {
                auto linestrings =
                    json::parse_linestring_array<Point>(geojson_str, pos);
                for (const auto& ls_points : linestrings) {
                  pyinterp::geometry::LineString<Point> ls{ls_points};
                  geom.push_back(ls);
                }
              } else if constexpr (std::is_same_v<
                                       Tag,
                                       boost::geometry::multi_polygon_tag>) {
                auto polygons =
                    json::parse_polygon_array<Point>(geojson_str, pos);
                for (const auto& poly_rings : polygons) {
                  pyinterp::geometry::Polygon<Point> poly;
                  if (!poly_rings.empty()) {
                    poly.outer() =
                        pyinterp::geometry::Ring<Point>{poly_rings[0]};
                    for (size_t i = 1; i < poly_rings.size(); ++i) {
                      poly.inners().push_back(
                          pyinterp::geometry::Ring<Point>{poly_rings[i]});
                    }
                  }
                  geom.push_back(poly);
                }
              }
              break;
            } else {
              // Skip other fields
              if (geojson_str[pos] == '"') {
                json::parse_string(geojson_str, pos);
              } else if (geojson_str[pos] == '[' || geojson_str[pos] == '{') {
                // Skip complex values - simple bracket counting
                char open = geojson_str[pos];
                char close = (open == '[') ? ']' : '}';
                int depth = 0;
                do {
                  if (geojson_str[pos] == open) ++depth;
                  if (geojson_str[pos] == close) --depth;
                  ++pos;
                } while (depth > 0 && pos < geojson_str.size());
              } else {
                // Skip primitive values
                while (pos < geojson_str.size() && geojson_str[pos] != ',' &&
                       geojson_str[pos] != '}') {
                  ++pos;
                }
              }
            }
          }
          json::skip_whitespace(geojson_str, pos);
          if (geojson_str[pos] == ',') {
            ++pos;
          }
        }

        return geom;
      },
      "geojson"_a, doc);
}

/// @brief Initialize GeoJSON algorithms in the given module
/// @tparam NS Namespace of the geometries (cartesian or geographic)
/// @param[in,out] m Nanobind module
template <GeometryNamespace NS>
inline auto init_geojson(nanobind::module_& m) -> void {
  if constexpr (NS == GeometryNamespace::kCartesian) {
    // to_geojson for all geometry types
    define_to_geojson<GEOJSON_TYPES(cartesian)>(m, kToGeojsonDoc);

    // from_geojson for each geometry type
    define_from_geojson<cartesian::Point>(m, "from_geojson_point",
                                          kFromGeojsonDoc);
    define_from_geojson<cartesian::LineString>(m, "from_geojson_linestring",
                                               kFromGeojsonDoc);
    define_from_geojson<cartesian::Ring>(m, "from_geojson_ring",
                                         kFromGeojsonDoc);
    define_from_geojson<cartesian::Polygon>(m, "from_geojson_polygon",
                                            kFromGeojsonDoc);
    define_from_geojson<cartesian::MultiPoint>(m, "from_geojson_multipoint",
                                               kFromGeojsonDoc);
    define_from_geojson<cartesian::MultiLineString>(
        m, "from_geojson_multilinestring", kFromGeojsonDoc);
    define_from_geojson<cartesian::MultiPolygon>(m, "from_geojson_multipolygon",
                                                 kFromGeojsonDoc);
  } else {
    // to_geojson for all geometry types
    define_to_geojson<GEOJSON_TYPES(geographic)>(m, kToGeojsonDoc);

    // from_geojson for each geometry type
    define_from_geojson<geographic::Point>(m, "from_geojson_point",
                                           kFromGeojsonDoc);
    define_from_geojson<geographic::LineString>(m, "from_geojson_linestring",
                                                kFromGeojsonDoc);
    define_from_geojson<geographic::Ring>(m, "from_geojson_ring",
                                          kFromGeojsonDoc);
    define_from_geojson<geographic::Polygon>(m, "from_geojson_polygon",
                                             kFromGeojsonDoc);
    define_from_geojson<geographic::MultiPoint>(m, "from_geojson_multipoint",
                                                kFromGeojsonDoc);
    define_from_geojson<geographic::MultiLineString>(
        m, "from_geojson_multilinestring", kFromGeojsonDoc);
    define_from_geojson<geographic::MultiPolygon>(
        m, "from_geojson_multipolygon", kFromGeojsonDoc);
  }
}

}  // namespace pyinterp::geometry::pybind

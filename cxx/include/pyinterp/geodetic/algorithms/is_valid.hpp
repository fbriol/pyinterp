#pragma once

#include <boost/geometry.hpp>
#include <string>

namespace pyinterp::geodetic {

/// @brief Check if a geometry is valid according to OGC standards
/// @tparam Geometry Geometry type
/// @param[in] geometry Geometry object
/// @return True if the geometry is valid, false otherwise
///
/// @details
/// Validity rules (OGC Simple Features specification):
/// - Points: Always valid (unless coordinates are NaN/Inf)
/// - LineStrings: Must have at least 2 points
/// - Rings: Must be closed and have at least 4 points
/// - Polygons: Outer ring must be counter-clockwise, inner rings clockwise,
///   rings must not cross each other or touch except at single points
/// - MultiPolygons: All constituent polygons must be valid
template <typename Geometry>
[[nodiscard]] inline auto is_valid(const Geometry& geometry) -> bool {
  return boost::geometry::is_valid(geometry);
}

/// @brief Check if a geometry is valid and get reason if not
/// @tparam Geometry Geometry type
/// @param[in] geometry Geometry object
/// @param[out] reason String describing why the geometry is invalid
/// @return True if the geometry is valid, false otherwise
template <typename Geometry>
[[nodiscard]] inline auto is_valid(const Geometry& geometry,
                                   std::string& reason) -> bool {
  return boost::geometry::is_valid(geometry, reason);
}

}  // namespace pyinterp::geodetic

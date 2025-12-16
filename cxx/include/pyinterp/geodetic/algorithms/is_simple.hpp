#pragma once

#include <boost/geometry.hpp>

namespace pyinterp::geodetic {

/// @brief Check if a geometry is simple (has no self-intersections)
/// @tparam Geometry Geometry type
/// @param[in] geometry Geometry object
/// @return True if the geometry is simple, false otherwise
///
/// @details
/// For linestrings and rings, a geometry is simple if it does not
/// intersect itself (except possibly at endpoints). For polygons,
/// this checks both the outer ring and inner rings.
///
/// Note: Points and boxes are always simple.
template <typename Geometry>
[[nodiscard]] inline auto is_simple(const Geometry& geometry) -> bool {
  return boost::geometry::is_simple(geometry);
}

}  // namespace pyinterp::geodetic

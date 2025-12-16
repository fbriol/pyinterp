#pragma once

#include <boost/geometry.hpp>

namespace pyinterp::geodetic {

/// @brief Check if a geometry is empty (contains no points)
/// @tparam Geometry Geometry type
/// @param[in] geometry Geometry object
/// @return True if the geometry is empty, false otherwise
template <typename Geometry>
[[nodiscard]] inline auto is_empty(const Geometry& geometry) -> bool {
  return boost::geometry::is_empty(geometry);
}

}  // namespace pyinterp::geodetic

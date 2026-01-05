#pragma once

#include <Eigen/Core>
#include <boost/geometry.hpp>

#include "pyinterp/eigen.hpp"
#include "pyinterp/geometry/cartesian/multi_point.hpp"

namespace pyinterp::geometry::cartesian {

/// @brief Calculate distance from each point in source to target geometry
/// @tparam SourceGeometry Source geometry type (MultiPoint, LineString, Ring)
/// @tparam TargetGeometry Target geometry type
/// @param[in] source Source geometry containing points
/// @param[in] target Target geometry to measure distance to
/// @return Vector of distances for each point
template <typename SourceGeometry, typename TargetGeometry>
[[nodiscard]] inline auto for_each_point_distance(const SourceGeometry& source,
                                                  const TargetGeometry& target)
    -> Eigen::VectorXd {
  Eigen::VectorXd result(source.size());
  for (std::size_t i = 0; i < source.size(); ++i) {
    result(i) = boost::geometry::distance(source[i], target);
  }
  return result;
}

}  // namespace pyinterp::geometry::cartesian

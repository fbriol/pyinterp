// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#pragma once

#include <Eigen/Core>
#include <algorithm>
#include <boost/geometry.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/multi_point.hpp>
#include <boost/geometry/index/parameters.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/strategies/default_distance_result.hpp>
#include <boost/geometry/strategies/default_strategy.hpp>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#include "pyinterp/eigen.hpp"
#include "pyinterp/geometry/point_traits.hpp"
#include "pyinterp/math/interpolate/kriging.hpp"
#include "pyinterp/math/interpolate/rbf.hpp"
#include "pyinterp/math/interpolate/window_function.hpp"

namespace pyinterp::geometry {

/// Spatial index for point data with various interpolation methods.
///
/// @tparam Point Boost.Geometry point type
/// @tparam Type Value type associated with each point
template <typename Point, std::floating_point Type>
class RTree {
 public:
  using dimension_t = typename boost::geometry::traits::dimension<Point>;
  static_assert(dimension_t::value == 2 || dimension_t::value == 3,
                "Only 2D and 3D points are supported");

  /// Type of point coordinates
  using coordinate_t =
      typename boost::geometry::traits::coordinate_type<Point>::type;

  /// Type of distance function result
  using distance_t =
      typename boost::geometry::default_distance_result<Point, Point>::type;

  /// Type of query results (distance, value)
  using result_t = std::pair<distance_t, Type>;

  /// Value handled by this object (point, value)
  using value_t = std::pair<Point, Type>;

  /// Spatial index type (R*-tree with fanout 16)
  using rtree_t =
      boost::geometry::index::rtree<value_t, boost::geometry::index::rstar<16>>;

  /// Type promotion for mixed coordinate/value arithmetic
  using promotion_t =
      decltype(std::declval<coordinate_t>() + std::declval<Type>());

  /// Default constructor
  RTree() : tree_{std::make_shared<rtree_t>()} {}

  /// Default destructor
  virtual ~RTree() = default;

  /// Default copy constructor
  RTree(const RTree&) = default;

  /// Default copy assignment operator
  auto operator=(const RTree&) -> RTree& = default;

  /// Move constructor
  RTree(RTree&&) noexcept = default;

  /// Move assignment operator
  auto operator=(RTree&&) noexcept -> RTree& = default;

  /// Returns the bounding box containing all stored values.
  /// @return The bounding box or nullopt if the container is empty
  [[nodiscard]] virtual auto bounds() const
      -> std::optional<typename rtree_t::bounds_type> {
    if (empty()) [[unlikely]] {
      return std::nullopt;
    }
    return tree_->bounds();
  }

  /// Returns the number of points in the tree
  [[nodiscard]] constexpr auto size() const noexcept -> size_t {
    return tree_->size();
  }

  /// Query if the container is empty
  [[nodiscard]] constexpr auto empty() const noexcept -> bool {
    return tree_->empty();
  }

  /// Removes all values from the container
  auto clear() -> void { tree_->clear(); }

  /// Rebuild the tree using packing algorithm (erases old data)
  /// @param[in] points Vector of (point, value) pairs
  auto packing(const std::vector<value_t>& points) -> void {
    *tree_ = rtree_t(points);
  }

  /// Insert new data into the search tree
  /// @param value (point, value) pair to insert
  auto insert(const value_t& value) -> void { tree_->insert(value); }

  /// Search for the K nearest neighbors using the given strategy.
  /// @param[in] point Point of interest
  /// @param[in] strategy Strategy to calculate distances
  /// @param[in] k Number of nearest neighbors
  /// @return Vector of (distance, value) pairs
  template <typename Strategy>
  [[nodiscard]] auto query(const Point& point, const Strategy& strategy,
                           const uint32_t k) const -> std::vector<result_t> {
    std::vector<result_t> result;
    result.reserve(k);

    auto query_range = tree_->qbegin(boost::geometry::index::nearest(point, k));
    std::ranges::for_each(query_range, tree_->qend(), [&](const auto& item) {
      result.emplace_back(
          boost::geometry::distance(point, item.first, strategy), item.second);
    });

    return result;
  }

  /// Search for K nearest neighbors with default strategy
  [[nodiscard]] auto query(const Point& point, const uint32_t k) const
      -> std::vector<result_t> {
    return query(point, boost::geometry::default_strategy(), k);
  }

  /// Search for neighbors within a radius using the given strategy.
  /// @param[in] point Point of interest
  /// @param[in] strategy Strategy to calculate distances
  /// @param[in] radius Maximum search distance
  /// @return Vector of (distance, value) pairs within radius
  template <typename Strategy>
  [[nodiscard]] auto query_ball(const Point& point, const Strategy& strategy,
                                const coordinate_t radius) const
      -> std::vector<result_t> {
    std::vector<result_t> result;

    auto satisfies_radius = [&](const auto& item) {
      return boost::geometry::distance(item.first, point) <= radius;
    };

    auto query_range =
        tree_->qbegin(boost::geometry::index::satisfies(satisfies_radius));

    std::ranges::for_each(query_range, tree_->qend(), [&](const auto& item) {
      result.emplace_back(
          boost::geometry::distance(point, item.first, strategy), item.second);
    });

    return result;
  }

  /// Search for neighbors within a radius with default strategy
  [[nodiscard]] auto query_ball(const Point& point,
                                const coordinate_t radius) const
      -> std::vector<result_t> {
    return query_ball(point, boost::geometry::default_strategy(), radius);
  }

  /// Search for K neighbors that surround the query point.
  /// @param[in] point Point of interest
  /// @param[in] strategy Strategy to calculate distances
  /// @param[in] k Number of nearest neighbors
  /// @return Vector of neighbors or empty if point not surrounded
  template <typename Strategy>
  [[nodiscard]] auto query_within(const Point& point, const Strategy& strategy,
                                  const uint32_t k) const
      -> std::vector<result_t> {
    std::vector<result_t> result;
    result.reserve(k);

    boost::geometry::model::multi_point<Point> points;
    points.reserve(k);

    auto query_range = tree_->qbegin(boost::geometry::index::nearest(point, k));
    std::ranges::for_each(query_range, tree_->qend(), [&](const auto& item) {
      points.emplace_back(item.first);
      result.emplace_back(
          boost::geometry::distance(point, item.first, strategy), item.second);
    });

    // Check if point is covered by its neighbors
    if (!boost::geometry::covered_by(
            point, boost::geometry::return_envelope<
                       boost::geometry::model::box<Point>>(points))) {
      return {};
    }
    return result;
  }

  /// Search for K neighbors that surround the query point (default strategy)
  [[nodiscard]] auto query_within(const Point& point, const uint32_t k) const
      -> std::vector<result_t> {
    return query_within(point, boost::geometry::default_strategy(), k);
  }

  /// Get K nearest neighbors, optionally filtered by radius and coverage.
  /// @param[in] point Point of interest
  /// @param[in] strategy Strategy to calculate distances
  /// @param[in] radius Optional maximum search distance
  /// @param[in] k Number of nearest neighbors
  /// @param[in] within If true, ensure point is surrounded by neighbors
  /// @return Vector of (point, value) pairs
  template <typename Strategy>
  [[nodiscard]] auto value(const Point& point, const Strategy& strategy,
                           const std::optional<coordinate_t>& radius,
                           const uint32_t k, const bool within) const
      -> std::vector<value_t> {
    std::vector<value_t> result;
    result.reserve(k);

    auto query_range = tree_->qbegin(boost::geometry::index::nearest(point, k));
    std::ranges::for_each(query_range, tree_->qend(),
                          [&](const auto& item) { result.emplace_back(item); });

    // Remove points outside the radius
    if (radius.has_value()) {
      std::erase_if(result, [&](const auto& item) {
        return boost::geometry::distance(item.first, point, strategy) >
               radius.value();
      });
    }

    // Check if point is surrounded by neighbors
    if (within) {
      boost::geometry::model::multi_point<Point> points;
      points.reserve(result.size());

      std::ranges::for_each(
          result, [&](const auto& item) { points.emplace_back(item.first); });

      if (!boost::geometry::covered_by(
              point, boost::geometry::return_envelope<
                         boost::geometry::model::box<Point>>(points))) {
        return {};
      }
    }
    return result;
  }

  /// Get K nearest neighbors (default strategy)
  /// @param[in] point Point of interest
  /// @param[in] radius Optional maximum search distance
  /// @param[in] k Number of nearest neighbors
  /// @param[in] within If true, ensure point is surrounded by neighbors
  /// @return Vector of (point, value) pairs
  [[nodiscard]] auto value(const Point& point,
                           const std::optional<coordinate_t>& radius,
                           const uint32_t k, const bool within) const
      -> std::vector<value_t> {
    return value(point, boost::geometry::default_strategy(), radius, k, within);
  }

  /// Inverse Distance Weighting interpolation.
  /// @param[in] point Point of interest
  /// @param[in] strategy Strategy to calculate distances
  /// @param[in] radius Maximum search distance
  /// @param[in] k Number of nearest neighbors
  /// @param[in] p Power parameter
  /// @param[in] within If true, ensure point is surrounded
  /// @return Pair of (interpolated value, number of neighbors used)
  template <typename Strategy>
  [[nodiscard]] auto inverse_distance_weighting(
      const Point& point, const Strategy& strategy, const coordinate_t radius,
      const uint32_t k, const uint32_t p, const bool within) const
      -> std::pair<coordinate_t, uint32_t> {
    constexpr coordinate_t epsilon = coordinate_t{1e-6};
    coordinate_t result{0};
    coordinate_t total_weight{0};

    const auto nearest =
        within ? query_within(point, strategy, k) : query(point, strategy, k);
    uint32_t neighbors{0};

    for (const auto& [distance, value] : nearest) {
      // If point coincides with a data point, return that value
      if (distance < epsilon) [[unlikely]] {
        return {static_cast<coordinate_t>(value), k};
      }

      if (distance <= radius) {
        const auto weight =
            static_cast<Type>(Type{1} / std::pow(static_cast<Type>(distance),
                                                 static_cast<Type>(p)));
        total_weight += weight;
        result += value * weight;
        ++neighbors;
      }
    }

    return total_weight != coordinate_t{0}
               ? std::pair{static_cast<coordinate_t>(result / total_weight),
                           neighbors}
               : std::pair{std::numeric_limits<coordinate_t>::quiet_NaN(),
                           uint32_t{0}};
  }

  /// Inverse Distance Weighting interpolation (default strategy)
  /// @param[in] point Point of interest
  /// @param[in] radius Maximum search distance
  /// @param[in] k Number of nearest neighbors
  /// @param[in] p Power parameter
  /// @param[in] within If true, ensure point is surrounded
  /// @return Pair of (interpolated value, number of neighbors used)
  [[nodiscard]] auto inverse_distance_weighting(const Point& point,
                                                const coordinate_t radius,
                                                const uint32_t k,
                                                const uint32_t p,
                                                const bool within) const
      -> std::pair<coordinate_t, uint32_t> {
    return inverse_distance_weighting(
        point, boost::geometry::default_strategy(), radius, k, p, within);
  }

  /// Kriging interpolation.
  /// @param[in] point Point of interest
  /// @param[in] radius Maximum search distance
  /// @param[in] k Number of nearest neighbors
  /// @param[in] within If true, ensure point is surrounded
  /// @param[in] model Kriging model
  /// @return Pair of (interpolated value, number of neighbors used)
  [[nodiscard]] auto kriging(
      const Point& point, const coordinate_t radius, const uint32_t k,
      const bool within,
      const math::interpolate::Kriging<promotion_t>& model) const
      -> std::pair<coordinate_t, uint32_t> {
    const auto [coords, values] =
        within ? nearest_within(point, boost::geometry::default_strategy(),
                                radius, k)
               : nearest(point, boost::geometry::default_strategy(), radius, k);

    if (values.size() == 0) [[unlikely]] {
      return {std::numeric_limits<promotion_t>::quiet_NaN(), uint32_t{0}};
    }

    // Kriging requires 3D coordinates
    if constexpr (dimension_t::value == 3) {
      // 3D case: use coordinates directly, no copy needed
      const Eigen::Vector3<promotion_t> point_3d(
          boost::geometry::get<0>(point), boost::geometry::get<1>(point),
          boost::geometry::get<2>(point));

      return {model(coords, values, point_3d),
              static_cast<uint32_t>(coords.cols())};

    } else if constexpr (dimension_t::value == 2) {
      // 2D case: pad with zeros in z-coordinate
      Eigen::Matrix<promotion_t, 3, Eigen::Dynamic> coords_3d(3, coords.cols());
      coords_3d.template topRows<2>() = coords;
      coords_3d.row(2).setZero();

      const Eigen::Vector3<promotion_t> point_3d(boost::geometry::get<0>(point),
                                                 boost::geometry::get<1>(point),
                                                 promotion_t{0});

      return {model(coords_3d, values, point_3d),
              static_cast<uint32_t>(coords_3d.cols())};
    }
  }

  /// Get K nearest neighbors with their coordinates and values.
  /// @param[in] point Point of interest
  /// @param[in] strategy Strategy to calculate distances
  /// @param[in] radius Maximum search distance
  /// @param[in] k Number of nearest neighbors
  /// @return Tuple of (coordinate matrix, value vector)
  template <typename Strategy>
  [[nodiscard]] auto nearest(const Point& point, const Strategy& strategy,
                             const coordinate_t radius, const uint32_t k) const
      -> std::tuple<Matrix<promotion_t>, Vector<promotion_t>> {
    auto coordinates = Matrix<promotion_t>(dimension_t::value, k);
    auto values = Vector<promotion_t>(k);
    uint32_t count{0};

    auto query_range = tree_->qbegin(boost::geometry::index::nearest(point, k));
    std::ranges::for_each(query_range, tree_->qend(), [&](const auto& item) {
      if (boost::geometry::distance(point, item.first, strategy) <= radius) {
        for (size_t ix = 0; ix < dimension_t::value; ++ix) {
          coordinates(ix, count) = geometry::point::get(item.first, ix);
        }
        values(count++) = item.second;
      }
    });

    coordinates.conservativeResize(dimension_t::value, count);
    values.conservativeResize(count);
    return {coordinates, values};
  }

  /// Get K nearest neighbors (default strategy)
  [[nodiscard]] auto nearest(const Point& point, const coordinate_t radius,
                             const uint32_t k) const
      -> std::tuple<Matrix<promotion_t>, Vector<promotion_t>> {
    return nearest(point, boost::geometry::default_strategy(), radius, k);
  }

  /// Get K nearest neighbors that surround the query point.
  /// @param[in] point Point of interest
  /// @param[in] strategy Strategy to calculate distances
  /// @param[in] radius Maximum search distance
  /// @param[in] k Number of nearest neighbors
  /// @return Tuple of (coordinate matrix, value vector), empty if not
  /// surrounded
  template <typename Strategy>
  [[nodiscard]] auto nearest_within(const Point& point,
                                    const Strategy& strategy,
                                    const coordinate_t radius,
                                    const uint32_t k) const
      -> std::tuple<Matrix<promotion_t>, Vector<promotion_t>> {
    boost::geometry::model::multi_point<Point> points;
    points.reserve(k);

    auto coordinates = Matrix<promotion_t>(dimension_t::value, k);
    auto values = Vector<promotion_t>(k);
    uint32_t count{0};

    auto query_range = tree_->qbegin(boost::geometry::index::nearest(point, k));
    std::ranges::for_each(query_range, tree_->qend(), [&](const auto& item) {
      if (boost::geometry::distance(point, item.first, strategy) <= radius) {
        points.emplace_back(item.first);
        for (size_t ix = 0; ix < dimension_t::value; ++ix) {
          coordinates(ix, count) = geometry::point::get(item.first, ix);
        }
        values(count++) = item.second;
      }
    });

    // Check if point is surrounded by neighbors
    if (!boost::geometry::covered_by(
            point, boost::geometry::return_envelope<
                       boost::geometry::model::box<Point>>(points))) {
      count = 0;
    }

    coordinates.conservativeResize(dimension_t::value, count);
    values.conservativeResize(count);
    return {coordinates, values};
  }

  /// Get K nearest neighbors that surround the query point (default strategy)
  ///
  /// @param[in] point Point of interest
  /// @param[in] radius Maximum search distance
  /// @param[in] k Number of nearest neighbors
  [[nodiscard]] auto nearest_within(const Point& point,
                                    const coordinate_t radius,
                                    const uint32_t k) const
      -> std::tuple<Matrix<promotion_t>, Vector<promotion_t>> {
    return nearest_within(point, boost::geometry::default_strategy(), radius,
                          k);
  }

  /// Radial Basis Function interpolation.
  /// @param[in] point Point of interest
  /// @param[in] strategy Strategy to calculate distances
  /// @param[in] rbf Radial basis function
  /// @param[in] radius Maximum search distance
  /// @param[in] k Number of nearest neighbors
  /// @param[in] within If true, ensure point is surrounded
  /// @return Pair of (interpolated value, number of neighbors used)
  template <typename Strategy>
  [[nodiscard]] auto radial_basis_function(
      const Point& point, const Strategy& strategy,
      const math::interpolate::RBF<promotion_t>& rbf, const coordinate_t radius,
      const uint32_t k, const bool within) const
      -> std::pair<promotion_t, uint32_t> {
    const auto [coordinates, values] =
        within ? nearest_within(point, strategy, radius, k)
               : nearest(point, strategy, radius, k);

    if (values.size() == 0) [[unlikely]] {
      return {std::numeric_limits<promotion_t>::quiet_NaN(), uint32_t{0}};
    }

    auto xi = Eigen::Matrix<promotion_t, dimension_t::value, 1>();
    for (size_t ix = 0; ix < dimension_t::value; ++ix) {
      xi(ix, 0) = geometry::point::get(point, ix);
    }

    const auto interpolated = rbf.interpolate(coordinates, values, xi);
    return {interpolated(0), static_cast<uint32_t>(values.size())};
  }

  /// Radial Basis Function interpolation (default strategy)
  [[nodiscard]] auto radial_basis_function(
      const Point& point, const math::interpolate::RBF<promotion_t>& rbf,
      const coordinate_t radius, const uint32_t k, const bool within) const
      -> std::pair<promotion_t, uint32_t> {
    return radial_basis_function(point, boost::geometry::default_strategy(),
                                 rbf, radius, k, within);
  }

  /// Window Function interpolation.
  /// @param[in] point Point of interest
  /// @param[in] strategy Strategy to calculate distances
  /// @param[in] wf Window function
  /// @param[in] arg Window function argument
  /// @param[in] radius Maximum search distance
  /// @param[in] k Number of nearest neighbors
  /// @param[in] within If true, ensure point is surrounded
  /// @return Pair of (interpolated value, number of neighbors used)
  template <typename Strategy>
  [[nodiscard]] auto window_function(
      const Point& point, const Strategy& strategy,
      const math::interpolate::WindowFunction<coordinate_t>& wf,
      const coordinate_t arg, const coordinate_t radius, const uint32_t k,
      const bool within) const -> std::pair<coordinate_t, uint32_t> {
    coordinate_t result{0};
    coordinate_t total_weight{0};

    const auto nearest =
        within ? query_within(point, strategy, k) : query(point, strategy, k);
    uint32_t neighbors{0};

    // Get distance of furthest neighbor if radius not specified
    const coordinate_t furthest_neighbor =
        radius == std::numeric_limits<coordinate_t>::max()
            ? (nearest.empty()
                   ? coordinate_t{0}
                   : static_cast<coordinate_t>(nearest.back().first))
            : radius;

    for (const auto& [distance, value] : nearest) {
      if (distance > radius) {
        break;
      }

      const auto weight =
          wf(static_cast<coordinate_t>(distance), furthest_neighbor, arg);
      total_weight += weight;
      result += value * weight;
      ++neighbors;
    }

    return total_weight != coordinate_t{0}
               ? std::pair{static_cast<coordinate_t>(result / total_weight),
                           neighbors}
               : std::pair{std::numeric_limits<coordinate_t>::quiet_NaN(),
                           uint32_t{0}};
  }

  /// Window Function interpolation (default strategy)
  /// @param[in] point Point of interest
  /// @param[in] wf Window function
  /// @param[in] arg Window function argument
  /// @param[in] radius Maximum search distance
  /// @param[in] k Number of nearest neighbors
  /// @param[in] within If true, ensure point is surrounded
  /// @return Pair of (interpolated value, number of neighbors used)
  [[nodiscard]] auto window_function(
      const Point& point,
      const math::interpolate::WindowFunction<coordinate_t>& wf,
      const coordinate_t arg, const coordinate_t radius, const uint32_t k,
      const bool within) const -> std::pair<coordinate_t, uint32_t> {
    return window_function(point, boost::geometry::default_strategy(), wf, arg,
                           radius, k, within);
  }

 protected:
  /// Spatial index (R*-tree)
  std::shared_ptr<rtree_t> tree_;
};

}  // namespace pyinterp::geometry

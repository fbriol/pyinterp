// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#pragma once

#include <Eigen/Core>
#include <concepts>
#include <cstdint>
#include <limits>
#include <optional>
#include <tuple>
#include <vector>

#include "pyinterp/eigen.hpp"
#include "pyinterp/geodetic/coordinates.hpp"
#include "pyinterp/geodetic/spheroid.hpp"
#include "pyinterp/geometry/point.hpp"
#include "pyinterp/geometry/rtree.hpp"
#include "pyinterp/math/interpolate/rbf.hpp"
#include "pyinterp/math/interpolate/window_function.hpp"
#include "pyinterp/serialization_buffer.hpp"

namespace pyinterp::pybind {

/// 3D bounding box for Cartesian coordinates
/// @tparam T Coordinate type
template <std::floating_point T>
struct Box3D {
  std::array<T, 3> min_corner;
  std::array<T, 3> max_corner;
};

/// RTree spatial index for 3D points
///
/// This class provides spatial indexing capabilities for 3D coordinates
/// using an R-tree data structure. It supports two coordinate systems:
/// - ECEF (Earth-Centered, Earth-Fixed): Cartesian coordinates in meters
/// - Geodetic: (longitude, latitude, altitude) in degrees/degrees/meters
///
/// The coordinate system is selected at construction time via the `spheroid`
/// parameter. If a spheroid is provided, input coordinates are assumed to be
/// geodetic and will be converted to ECEF internally. If no spheroid is given,
/// input coordinates are assumed to be in ECEF.
template <std::floating_point T>
class RTree3D : public geometry::RTree<geometry::ECEF<T>, T> {
 public:
  /// Scalar type for coordinates and values
  using value_type = T;

  /// Point type (3D Cartesian internally)
  using point_t = geometry::ECEF<T>;

  /// Base class type
  using base_t = geometry::RTree<point_t, T>;

  /// Coordinate type
  using coordinate_t = typename base_t::coordinate_t;

  /// Distance type between points
  using distance_t = typename base_t::distance_t;

  /// Query result type (distance, value, point)
  using result_t = typename base_t::result_t;

  /// Promoted type for arithmetic
  using promotion_t = typename base_t::promotion_t;

  /// Coordinate matrix type: (n, 3) or (n, 2)
  using CoordinateMatrix = RowMajorMatrix<T>;

  /// Value vector type
  using ValueVector = Vector<T>;

  /// Default constructor (WGS84 spheroid)
  RTree3D() : spheroid_(geodetic::Spheroid()) {}

  /// Construct with specified coordinate system
  /// @param[in] spheroid Optional spheroid used to convert geodetic inputs to
  /// ECEF; if std::nullopt, inputs are assumed already ECEF.
  explicit RTree3D(const std::optional<geodetic::Spheroid>& spheroid)
      : spheroid_(spheroid) {}

  /// Get the spheroid
  [[nodiscard]] constexpr auto spheroid() const noexcept
      -> const std::optional<geodetic::Spheroid>& {
    return spheroid_;
  }

  /// Get the 3D bounding box of all stored points
  /// @return Optional box containing all points, or nullopt if empty
  [[nodiscard]] auto bounds() const -> std::optional<Box3D<T>>;

  /// Bulk-load points using STR packing algorithm
  ///
  /// @param[in] coordinates Matrix of shape (n, 3) or (n, 2) containing
  /// coordinates.
  /// For ECEF: (x, y, z) in meters
  /// For geodetic: (lon, lat, alt) in degrees/degrees/meters
  /// If shape is (n, 2), the third coordinate is assumed to be zero.
  /// @param[in] values Vector of size n containing values at each point
  void packing(const Eigen::Ref<const CoordinateMatrix>& coordinates,
               const Eigen::Ref<const ValueVector>& values);

  /// Insert points into the tree
  ///
  /// @param[in] coordinates Matrix of shape (n, 3) or (n, 2) containing
  /// coordinates
  /// @param[in] values Vector of size n containing values at each point
  void insert(const Eigen::Ref<const CoordinateMatrix>& coordinates,
              const Eigen::Ref<const ValueVector>& values);

  /// Query k-nearest neighbors for multiple points
  ///
  /// @param[in] coordinates Query coordinates, shape (n, 3) or (n, 2)
  /// @param[in] k Number of neighbors to find
  /// @param[in] check Type of boundary verification to apply
  /// @param[in] num_threads Number of threads (0 = auto)
  /// @return Tuple of (distances, values) matrices [n_points x k]
  [[nodiscard]] auto query(
      const Eigen::Ref<const CoordinateMatrix>& coordinates, uint32_t k,
      const geometry::BoundaryCheck check, size_t num_threads) const
      -> std::tuple<Matrix<distance_t>, Matrix<promotion_t>>;

  /// Inverse distance weighting interpolation
  ///
  /// @param[in] coordinates Query coordinates, shape (n, 3) or (n, 2)
  /// @param[in] radius Optional search radius (meters)
  /// @param[in] k Number of neighbors to use
  /// @param[in] p Power parameter (typically 2)
  /// @param[in] check Type of boundary verification to apply
  /// @param[in] num_threads Number of threads (0 = auto)
  /// @return Tuple of (interpolated values, neighbor counts)
  [[nodiscard]] auto inverse_distance_weighting(
      const Eigen::Ref<const CoordinateMatrix>& coordinates,
      const std::optional<coordinate_t>& radius, uint32_t k, uint32_t p,
      const geometry::BoundaryCheck check, size_t num_threads) const
      -> std::tuple<ValueVector, Vector<uint32_t>>;

  /// Kriging interpolation
  ///
  /// @param[in] coordinates Query coordinates, shape (n, 3) or (n, 2)
  /// @param[in] radius Optional search radius (meters)
  /// @param[in] k Number of neighbors to use
  /// @param[in] covariance_model Covariance function
  /// @param[in] drift_function Optional drift function
  /// @param[in] sigma Sill parameter
  /// @param[in] lambda Range parameter
  /// @param[in] nugget Nugget effect parameter
  /// @param[in] check Type of boundary verification to apply
  /// @param[in] num_threads Number of threads (0 = auto)
  /// @return Tuple of (interpolated values, neighbor counts)
  [[nodiscard]] auto kriging(
      const Eigen::Ref<const CoordinateMatrix>& coordinates,
      const std::optional<coordinate_t>& radius, const uint32_t k,
      const math::interpolate::CovarianceFunction covariance_model,
      const std::optional<math::interpolate::DriftFunction>& drift_function,
      const coordinate_t sigma, const coordinate_t lambda,
      const coordinate_t nugget, const geometry::BoundaryCheck check,
      const size_t num_threads) const
      -> std::tuple<ValueVector, Vector<uint32_t>>;

  /// Radial basis function interpolation
  ///
  /// @param[in] coordinates Query coordinates, shape (n, 3) or (n, 2)
  /// @param[in] radius Optional search radius (meters)
  /// @param[in] k Number of neighbors to use
  /// @param[in] rbf Radial basis function type
  /// @param[in] epsilon Optional shape parameter
  /// @param[in] smooth Smoothing parameter
  /// @param[in] check Type of boundary verification to apply
  /// @param[in] num_threads Number of threads (0 = auto)
  /// @return Tuple of (interpolated values, neighbor counts)
  [[nodiscard]] auto radial_basis_function(
      const Eigen::Ref<const CoordinateMatrix>& coordinates,
      const std::optional<T>& radius, uint32_t k,
      math::interpolate::RadialBasisFunction rbf,
      const std::optional<T>& epsilon, T smooth,
      const geometry::BoundaryCheck check, size_t num_threads) const
      -> std::tuple<ValueVector, Vector<uint32_t>>;

  /// Window function based interpolation
  ///
  /// @param coordinates Query coordinates, shape (n, 3) or (n, 2)
  /// @param radius Optional search radius (meters)
  /// @param k Number of neighbors to use
  /// @param wf Window function type
  /// @param arg Optional window function argument
  /// @param check Type of boundary verification to apply
  /// @param num_threads Number of threads (0 = auto)
  /// @return Tuple of (interpolated values, neighbor counts)
  [[nodiscard]] auto window_function(
      const Eigen::Ref<const CoordinateMatrix>& coordinates,
      const std::optional<T>& radius, uint32_t k,
      math::interpolate::window::Function wf, const std::optional<double>& arg,
      const geometry::BoundaryCheck check, size_t num_threads) const
      -> std::tuple<ValueVector, Vector<uint32_t>>;

  /// Serialize the RTree3D state
  /// @return Serialized byte buffer
  [[nodiscard]] auto serialize() const -> std::vector<std::byte>;

  /// Deserialize an RTree3D from a byte buffer
  /// @param buffer Serialized data
  /// @return Deserialized RTree3D instance
  [[nodiscard]] static auto deserialize(std::span<const std::byte> buffer)
      -> RTree3D;

 private:
  /// Spheroid for geodetic calculations
  std::optional<geodetic::Spheroid> spheroid_;

  /// Convert input coordinates to internal ECEF representation
  /// @param coordinates Input matrix (n, 3) or (n, 2)
  /// @param row Row index
  /// @return 3D point in internal coordinate system
  [[nodiscard]] auto to_internal_point(
      const Eigen::Ref<const CoordinateMatrix>& coordinates,
      Eigen::Index row) const -> point_t;

  /// Convert geodetic (lon, lat, alt) to ECEF (x, y, z)
  [[nodiscard]] auto geodetic_to_ecef(T lon, T lat, T alt) const -> point_t;

  /// Validate coordinate matrix dimensions
  static void validate_coordinates(
      const Eigen::Ref<const CoordinateMatrix>& coordinates,
      const Eigen::Ref<const ValueVector>& values);

  static void validate_coordinates(
      const Eigen::Ref<const CoordinateMatrix>& coordinates);

  /// Helper: perform batch query operation
  template <typename QueryFunc>
  [[nodiscard]] auto batch_query(
      const Eigen::Ref<const CoordinateMatrix>& coordinates, uint32_t k,
      size_t num_threads, QueryFunc&& query_func) const
      -> std::tuple<Matrix<distance_t>, Matrix<promotion_t>>;

  /// Helper: perform batch interpolation operation
  template <typename InterpolateFunc>
  [[nodiscard]] auto batch_interpolate(
      const Eigen::Ref<const CoordinateMatrix>& coordinates, size_t num_threads,
      InterpolateFunc&& interpolate_func) const
      -> std::tuple<ValueVector, Vector<uint32_t>>;
};

// ////////////////////////////////////////////////////////////////////////////
// Implementation details
// ////////////////////////////////////////////////////////////////////////////

template <std::floating_point T>
void RTree3D<T>::validate_coordinates(
    const Eigen::Ref<const CoordinateMatrix>& coordinates,
    const Eigen::Ref<const ValueVector>& values) {
  validate_coordinates(coordinates);
  if (coordinates.rows() != values.size()) {
    throw std::invalid_argument(
        "Number of coordinates must match number of values");
  }
}

// ////////////////////////////////////////////////////////////////////////////

template <std::floating_point T>
void RTree3D<T>::validate_coordinates(
    const Eigen::Ref<const CoordinateMatrix>& coordinates) {
  if (coordinates.cols() != 2 && coordinates.cols() != 3) {
    throw std::invalid_argument("Coordinates must have shape (n, 2) or (n, 3)");
  }
}

// ////////////////////////////////////////////////////////////////////////////

template <std::floating_point T>
auto RTree3D<T>::geodetic_to_ecef(T lon, T lat, T alt) const -> point_t {
  auto transformer = geodetic::Coordinates(*spheroid_);
  return transformer.lla_to_ecef<T>(geometry::LLA<T>{lon, lat, alt});
}

// ////////////////////////////////////////////////////////////////////////////

template <std::floating_point T>
auto RTree3D<T>::to_internal_point(
    const Eigen::Ref<const CoordinateMatrix>& coordinates,
    Eigen::Index row) const -> point_t {
  const auto c0 = coordinates(row, 0);
  const auto c1 = coordinates(row, 1);
  const auto c2 = coordinates.cols() == 3 ? coordinates(row, 2) : T{0};

  if (spheroid_ == std::nullopt) {
    // Already in ECEF, use directly
    return point_t{c0, c1, c2};
  }
  // Convert from geodetic (lon, lat, alt) to ECEF
  return geodetic_to_ecef(c0, c1, c2);
}

// ////////////////////////////////////////////////////////////////////////////

template <std::floating_point T>
auto RTree3D<T>::bounds() const -> std::optional<Box3D<T>> {
  if (base_t::empty()) {
    return std::nullopt;
  }

  auto box = base_t::bounds();
  return Box3D<T>{
      {box.min_corner().template get<0>(), box.min_corner().template get<1>(),
       box.min_corner().template get<2>()},
      {box.max_corner().template get<0>(), box.max_corner().template get<1>(),
       box.max_corner().template get<2>()}};
}

// ////////////////////////////////////////////////////////////////////////////

template <std::floating_point T>
void RTree3D<T>::packing(const Eigen::Ref<const CoordinateMatrix>& coordinates,
                         const Eigen::Ref<const ValueVector>& values) {
  validate_coordinates(coordinates, values);

  std::vector<std::pair<point_t, T>> items;
  items.reserve(static_cast<size_t>(coordinates.rows()));

  for (int64_t ix = 0; ix < coordinates.rows(); ++ix) {
    items.emplace_back(to_internal_point(coordinates, ix), values[ix]);
  }

  base_t::packing(items);
}

// ////////////////////////////////////////////////////////////////////////////

template <std::floating_point T>
void RTree3D<T>::insert(const Eigen::Ref<const CoordinateMatrix>& coordinates,
                        const Eigen::Ref<const ValueVector>& values) {
  validate_coordinates(coordinates, values);

  for (int64_t ix = 0; ix < coordinates.rows(); ++ix) {
    base_t::insert(to_internal_point(coordinates, ix), values[ix]);
  }
}

// ////////////////////////////////////////////////////////////////////////////

template <std::floating_point T>
template <typename QueryFunc>
auto RTree3D<T>::batch_query(
    const Eigen::Ref<const CoordinateMatrix>& coordinates, uint32_t k,
    size_t num_threads, QueryFunc&& query_func) const
    -> std::tuple<Matrix<distance_t>, Matrix<promotion_t>> {
  validate_coordinates(coordinates);

  const auto n = coordinates.rows();
  Matrix<distance_t> distances(n, k);
  Matrix<promotion_t> values(n, k);

  // Initialize with sentinel values
  distances.setConstant(std::numeric_limits<distance_t>::quiet_NaN());
  values.setConstant(std::numeric_limits<promotion_t>::quiet_NaN());

  parallel_for(
      static_cast<size_t>(n),
      [&](size_t start, size_t end) {
        for (size_t idx = start; idx < end; ++idx) {
          const auto ix = static_cast<int64_t>(idx);
          auto point = to_internal_point(coordinates, ix);
          auto results = query_func(point, k);

          for (size_t jx = 0; jx < results.size() && jx < k; ++jx) {
            distances(ix, static_cast<int64_t>(jx)) = std::get<0>(results[jx]);
            values(ix, static_cast<int64_t>(jx)) = std::get<1>(results[jx]);
          }
        }
      },
      num_threads);

  return {std::move(distances), std::move(values)};
}

// //////////////////////////////////////////////////////////////////////////

template <std::floating_point T>
template <typename InterpolateFunc>
auto RTree3D<T>::batch_interpolate(
    const Eigen::Ref<const CoordinateMatrix>& coordinates, size_t num_threads,
    InterpolateFunc&& interpolate_func) const
    -> std::tuple<ValueVector, Vector<uint32_t>> {
  validate_coordinates(coordinates);

  const auto n = coordinates.rows();
  ValueVector values(n);
  Vector<uint32_t> counts(n);

  values.setConstant(std::numeric_limits<promotion_t>::quiet_NaN());
  counts.setZero();

  parallel_for(
      static_cast<size_t>(n),
      [&](size_t start, size_t end) {
        for (size_t idx = start; idx < end; ++idx) {
          const auto ix = static_cast<int64_t>(idx);
          auto point = to_internal_point(coordinates, ix);
          auto [value, count] = interpolate_func(point);
          values[ix] = value;
          counts[ix] = count;
        }
      },
      num_threads);

  return {std::move(values), std::move(counts)};
}

// //////////////////////////////////////////////////////////////////////////

template <std::floating_point T>
auto RTree3D<T>::query(const Eigen::Ref<const CoordinateMatrix>& coordinates,
                       uint32_t k, geometry::BoundaryCheck check,
                       size_t num_threads) const
    -> std::tuple<Matrix<distance_t>, Matrix<promotion_t>> {
  return batch_query(coordinates, k, num_threads,
                     [this, check](const point_t& pt, uint32_t neighbors) {
                       return base_t::query(pt, neighbors, check);
                     });
}

// //////////////////////////////////////////////////////////////////////////

template <std::floating_point T>
auto RTree3D<T>::inverse_distance_weighting(
    const Eigen::Ref<const CoordinateMatrix>& coordinates,
    const std::optional<coordinate_t>& radius, uint32_t k, uint32_t p,
    const geometry::BoundaryCheck check, size_t num_threads) const
    -> std::tuple<ValueVector, Vector<uint32_t>> {
  auto resolved_radius = radius.has_value()
                             ? radius.value()
                             : std::numeric_limits<coordinate_t>::max();
  return batch_interpolate(
      coordinates, num_threads,
      [this, &resolved_radius, k, p,
       check](const point_t& pt) -> std::pair<promotion_t, uint32_t> {
        return base_t::inverse_distance_weighting(pt, resolved_radius, k, p,
                                                  check);
      });
}

// //////////////////////////////////////////////////////////////////////////

template <std::floating_point T>
auto RTree3D<T>::kriging(
    const Eigen::Ref<const CoordinateMatrix>& coordinates,
    const std::optional<coordinate_t>& radius, const uint32_t k,
    const math::interpolate::CovarianceFunction covariance_model,
    const std::optional<math::interpolate::DriftFunction>& drift_function,
    const coordinate_t sigma, const coordinate_t lambda,
    const coordinate_t nugget, const geometry::BoundaryCheck check,
    const size_t num_threads) const
    -> std::tuple<ValueVector, Vector<uint32_t>> {
  auto resolved_radius = radius.has_value()
                             ? radius.value()
                             : std::numeric_limits<coordinate_t>::max();
    auto model = math::interpolate::Kriging<promotion_t>(
        sigma, lambda, nugget, covariance_model, drift_function);
  return batch_interpolate(
        coordinates, num_threads,
        [this, &model, &resolved_radius, k,
         check](const point_t& pt) -> std::pair<promotion_t, uint32_t> {
          return base_t::kriging(pt, resolved_radius, k, check, model);
        });
}

// //////////////////////////////////////////////////////////////////////////

template <std::floating_point T>
auto RTree3D<T>::radial_basis_function(
    const Eigen::Ref<const CoordinateMatrix>& coordinates,
    const std::optional<T>& radius, uint32_t k,
    math::interpolate::RadialBasisFunction rbf, const std::optional<T>& epsilon,
    T smooth, const geometry::BoundaryCheck check, size_t num_threads) const
    -> std::tuple<ValueVector, Vector<uint32_t>> {
  auto resolved_radius =
      radius.has_value() ? radius.value() : std::numeric_limits<T>::max();
  auto resolved_epsilon = epsilon.has_value()
                              ? static_cast<promotion_t>(epsilon.value())
                              : std::numeric_limits<promotion_t>::quiet_NaN();
  auto model =
      math::interpolate::RBF<promotion_t>(resolved_epsilon, smooth, rbf);
  return batch_interpolate(
      coordinates, num_threads,
      [this, &model, &resolved_radius, k,
       check](const point_t& pt) -> std::pair<promotion_t, uint32_t> {
        return base_t::radial_basis_function(pt, model, resolved_radius, k,
                                             check);
      });
}

// //////////////////////////////////////////////////////////////////////////

template <std::floating_point T>
auto RTree3D<T>::window_function(
    const Eigen::Ref<const CoordinateMatrix>& coordinates,
    const std::optional<T>& radius, uint32_t k,
    math::interpolate::window::Function wf, const std::optional<double>& arg,
    const geometry::BoundaryCheck check, size_t num_threads) const
    -> std::tuple<ValueVector, Vector<uint32_t>> {
  auto resolved_arg = arg.has_value() ? arg.value() : 0.0;
  auto resolved_radius =
      radius.has_value() ? radius.value() : std::numeric_limits<T>::max();
  auto model = math::interpolate::WindowFunction<coordinate_t>(wf);
  return batch_interpolate(
      coordinates, num_threads,
      [this, &model, &resolved_radius, resolved_arg, k,
       check](const point_t& pt) -> std::pair<promotion_t, uint32_t> {
        return base_t::window_function(pt, model, resolved_arg, resolved_radius,
                                       k, check);
      });
}

}  // namespace pyinterp::pybind
// Copyright (c) 2026 CNES.
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#pragma once

#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/SparseCore>
#include <Eigen/SparseLU>
#include <concepts>
#include <cstdint>
#include <limits>
#include <ranges>
#include <vector>

#include "pyinterp/eigen.hpp"
#include "pyinterp/math/interpolate/univariate/cspline_base.hpp"

namespace pyinterp::math::interpolate::univariate {

/// @brief Cubic spline interpolation with not-a-knot end conditions.
/// Not-a-knot conditions force the third derivative to be continuous at the
/// second and second-to-last interior points, resulting in a more natural
/// curve.
/// @tparam T Data type
template <std::floating_point T>
class CSplineNotAKnot : public CSplineBase<T> {
 public:
  using CSplineBase<T>::operator();
  using CSplineBase<T>::derivative;

  /// @brief Triplet type for sparse matrix construction
  using Triplet = Eigen::Triplet<T>;

  /// @brief Index type for the sparse matrix
  using StorageIndex = typename Eigen::SparseMatrix<T>::StorageIndex;

  /// @brief Default constructor.
  CSplineNotAKnot() : CSplineBase<T>(), triplets_{} {}

  /// @brief Minimum number of data points required is 4.
  /// @return Minimum number of data points required
  [[nodiscard]] constexpr auto min_size() const -> int64_t final { return 4; }

 protected:
  /// @brief Compute the spline coefficients (i.e. the first derivatives at the
  /// data points) by solving an n×n system with not-a-knot boundary conditions.
  /// @param[in] xa X-coordinates of the data points.
  /// @param[in] ya Y-coordinates of the data points.
  /// @return True if coefficients computed successfully
  [[nodiscard]] auto compute_coefficients(const Vector<T>& xa,
                                          const Vector<T>& ya) -> bool final;

 private:
  /// @brief Member variable to avoid reallocations in repeated calls to
  /// compute_coefficients
  std::vector<Triplet> triplets_{};
};

// ============================================================================
// Implementation
// ============================================================================

template <std::floating_point T>
auto CSplineNotAKnot<T>::compute_coefficients(const Vector<T>& xa,
                                              const Vector<T>& ya) -> bool {
  if (!CSplineBase<T>::compute_coefficients(xa, ya)) [[unlikely]] {
    return false;
  }

  const auto size = xa.size();
  const auto size_m1 = size - 1;
  const auto size_m2 = size - 2;

  // Use tolerance for numerical stability
  constexpr T epsilon = std::numeric_limits<T>::epsilon() * T{10};

  // Compute step sizes: h_i = x[i+1] - x[i] for i = 0 ... n-2.
  // Vectorized computation for better performance
  const Vector<T> h = xa.tail(size_m1) - xa.head(size_m1);

  // Check for duplicate x values
  if ((h.array().abs() < epsilon).any()) [[unlikely]] {
    return false;  // Two consecutive x values are too close.
  }

  // The not-a-knot boundary conditions create a banded matrix with bandwidth 3
  this->b_.resize(size);
  this->x_.resize(size);

  // Not-a-knot boundary conditions result in b[0] = b[n-1] = 0
  this->b_(0) = T{0};
  this->b_(size_m1) = T{0};

  // Compute divided differences: delta_i = (y[i+1] - y[i]) / h[i]
  const auto delta = (ya.tail(size_m1) - ya.head(size_m1)).array() / h.array();

  // Compute interior values: b_i = 3 * (delta_i - delta_{i-1})
  this->b_.segment(1, size_m2) =
      T{3} * (delta.tail(size_m2) - delta.head(size_m2));

  // Build the sparse system matrix A
  triplets_.clear();
  triplets_.reserve(3 * size);

  // Row 0: Left not-a-knot condition
  // Forces third derivative continuity at x[1]
  triplets_.emplace_back(0, 0, -h(1));
  triplets_.emplace_back(0, 1, h(0) + h(1));
  triplets_.emplace_back(0, 2, -h(0));

  // Rows 1 to n-2: Standard cubic spline tridiagonal equations
  for (const auto i : std::views::iota(int64_t{1}, size_m1)) {
    const auto idx = static_cast<StorageIndex>(i);
    const auto idx_m1 = static_cast<StorageIndex>(i - 1);
    const auto idx_p1 = static_cast<StorageIndex>(i + 1);

    triplets_.emplace_back(idx, idx_m1, h(i - 1));
    triplets_.emplace_back(idx, idx, T{2} * (h(i - 1) + h(i)));
    triplets_.emplace_back(idx, idx_p1, h(i));
  }

  // Row n-1: Right not-a-knot condition
  // Forces third derivative continuity at x[n-2]
  const T h_nm2 = h(size_m2);
  const T h_nm3 = h(size - 3);
  triplets_.emplace_back(size_m1, size - 3, h_nm2);
  triplets_.emplace_back(size_m1, size_m2, -(h_nm2 + h_nm3));
  triplets_.emplace_back(size_m1, size_m1, h_nm3);

  // Solve the sparse linear system Ax = b
  Eigen::SparseMatrix<T> A_sparse(size, size);
  A_sparse.setFromTriplets(triplets_.begin(), triplets_.end());

  Eigen::SparseLU<Eigen::SparseMatrix<T>> solver;
  solver.compute(A_sparse);

  if (solver.info() != Eigen::Success) [[unlikely]] {
    return false;
  }

  this->x_ = solver.solve(this->b_);

  return solver.info() == Eigen::Success;
}

}  // namespace pyinterp::math::interpolate::univariate

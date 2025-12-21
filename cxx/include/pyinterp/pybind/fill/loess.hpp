// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#pragma once

#include <nanobind/nanobind.h>

#include <cmath>
#include <concepts>
#include <cstdint>
#include <vector>

#include "pyinterp/eigen.hpp"
#include "pyinterp/parallel_for.hpp"
#include "pyinterp/pybind/config/fill.hpp"
#include "pyinterp/pybind/fill/helpers.hpp"

namespace pyinterp::pybind::fill {

/// Supported floating-point types for LOESS operations.
template <typename T>
concept LoessScalar = std::floating_point<T>;

namespace detail {

/// Tri-cube weight function: w(d) = (1 - |d|³)³ for |d| ≤ 1, else 0.
template <LoessScalar T>
[[nodiscard]] constexpr auto tricube_weight(T distance) noexcept -> T {
  if (distance > T{1}) {
    return T{0};
  }
  constexpr T power = T{3};
  const auto d_cubed = std::pow(distance, power);
  return std::pow(T{1} - d_cubed, power);
}

/// Determines if a value should be processed based on its defined status.
template <LoessScalar T>
[[nodiscard]] constexpr auto should_process(
    T value, config::fill::LoessValueType value_type) noexcept -> bool {
  const bool is_undefined = std::isnan(value);
  switch (value_type) {
    case config::fill::LoessValueType::kAll:
      return true;
    case config::fill::LoessValueType::kDefined:
      return !is_undefined;
    case config::fill::LoessValueType::kUndefined:
      return is_undefined;
  }
  std::unreachable();
}

/// Thread-local workspace for LOESS computation.
struct LoessWorkspace {
  std::vector<std::int64_t> x_frame;
  std::vector<std::int64_t> y_frame;

  LoessWorkspace(std::uint32_t nx, std::uint32_t ny)
      : x_frame(2 * static_cast<std::size_t>(nx) + 1),
        y_frame(2 * static_cast<std::size_t>(ny) + 1) {}
};

/// Compute LOESS value for a single point.
template <LoessScalar T>
[[nodiscard]] auto loess_point(const RowMajorMatrix<T>& data,
                               const config::fill::Loess& config,
                               const LoessWorkspace& workspace,
                               const int64_t ix, const int64_t iy) -> T {
  const auto z = data(ix, iy);

  if (!should_process(z, config.value_type())) {
    return z;
  }

  // Accumulate weighted values
  T weighted_sum{0};
  T weight_sum{0};

  for (const auto wx : workspace.x_frame) {
    for (const auto wy : workspace.y_frame) {
      const auto zi = data(wx, wy);

      if (!std::isnan(zi)) {
        // Compute normalized distance
        const auto dx = static_cast<T>(wx - ix) / static_cast<T>(config.nx());
        const auto dy = static_cast<T>(wy - iy) / static_cast<T>(config.ny());
        const auto distance = std::sqrt(dx * dx + dy * dy);

        // Apply tri-cube weight
        const auto weight = tricube_weight(distance);
        weighted_sum += weight * zi;
        weight_sum += weight;
      }
    }
  }

  return weight_sum != T{0} ? weighted_sum / weight_sum : z;
}

/// Process a single row (x-slice) of the matrix.
template <LoessScalar T>
void process_row(const RowMajorMatrix<T>& input, RowMajorMatrix<T>& output,
                 const config::fill::Loess& config, LoessWorkspace& workspace,
                 const int64_t ix) {
  const auto num_rows = input.rows();
  const auto num_cols = input.cols();

  // Build frame indices for x-axis
  frame_index(ix, num_rows, config.is_periodic(), workspace.x_frame);

  // Process all columns for this row
  for (int64_t iy = 0; iy < num_cols; ++iy) {
    // Build frame indices for y-axis (never circular)
    frame_index(iy, num_cols, /*is_angle=*/false, workspace.y_frame);

    output(ix, iy) = loess_point(input, config, workspace, ix, iy);
  }
}

/// Compute zonal average (mean of all defined values).
template <LoessScalar T>
[[nodiscard]] auto compute_zonal_average(const RowMajorMatrix<T>& data) -> T {
  const auto valid_mask = data.array().isFinite();
  const auto count = valid_mask.count();

  if (count == 0) {
    return T{0};
  }

  const auto sum = valid_mask.select(data.array(), T{0}).sum();
  return sum / static_cast<T>(count);
}

/// Apply initial guess to undefined values.
template <LoessScalar T>
void apply_first_guess(RowMajorMatrix<T>& data,
                       config::fill::FirstGuess first_guess) {
  const T fill_value = (first_guess == config::fill::FirstGuess::kZonalAverage)
                           ? compute_zonal_average(data)
                           : T{0};

  data.array() = data.array().isNaN().select(fill_value, data.array());
}

/// Compute maximum absolute difference between two matrices.
template <LoessScalar T>
[[nodiscard]] auto compute_max_difference(const RowMajorMatrix<T>& current,
                                          const RowMajorMatrix<T>& previous)
    -> T {
  const auto diff = (current.array() - previous.array()).abs();
  const auto valid_mask = !diff.isNaN();

  if (valid_mask.count() == 0) {
    return T{0};
  }

  return valid_mask.select(diff, T{0}).maxCoeff();
}

/// Single-pass LOESS processing.
template <LoessScalar T>
void loess_single_pass(const RowMajorMatrix<T>& input,
                       RowMajorMatrix<T>& output,
                       const config::fill::Loess& config) {
  parallel_for(
      input.rows(),
      [&](std::int64_t start, std::int64_t end) {
        LoessWorkspace workspace(config.nx(), config.ny());
        for (int64_t ix = start; ix < end; ++ix) {
          process_row(input, output, config, workspace, ix);
        }
      },
      config.num_threads());
}

}  // namespace detail

/// Fills undefined values using locally weighted regression (LOESS).
///
/// The weight function is the tri-cube: w(d) = (1 - |d|³)³
///
/// @tparam T Floating-point scalar type
/// @param data Input matrix to process
/// @param nx Half-window size along x-axis (rows)
/// @param ny Half-window size along y-axis (columns)
/// @param value_type Which values to process
/// @param config LOESS configuration
/// @return New matrix with processed values
template <LoessScalar T>
[[nodiscard]] auto loess(const RowMajorMatrix<T>& data,
                         const config::fill::Loess& config)
    -> RowMajorMatrix<T> {
  RowMajorMatrix<T> result(data.rows(), data.cols());

  // Single-pass for non-undefined processing or single iteration
  if (config.value_type() != config::fill::LoessValueType::kUndefined ||
      config.max_iterations() <= 1) {
    detail::loess_single_pass(data, result, config);
    return result;
  }

  // Iterative filling for undefined values
  result = data;
  detail::apply_first_guess(result, config.first_guess());

  RowMajorMatrix<T> previous(data.rows(), data.cols());

  for (uint32_t iter = 0; iter < config.max_iterations(); ++iter) {
    previous = result;

    detail::loess_single_pass(previous, result, config);

    // Check convergence
    if (detail::compute_max_difference(result, previous) <
        static_cast<T>(config.epsilon())) {
      break;
    }
  }

  return result;
}

/// Bind LOESS functions to Python module.
/// @param[in,out] m Python module
void bind_loess(nanobind::module_& m);

}  // namespace pyinterp::pybind::fill

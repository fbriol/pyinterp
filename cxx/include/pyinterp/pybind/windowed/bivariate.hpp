#pragma once

#include <nanobind/eigen/dense.h>
#include <nanobind/nanobind.h>

#include "pyinterp/broadcast.hpp"
#include "pyinterp/config/windowed.hpp"
#include "pyinterp/math/interpolate/bivariate.hpp"
#include "pyinterp/math/interpolate/cache.hpp"
#include "pyinterp/math/interpolate/cache_loader.hpp"
#include "pyinterp/math/interpolate/interpolation_result.hpp"
#include "pyinterp/parallel_for.hpp"
#include "pyinterp/pybind/grid.hpp"

namespace pyinterp::windowed::pybind {

/// @brief Alias for the two-dimensional grid type
/// @tparam DataType Data type stored in the grid
template <typename DataType>
using Grid2D = pyinterp::pybind::Grid2D<DataType>;

namespace detail {

/// @brief Alias for the interpolation result type
/// @tparam T Value type
template <typename T>
using InterpolationResult = pyinterp::math::interpolate::InterpolationResult<T>;

/// @brief Alias for the interpolation cache type
using InterpolationCache =
    pyinterp::math::interpolate::InterpolationCache<double, double, double>;

/// @brief Perform bivariate interpolation for a single point
/// @tparam DataType Data type of the grid
/// @tparam ResultType Result type of the interpolation
/// @param grid 2D grid containing data to interpolate
/// @param[in] x X-coordinate for interpolation
/// @param[in] y Y-coordinate for interpolation
/// @param[in] cfg Configuration parameters for interpolation
/// @param[in,out] interpolator Interpolator object
/// @param[in,out] cache Interpolation cache
/// @return Interpolated value
template <typename DataType, typename ResultType>
[[nodiscard]] auto bivariate_single(
    const Grid2D<DataType>& grid, const double x, const double y,
    const config::windowed::Bivariate& cfg,
    math::interpolate::BivariateBase<double>* interpolator,
    InterpolationCache& cache) -> InterpolationResult<ResultType> {
  auto cache_load_result = math::interpolate::update_cache_if_needed(
      cache, grid, std::make_tuple(x, y), cfg.spatial().boundary_mode(),
      cfg.common().bounds_error());
  if (!cache_load_result.success) {
    // Point is out of bounds. If bounds_error is enabled, the cache loader
    // has recorded an error message that will be raised below.
    if (cache_load_result.error_message.has_value()) {
      throw std::out_of_range(cache_load_result.error_message.value());
    }
    return {};
  }
  if (!cache.is_valid()) {
    // Cache contains only NaN values, interpolation cannot proceed
    return {};
  }
  auto result = (*interpolator)(cache.template coords_as_eigen<0>(),
                                cache.template coords_as_eigen<1>(),
                                cache.matrix(), x, y);
  return InterpolationResult<ResultType>{static_cast<ResultType>(result)};
}

/// @brief Vectorized bivariate interpolation
/// @tparam DataType Data type of the grid
/// @tparam ResultType Result type of the interpolation
/// @param grid 2D grid containing data to interpolate
/// @param[in] x X-coordinates for interpolation
/// @param[in] y Y-coordinates for interpolation
/// @param[in] cfg Configuration parameters for interpolation
/// @return Vector of interpolated values
template <typename DataType, typename ResultType>
[[nodiscard]] auto bivariate(const Grid2D<DataType>& grid,
                             const Eigen::Ref<const Eigen::VectorXd>& x,
                             const Eigen::Ref<const Eigen::VectorXd>& y,
                             const config::windowed::Bivariate& cfg) {
  broadcast::check_eigen_shape("x", x, "y", y);

  Vector<ResultType> result(x.size());
  result.setConstant(std::numeric_limits<ResultType>::quiet_NaN());

  parallel_for(
      x.size(),
      [&](const int64_t start, const int64_t end) {
        // Create cache and interpolator for this thread
        auto cache = InterpolationCache(cfg.spatial().window_size_x(),
                                        cfg.spatial().window_size_y());
        auto interpolator = cfg.spatial().factory<double>();

        for (int64_t ix = start; ix < end; ++ix) {
          auto interpolated_value =
              detail::bivariate_single<DataType, ResultType>(
                  grid, x[ix], y[ix], cfg, interpolator.get(), cache);
          if (interpolated_value.has_value()) {
            result[ix] = *interpolated_value.value;
          }
        }
      },
      cfg.common().num_threads());

  return result;
}

constexpr const char* const kBivariateDocstring = R"doc(
    Perform bivariate interpolation on a 2D grid.

Args:
    grid: 2D grid containing data to interpolate.
    x: X-coordinates for interpolation.
    y: Y-coordinates for interpolation.
    config: Configuration parameters for interpolation.

Returns:
    Vector of interpolated values.

Raises:
    ValueError: If input arrays have mismatched shapes or if interpolation
                cannot be performed due to boundary conditions.
)doc";

}  // namespace detail

/// @brief Bind bivariate interpolation function to a Python module
/// @tparam DataType Data type of the grid
/// @tparam ResultType Result type of the interpolation
/// @param[in,out] m Python module
template <typename DataType, typename ResultType>
auto bind_bivariate(nanobind::module_& m) -> void {
  m.def(
      "bivariate",
      [](const Grid2D<DataType>& grid,
         const Eigen::Ref<const Eigen::VectorXd>& x,
         const Eigen::Ref<const Eigen::VectorXd>& y,
         const config::windowed::Bivariate& config) -> Vector<ResultType> {
        return detail::bivariate<DataType, ResultType>(grid, x, y, config);
      },
      nanobind::arg("grid"), nanobind::arg("x"), nanobind::arg("y"),
      nanobind::arg("config"), detail::kBivariateDocstring,
      nanobind::call_guard<nanobind::gil_scoped_release>());
}

}  // namespace pyinterp::windowed::pybind

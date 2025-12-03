#pragma once

#include <Eigen/Core>
#include <Eigen/Dense>
#include <concepts>
#include <cstdint>
#include <numbers>
#include <optional>
#include <ranges>
#include <stdexcept>

#include "pyinterp/eigen.hpp"
#include "pyinterp/math.hpp"

namespace pyinterp::math::interpolate {

/// Matérn covariance function for ν = 0.5 (i.e., exponential covariance)
template <std::floating_point T>
[[nodiscard]] constexpr auto matern_covariance_12(
    const Eigen::Ref<const Eigen::Vector3<T>>& p1,
    const Eigen::Ref<const Eigen::Vector3<T>>& p2, const T sigma,
    const T lambda) noexcept -> T {
  const T r = (p1 - p2).norm();
  return math::sqr(sigma) * std::exp(-r / lambda);
}

/// Matérn covariance function for ν = 1.5
template <std::floating_point T>
[[nodiscard]] constexpr auto matern_covariance_32(
    const Eigen::Ref<const Eigen::Vector3<T>>& p1,
    const Eigen::Ref<const Eigen::Vector3<T>>& p2, const T sigma,
    const T lambda) noexcept -> T {
  const T r = (p1 - p2).norm();
  const T d = r / lambda;
  constexpr T sqrt3 = std::numbers::sqrt3_v<T>;
  return math::sqr(sigma) * std::fma(sqrt3, d, T{1}) * std::exp(-sqrt3 * d);
}

/// Matérn covariance function for ν = 2.5
template <std::floating_point T>
[[nodiscard]] constexpr auto matern_covariance_52(
    const Eigen::Ref<const Eigen::Vector3<T>>& p1,
    const Eigen::Ref<const Eigen::Vector3<T>>& p2, const T sigma,
    const T lambda) noexcept -> T {
  const T r = (p1 - p2).norm();
  const T d = r / lambda;
  const T sqrt5 = std::numbers::sqrt3_v<T> * std::numbers::sqrt3_v<T> /
                  std::numbers::sqrt3_v<T> * std::sqrt(T{5} / T{3});
  const T term = T{1} + sqrt5 * d + T{5} / T{3} * math::sqr(d);
  return math::sqr(sigma) * term * std::exp(-sqrt5 * d);
}

/// Cauchy covariance function
template <std::floating_point T>
[[nodiscard]] constexpr auto cauchy_covariance(
    const Eigen::Ref<const Eigen::Vector3<T>>& p1,
    const Eigen::Ref<const Eigen::Vector3<T>>& p2, const T sigma,
    const T lambda) noexcept -> T {
  const T r = (p1 - p2).norm();
  return math::sqr(sigma) / (T{1} + math::sqr(r / lambda));
}

/// Spherical covariance function (compact support)
template <std::floating_point T>
[[nodiscard]] constexpr auto spherical_covariance(
    const Eigen::Ref<const Eigen::Vector3<T>>& p1,
    const Eigen::Ref<const Eigen::Vector3<T>>& p2, const T sigma,
    const T lambda) noexcept -> T {
  const T r = (p1 - p2).norm();

  if (r > lambda) [[unlikely]] {
    return T{0};
  }

  const T t = r / lambda;
  return math::sqr(sigma) * (T{1} - T{1.5} * t + T{0.5} * t * t * t);
}

/// Gaussian covariance function (infinitely smooth)
template <std::floating_point T>
[[nodiscard]] constexpr auto gaussian_covariance(
    const Eigen::Ref<const Eigen::Vector3<T>>& p1,
    const Eigen::Ref<const Eigen::Vector3<T>>& p2, const T sigma,
    const T lambda) noexcept -> T {
  const T r = (p1 - p2).norm();
  return math::sqr(sigma) * std::exp(-math::sqr(r / lambda));
}

/// Linear covariance function (Wendland φ_{3,0})
/// Uses Wendland φ_{3,0}(r) = (1 - r/λ)²₊ which is positive definite in R³
template <std::floating_point T>
[[nodiscard]] constexpr auto linear_covariance(
    const Eigen::Ref<const Eigen::Vector3<T>>& p1,
    const Eigen::Ref<const Eigen::Vector3<T>>& p2, const T sigma,
    const T lambda) noexcept -> T {
  const T r = (p1 - p2).norm();

  if (r >= lambda) [[unlikely]] {
    return T{0};
  }

  const T t = T{1} - r / lambda;
  return math::sqr(sigma) * math::sqr(t);
}

/// Known covariance functions
enum class CovarianceFunction : uint8_t {
  kMatern_12,  // ν = 0.5 (exponential)
  kMatern_32,  // ν = 1.5
  kMatern_52,  // ν = 2.5
  kCauchy,     // Heavy-tailed
  kSpherical,  // Compact support
  kGaussian,   // Infinitely smooth
  kLinear,     // Wendland φ_{3,0}
};

/// Known drift functions for universal kriging
enum class DriftFunction : uint8_t {
  kLinear,     // Constant + linear terms (4 parameters)
  kQuadratic,  // Constant + linear + quadratic terms (10 parameters)
};

/// @brief Kriging interpolation for spatial data
/// @tparam T Floating-point type for calculations
template <std::floating_point T>
class Kriging {
 public:
  /// Pointer to the covariance function
  using PtrCovarianceFunction =
      T (*)(const Eigen::Ref<const Eigen::Vector3<T>>&,
            const Eigen::Ref<const Eigen::Vector3<T>>&, T, T) noexcept;

  /// @brief Constructor
  /// @param[in] sigma Magnitude parameter (scale of covariance, must be > 0)
  /// @param[in] lambda Decay rate parameter (spatial scale, must be > 0)
  /// @param[in] nugget Nugget effect (measurement error/microscale variation,
  /// must be ≥ 0)
  /// @param[in] function Covariance function to use
  /// @param[in] drift_function Optional drift function for universal kriging
  /// @note If drift_function is not specified, simple kriging (zero mean) is
  /// used
  Kriging(const T sigma, const T lambda, const T nugget,
          const CovarianceFunction function,
          const std::optional<DriftFunction> drift_function = std::nullopt)
      : sigma_{sigma},
        lambda_{lambda},
        nugget_{nugget},
        drift_function_{drift_function.value_or(DriftFunction::kLinear)} {
    // Validate parameters
    if (sigma_ <= T{0}) [[unlikely]] {
      throw std::invalid_argument("sigma must be greater than 0");
    }
    if (lambda_ <= T{0}) [[unlikely]] {
      throw std::invalid_argument("lambda must be greater than 0");
    }
    if (nugget_ < T{0}) [[unlikely]] {
      throw std::invalid_argument("nugget must be >= 0");
    }

    // Select covariance function
    switch (function) {
      case CovarianceFunction::kMatern_12:
        function_ = matern_covariance_12<T>;
        break;
      case CovarianceFunction::kMatern_32:
        function_ = matern_covariance_32<T>;
        break;
      case CovarianceFunction::kMatern_52:
        function_ = matern_covariance_52<T>;
        break;
      case CovarianceFunction::kCauchy:
        function_ = cauchy_covariance<T>;
        break;
      case CovarianceFunction::kSpherical:
        function_ = spherical_covariance<T>;
        break;
      case CovarianceFunction::kGaussian:
        function_ = gaussian_covariance<T>;
        break;
      case CovarianceFunction::kLinear:
        function_ = linear_covariance<T>;
        break;
      [[unlikely]] default:
        throw std::invalid_argument("Invalid covariance function");
    }

    // Select kriging method
    method_ptr_ = drift_function.has_value() ? &Kriging::universal_kriging
                                             : &Kriging::simple_kriging;
  }

  /// @brief Estimate the value at a query point
  /// @param[in] coordinates Coordinates of known points (3 × n matrix)
  /// @param[in] values Values at known points (n-vector)
  /// @param[in] query Coordinates of query point (3-vector)
  /// @return Estimated value at query point
  [[nodiscard]] auto operator()(const Eigen::Matrix<T, 3, -1>& coordinates,
                                const Eigen::Matrix<T, -1, 1>& values,
                                const Eigen::Vector3<T>& query) const -> T {
    return (this->*method_ptr_)(coordinates, values, query);
  }

 private:
  using MethodPtr = T (Kriging::*)(const Eigen::Matrix<T, 3, -1>&,
                                   const Eigen::Matrix<T, -1, 1>&,
                                   const Eigen::Vector3<T>&) const;
  const T sigma_;
  const T lambda_;
  const T nugget_;
  DriftFunction drift_function_;
  PtrCovarianceFunction function_;
  MethodPtr method_ptr_;

  /// @brief Get drift terms for a point (polynomial basis functions)
  /// @param[in] point The point coordinates
  /// @param[in] function The drift function type
  /// @return Vector of drift terms
  [[nodiscard]] static auto get_drift_terms(const Eigen::Vector3<T>& point,
                                            const DriftFunction function)
      -> Vector<T> {
    switch (function) {
      case DriftFunction::kLinear: {
        // [1, x, y, z]
        Vector<T> terms(4);
        terms << T{1}, point(0), point(1), point(2);
        return terms;
      }
      case DriftFunction::kQuadratic: {
        // [1, x, y, z, x², y², z², xy, xz, yz]
        Vector<T> terms(10);
        terms << T{1}, point(0), point(1), point(2), math::sqr(point(0)),
            math::sqr(point(1)), math::sqr(point(2)), point(0) * point(1),
            point(0) * point(2), point(1) * point(2);
        return terms;
      }
      [[unlikely]] default:
        throw std::invalid_argument("Invalid drift function");
    }
  }

  /// @brief Simple kriging (assumes zero mean)
  /// @param[in] coordinates Known point coordinates
  /// @param[in] values Known values
  /// @param[in] query Query point
  /// @return Estimated value
  [[nodiscard]] auto simple_kriging(const Eigen::Matrix<T, 3, -1>& coordinates,
                                    const Eigen::Matrix<T, -1, 1>& values,
                                    const Eigen::Vector3<T>& query) const -> T {
    const auto k = coordinates.cols();

    // Build covariance matrix C (k × k)
    Matrix<T> C(k, k);
    for (auto i : std::views::iota(Eigen::Index{0}, k)) {
      for (auto j : std::views::iota(i, k)) {
        C(i, j) =
            function_(coordinates.col(i), coordinates.col(j), sigma_, lambda_);
        if (i != j) {
          C(j, i) = C(i, j);  // Symmetric
        }
      }
      C(i, i) += nugget_;  // Add nugget to diagonal
    }

    // Build covariance vector c (k-vector)
    Vector<T> c(k);
    for (auto i : std::views::iota(Eigen::Index{0}, k)) {
      c(i) = function_(query, coordinates.col(i), sigma_, lambda_);
    }

    // Solve C w = c for weights w, then compute weighted sum
    const Vector<T> w = C.ldlt().solve(c);
    return values.dot(w);
  }

  /// @brief Universal kriging (with polynomial drift)
  /// @param[in] coordinates Known point coordinates
  /// @param[in] values Known values
  /// @param[in] query Query point
  /// @return Estimated value
  [[nodiscard]] auto universal_kriging(
      const Eigen::Matrix<T, 3, -1>& coordinates,
      const Eigen::Matrix<T, -1, 1>& values,
      const Eigen::Vector3<T>& query) const -> T {
    const auto k = coordinates.cols();
    const auto f = get_drift_terms(query, drift_function_);
    const auto p = f.size();

    // Build covariance matrix C (k × k)
    Matrix<T> C(k, k);
    for (auto i : std::views::iota(Eigen::Index{0}, k)) {
      for (auto j : std::views::iota(i, k)) {
        C(i, j) =
            function_(coordinates.col(i), coordinates.col(j), sigma_, lambda_);
        if (i != j) {
          C(j, i) = C(i, j);  // Symmetric
        }
      }
      C(i, i) += nugget_;
    }

    // Build drift matrix F (k × p)
    Matrix<T> F(k, p);
    for (auto i : std::views::iota(Eigen::Index{0}, k)) {
      F.row(i) = get_drift_terms(coordinates.col(i), drift_function_);
    }

    // Build augmented system: [C  F ] [w] = [c]
    //                         [F' 0 ] [β]   [f]
    Matrix<T> A(k + p, k + p);
    A.topLeftCorner(k, k) = C;
    A.topRightCorner(k, p) = F;
    A.bottomLeftCorner(p, k) = F.transpose();
    A.bottomRightCorner(p, p).setZero();

    // Build covariance vector c (k-vector)
    Vector<T> c(k);
    for (auto i : std::views::iota(Eigen::Index{0}, k)) {
      c(i) = function_(query, coordinates.col(i), sigma_, lambda_);
    }

    // Build right-hand side [c; f]
    Vector<T> b(k + p);
    b.head(k) = c;
    b.tail(p) = f;

    // Solve augmented system
    const Vector<T> x = A.colPivHouseholderQr().solve(b);

    // Return prediction (only use kriging weights, not drift coefficients)
    return values.dot(x.head(k));
  }
};

}  // namespace pyinterp::math::interpolate

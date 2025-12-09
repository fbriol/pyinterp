#pragma once

#include <cstdint>
#include <optional>

#include "pyinterp/math/interpolate/kriging.hpp"
#include "pyinterp/math/interpolate/rbf.hpp"
#include "pyinterp/math/interpolate/window_function.hpp"
#include "pyinterp/pybind/config/common.hpp"

namespace pyinterp::pybind::config::rtree {

/// @brief Base class for RTree interpolation configurations using CRTP.
/// @tparam Derived The derived configuration class.
/// @details Provides common functionality for all RTree-based interpolation
/// methods, including neighbor count (k), search radius, and thread settings.
template <typename Derived>
class RTreeBase : public ThreadConfig {
 public:
  /// @brief Default constructor
  constexpr RTreeBase() = default;

  /// @brief Get the number of neighbors.
  /// @return Number of neighbors to consider
  [[nodiscard]] constexpr auto k() const -> uint32_t { return k_; }

  /// @brief Get the search radius.
  /// @return Optional search radius in meters (nullopt = unlimited)
  [[nodiscard]] constexpr auto radius() const -> const std::optional<double>& {
    return radius_;
  }

  /// @brief Set the number of neighbors
  /// @param[in] value Number of neighbors
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_k(uint32_t value) const -> Derived {
    auto copy = static_cast<const Derived&>(*this);
    copy.k_ = value;
    return copy;
  }

  /// @brief Set the search radius in meters
  /// @param[in] value Search radius (nullopt for unlimited)
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_radius(
      const std::optional<double>& value) const -> Derived {
    auto copy = static_cast<const Derived&>(*this);
    copy.radius_ = value;
    return copy;
  }

  /// @brief Set the number of threads
  /// @param[in] value Number of threads
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_num_threads(uint32_t value) const
      -> Derived {
    auto copy = static_cast<const Derived&>(*this);
    static_cast<ThreadConfig&>(copy) = ThreadConfig::with_num_threads(value);
    return copy;
  }

 protected:
  /// Number of neighbors to consider
  uint32_t k_{8};

  /// Optional search radius in meters (nullopt = unlimited)
  std::optional<double> radius_;
};

/// Configuration for inverse distance weighting interpolation
class InverseDistanceWeighting : public RTreeBase<InverseDistanceWeighting> {
 public:
  /// @brief Default constructor
  constexpr InverseDistanceWeighting() = default;

  /// @brief Get the power parameter.
  /// @return Power parameter (exponent)
  [[nodiscard]] constexpr auto p() const -> uint32_t { return p_; }

  /// @brief Set the power parameter
  /// @param[in] value Power parameter
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_p(uint32_t value) const
      -> InverseDistanceWeighting {
    auto copy = *this;
    copy.p_ = value;
    return copy;
  }

 private:
  /// Power parameter (exponent)
  uint32_t p_{2};
};

// ////////////////////////////////////////////////////////////////////////////

/// Configuration for kriging interpolation
class Kriging : public RTreeBase<Kriging> {
 public:
  /// @brief Default constructor
  constexpr Kriging() = default;

  /// @brief Get the sill parameter.
  /// @return Sill parameter (variance at infinity)
  [[nodiscard]] constexpr auto sigma() const -> double { return sigma_; }

  /// @brief Get the range parameter.
  /// @return Range parameter (distance scale)
  [[nodiscard]] constexpr auto lambda() const -> double { return lambda_; }

  /// @brief Get the nugget effect parameter.
  /// @return Nugget effect parameter (micro-scale variance)
  [[nodiscard]] constexpr auto nugget() const -> double { return nugget_; }

  /// @brief Get the covariance function type.
  /// @return Covariance function type
  [[nodiscard]] constexpr auto covariance_model() const
      -> math::interpolate::CovarianceFunction {
    return covariance_model_;
  }

  /// @brief Get the drift function.
  /// @return Optional drift function
  [[nodiscard]] constexpr auto drift_function() const
      -> const std::optional<math::interpolate::DriftFunction>& {
    return drift_function_;
  }

  /// @brief Set the sill parameter
  /// @param[in] value Sill parameter
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_sigma(double value) const -> Kriging {
    auto copy = *this;
    copy.sigma_ = value;
    return copy;
  }

  /// @brief Set the range parameter
  /// @param[in] value Range parameter
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_lambda(double value) const -> Kriging {
    auto copy = *this;
    copy.lambda_ = value;
    return copy;
  }

  /// @brief Set the nugget effect parameter
  /// @param[in] value Nugget effect parameter
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_nugget(double value) const -> Kriging {
    auto copy = *this;
    copy.nugget_ = value;
    return copy;
  }

  /// @brief Set the covariance model
  /// @param[in] value Covariance function type
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_covariance_model(
      math::interpolate::CovarianceFunction value) const -> Kriging {
    auto copy = *this;
    copy.covariance_model_ = value;
    return copy;
  }

  /// @brief Set the drift function
  /// @param[in] value Drift function type
  /// @return Updated configuration
  [[nodiscard]] auto with_drift_function(
      std::optional<math::interpolate::DriftFunction> value) const -> Kriging {
    auto copy = *this;
    copy.drift_function_ = value;
    return copy;
  }

 private:
  /// Sill parameter (variance at infinity)
  double sigma_{1.0};

  /// Range parameter (distance scale)
  double lambda_{1.0};

  /// Nugget effect parameter (micro-scale variance)
  double nugget_{0.0};

  /// Covariance function type
  math::interpolate::CovarianceFunction covariance_model_{
      math::interpolate::CovarianceFunction::kSpherical};

  /// Optional drift function
  std::optional<math::interpolate::DriftFunction> drift_function_;
};

// ////////////////////////////////////////////////////////////////////////////

/// Configuration for radial basis function interpolation
class RadialBasisFunction : public RTreeBase<RadialBasisFunction> {
 public:
  /// @brief Default constructor
  constexpr RadialBasisFunction() = default;

  /// @brief Get the radial basis function type.
  /// @return Radial basis function type
  [[nodiscard]] constexpr auto rbf() const -> math::interpolate::RBFKernel {
    return rbf_;
  }

  /// @brief Get the shape parameter (epsilon).
  /// @return Optional shape parameter (epsilon)
  [[nodiscard]] constexpr auto epsilon() const -> const std::optional<double>& {
    return epsilon_;
  }

  /// @brief Get the smoothing parameter.
  /// @return Smoothing parameter
  [[nodiscard]] constexpr auto smooth() const -> double { return smooth_; }

  /// @brief Set the RBF type
  /// @param[in] value Radial basis function type
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_rbf(
      math::interpolate::RBFKernel value) const -> RadialBasisFunction {
    auto copy = *this;
    copy.rbf_ = value;
    return copy;
  }

  /// @brief Set the shape parameter (epsilon)
  /// @param[in] value Shape parameter (nullopt for auto)
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_epsilon(std::optional<double> value) const
      -> RadialBasisFunction {
    auto copy = *this;
    copy.epsilon_ = value;
    return copy;
  }

  /// @brief Set the smoothing parameter
  /// @param[in] value Smoothing parameter
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_smooth(double value) const
      -> RadialBasisFunction {
    auto copy = *this;
    copy.smooth_ = value;
    return copy;
  }

 private:
  /// Radial basis function type
  math::interpolate::RBFKernel rbf_{
      math::interpolate::RBFKernel::kMultiquadric};

  /// Optional shape parameter (epsilon)
  std::optional<double> epsilon_;

  /// Smoothing parameter
  double smooth_{0.0};
};

// ////////////////////////////////////////////////////////////////////////////

/// Configuration for window function interpolation
class InterpolationWindow : public RTreeBase<InterpolationWindow> {
 public:
  /// @brief Default constructor
  constexpr InterpolationWindow() = default;

  /// @brief Get the window function type.
  /// @return Window function type
  [[nodiscard]] constexpr auto wf() const -> math::interpolate::window::Kernel {
    return wf_;
  }

  /// @brief Get the window function argument.
  /// @return Optional window function argument
  [[nodiscard]] constexpr auto arg() const -> const std::optional<double>& {
    return arg_;
  }

  /// @brief Set the window function type
  /// @param[in] value Window function type
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_wf(
      math::interpolate::window::Kernel value) const -> InterpolationWindow {
    auto copy = *this;
    copy.wf_ = value;
    return copy;
  }

  /// @brief Set the window function argument
  /// @param[in] value Window function argument
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_arg(std::optional<double> value) const
      -> InterpolationWindow {
    auto copy = *this;
    copy.arg_ = value;
    return copy;
  }

 private:
  /// Window function type
  math::interpolate::window::Kernel wf_{
      math::interpolate::window::Kernel::kGaussian};

  /// Optional window function argument
  std::optional<double> arg_;
};

}  // namespace pyinterp::pybind::config::rtree

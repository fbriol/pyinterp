#pragma once

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

#include "pyinterp/math/interpolate/rbf.hpp"
#include "pyinterp/math/interpolate/window_function.hpp"
#include "pyinterp/pybind/config/common.hpp"

namespace pyinterp::pybind::config {
namespace rtree {

/// Configuration for inverse distance weighting interpolation
struct InverseDistanceWeighting : Common {
  /// Number of neighbors to consider
  uint32_t k{8};

  /// Power parameter (exponent)
  uint32_t p{2};

  /// Optional search radius in meters (nullopt = unlimited)
  std::optional<double> radius;

  /// Default constructor
  constexpr InverseDistanceWeighting() = default;

  /// Constructor with k parameter
  constexpr explicit InverseDistanceWeighting(uint32_t k) : k(k) {}

  /// Constructor with k and p parameters
  constexpr InverseDistanceWeighting(uint32_t k, uint32_t p)
      : k(k), p(p) {}

  /// @brief Set the number of neighbors
  /// @param[in] value Number of neighbors
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_k(uint32_t value) const
      -> InverseDistanceWeighting {
    auto copy = *this;
    copy.k = value;
    return copy;
  }

  /// @brief Set the power parameter
  /// @param[in] value Power parameter
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_p(uint32_t value) const
      -> InverseDistanceWeighting {
    auto copy = *this;
    copy.p = value;
    return copy;
  }

  /// @brief Set the search radius in meters
  /// @param[in] value Search radius (nullopt for unlimited)
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_radius(std::optional<double> value) const
      -> InverseDistanceWeighting {
    auto copy = *this;
    copy.radius = value;
    return copy;
  }

  /// @brief Create a default IDW configuration
  /// @param[in] k Number of neighbors
  /// @return IDW configuration with default parameters
  [[nodiscard]] static constexpr auto create(uint32_t k = 8)
      -> InverseDistanceWeighting {
    return InverseDistanceWeighting{k};
  }
};

// ////////////////////////////////////////////////////////////////////////////

/// Configuration for kriging interpolation
struct Kriging : Common {
  /// Number of neighbors to consider
  uint32_t k{8};

  /// Sill parameter (variance at infinity)
  double sigma{1.0};

  /// Range parameter (distance scale)
  double lambda{1.0};

  /// Nugget effect parameter (micro-scale variance)
  double nugget{0.0};

  /// Covariance function type
  math::interpolate::CovarianceFunction covariance_model{
      math::interpolate::CovarianceFunction::kSpherical};

  /// Optional drift function
  std::optional<math::interpolate::DriftFunction> drift_function;

  /// Optional search radius in meters (nullopt = unlimited)
  std::optional<double> radius;

  /// Default constructor
  constexpr Kriging() = default;

  /// Constructor with k parameter
  constexpr explicit Kriging(uint32_t k) : k(k) {}

  /// @brief Set the number of neighbors
  /// @param[in] value Number of neighbors
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_k(uint32_t value) const -> Kriging {
    auto copy = *this;
    copy.k = value;
    return copy;
  }

  /// @brief Set the sill parameter
  /// @param[in] value Sill parameter
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_sigma(double value) const -> Kriging {
    auto copy = *this;
    copy.sigma = value;
    return copy;
  }

  /// @brief Set the range parameter
  /// @param[in] value Range parameter
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_lambda(double value) const -> Kriging {
    auto copy = *this;
    copy.lambda = value;
    return copy;
  }

  /// @brief Set the nugget effect parameter
  /// @param[in] value Nugget effect parameter
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_nugget(double value) const -> Kriging {
    auto copy = *this;
    copy.nugget = value;
    return copy;
  }

  /// @brief Set the covariance model
  /// @param[in] value Covariance function type
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_covariance_model(
      math::interpolate::CovarianceFunction value) const -> Kriging {
    auto copy = *this;
    copy.covariance_model = value;
    return copy;
  }

  /// @brief Set the drift function
  /// @param[in] value Drift function type
  /// @return Updated configuration
  [[nodiscard]] auto with_drift_function(
      std::optional<math::interpolate::DriftFunction> value) const -> Kriging {
    auto copy = *this;
    copy.drift_function = value;
    return copy;
  }

  /// @brief Set the search radius in meters
  /// @param[in] value Search radius (nullopt for unlimited)
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_radius(std::optional<double> value) const
      -> Kriging {
    auto copy = *this;
    copy.radius = value;
    return copy;
  }

  /// @brief Create a default kriging configuration
  /// @param[in] k Number of neighbors
  /// @return Kriging configuration with default parameters
  [[nodiscard]] static constexpr auto create(uint32_t k = 8) -> Kriging {
    return Kriging{k};
  }
};

// ////////////////////////////////////////////////////////////////////////////

/// Configuration for radial basis function interpolation
struct RadialBasisFunction : Common {
  /// Number of neighbors to consider
  uint32_t k{8};

  /// Radial basis function type
  math::interpolate::RadialBasisFunction rbf{
      math::interpolate::RadialBasisFunction::kMultiquadric};

  /// Optional shape parameter (epsilon)
  std::optional<double> epsilon;

  /// Smoothing parameter
  double smooth{0.0};

  /// Optional search radius in meters (nullopt = unlimited)
  std::optional<double> radius;

  /// Default constructor
  constexpr RadialBasisFunction() = default;

  /// Constructor with k parameter
  constexpr explicit RadialBasisFunction(uint32_t k) : k(k) {}

  /// @brief Set the number of neighbors
  /// @param[in] value Number of neighbors
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_k(uint32_t value) const
      -> RadialBasisFunction {
    auto copy = *this;
    copy.k = value;
    return copy;
  }

  /// @brief Set the RBF type
  /// @param[in] value Radial basis function type
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_rbf(
      math::interpolate::RadialBasisFunction value) const
      -> RadialBasisFunction {
    auto copy = *this;
    copy.rbf = value;
    return copy;
  }

  /// @brief Set the shape parameter (epsilon)
  /// @param[in] value Shape parameter (nullopt for auto)
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_epsilon(std::optional<double> value) const
      -> RadialBasisFunction {
    auto copy = *this;
    copy.epsilon = value;
    return copy;
  }

  /// @brief Set the smoothing parameter
  /// @param[in] value Smoothing parameter
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_smooth(double value) const
      -> RadialBasisFunction {
    auto copy = *this;
    copy.smooth = value;
    return copy;
  }

  /// @brief Set the search radius in meters
  /// @param[in] value Search radius (nullopt for unlimited)
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_radius(std::optional<double> value) const
      -> RadialBasisFunction {
    auto copy = *this;
    copy.radius = value;
    return copy;
  }

  /// @brief Create a default RBF configuration
  /// @param[in] k Number of neighbors
  /// @return RBF configuration with default parameters
  [[nodiscard]] static constexpr auto create(uint32_t k = 8)
      -> RadialBasisFunction {
    return RadialBasisFunction{k};
  }
};

// ////////////////////////////////////////////////////////////////////////////

/// Configuration for window function interpolation
struct WindowFunction : Common {
  /// Number of neighbors to consider
  uint32_t k{8};

  /// Window function type
  math::interpolate::window::Function wf{
      math::interpolate::window::Function::kTriangle};

  /// Optional window function argument
  std::optional<double> arg;

  /// Optional search radius in meters (nullopt = unlimited)
  std::optional<double> radius;

  /// Default constructor
  constexpr WindowFunction() = default;

  /// Constructor with k parameter
  constexpr explicit WindowFunction(uint32_t k) : k(k) {}

  /// @brief Set the number of neighbors
  /// @param[in] value Number of neighbors
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_k(uint32_t value) const
      -> WindowFunction {
    auto copy = *this;
    copy.k = value;
    return copy;
  }

  /// @brief Set the window function type
  /// @param[in] value Window function type
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_wf(
      math::interpolate::window::Function value) const -> WindowFunction {
    auto copy = *this;
    copy.wf = value;
    return copy;
  }

  /// @brief Set the window function argument
  /// @param[in] value Window function argument
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_arg(std::optional<double> value) const
      -> WindowFunction {
    auto copy = *this;
    copy.arg = value;
    return copy;
  }

  /// @brief Set the search radius in meters
  /// @param[in] value Search radius (nullopt for unlimited)
  /// @return Updated configuration
  [[nodiscard]] constexpr auto with_radius(std::optional<double> value) const
      -> WindowFunction {
    auto copy = *this;
    copy.radius = value;
    return copy;
  }

  /// @brief Create a default window function configuration
  /// @param[in] k Number of neighbors
  /// @return Window function configuration with default parameters
  [[nodiscard]] static constexpr auto create(uint32_t k = 8)
      -> WindowFunction {
    return WindowFunction{k};
  }
};

}  // namespace rtree
}  // namespace pyinterp::pybind::config

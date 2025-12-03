#pragma once

#include <stdexcept>
#include <string_view>

#include "pyinterp/math/interpolate/geometric/bivariate.hpp"
#include "pyinterp/pybind/config/common.hpp"

namespace pyinterp::pybind::config {
namespace geometric {

/// Method for 2D spatial interpolation.
using SpatialMethod = math::interpolate::geometric::InterpolationMethod;

/// @brief Parser for a spatial interpolation method name.
/// @param[in] method_name Name of the interpolation method (case-sensitive)
/// @return Corresponding `SpatialMethod` enum value.
/// @throws std::invalid_argument if the method name is unknown.
[[nodiscard]] inline auto parse_spatial_method(std::string_view method_name)
    -> SpatialMethod {
  if (method_name == "bilinear") {
    return SpatialMethod::kBilinear;
  }
  if (method_name == "idw") {
    return SpatialMethod::kInverseDistanceWeighting;
  }
  if (method_name == "nearest") {
    return SpatialMethod::kNearest;
  }
  throw std::invalid_argument("Unknown spatial interpolation method: " +
                              std::string(method_name));
}

/// Configuration for 2D spatial interpolation.
struct Spatial {
  /// @brief Interpolation method
  SpatialMethod method{SpatialMethod::kBilinear};

  /// @brief Exponent for inverse distance weighting method
  int exponent{2};

  /// @brief Default constructor
  constexpr Spatial() = default;

  /// @brief Constructor with an explicit interpolation method
  /// @param[in] method Interpolation method to use
  constexpr explicit Spatial(SpatialMethod method) : method(method) {}

  /// @brief Constructor with method and exponent
  /// @param[in] method Interpolation method to use
  /// @param[in] exponent Exponent for the inverse distance weighting method
  constexpr Spatial(SpatialMethod method, int exponent)
      : method(method), exponent(exponent) {}

  /// @brief Create a configuration for bilinear interpolation.
  /// @return `Spatial` configured with the bilinear interpolation method.
  [[nodiscard]] static constexpr auto bilinear() -> Spatial { return {}; }

  /// @brief Create a configuration for nearest-neighbor interpolation.
  /// @return `Spatial` configured with the nearest-neighbor method.
  [[nodiscard]] static constexpr auto nearest() -> Spatial {
    return Spatial{SpatialMethod::kNearest};
  }

  /// @brief Create a configuration for inverse distance weighting (IDW)
  /// interpolation.
  /// @param[in] exponent Exponent for the inverse distance weighting method
  /// (default: 2).
  /// @return `Spatial` configured with the IDW interpolation method.
  [[nodiscard]] static constexpr auto idw(int exponent = 2) -> Spatial {
    return Spatial{SpatialMethod::kInverseDistanceWeighting, exponent};
  }
};

// Forward declarations
struct Bivariate;
struct Trivariate;
struct Quadrivariate;

}  // namespace geometric

/// @brief Traits to determine the number of additional axes for the
/// `Bivariate` configuration type.
template <>
struct InterpolationTraits<geometric::Bivariate> {
  static constexpr size_t num_axes = 0;
};

/// @brief Traits to determine the number of additional axes for the
/// `Trivariate` configuration type.
template <>
struct InterpolationTraits<geometric::Trivariate> {
  static constexpr size_t num_axes = 1;
};

/// @brief Traits to determine the number of additional axes for the
/// `Quadrivariate` configuration type.
template <>
struct InterpolationTraits<geometric::Quadrivariate> {
  static constexpr size_t num_axes = 2;
};

namespace geometric {

/// @brief Base class for bivariate-based interpolation configurations using
/// the Curiously Recurring Template Pattern (CRTP).
/// @tparam Derived The derived configuration class.
template <typename Derived>
struct BivariateBase : Base<Spatial, Derived> {
  /// @brief Create a configuration for bilinear interpolation
  [[nodiscard]] static constexpr auto bilinear() -> Derived {
    return create_config(Spatial::bilinear(), AxisConfig::linear());
  }

  /// @brief Create a configuration for IDW interpolation
  [[nodiscard]] static constexpr auto idw(int exp = 2) -> Derived {
    return create_config(Spatial::idw(exp), AxisConfig::linear());
  }

  /// @brief Create a configuration for nearest-neighbor interpolation
  [[nodiscard]] static constexpr auto nearest() -> Derived {
    return create_config(Spatial::nearest(), AxisConfig::nearest());
  }

 private:
  /// @brief Helper to create a configuration with the appropriate number
  /// of axes for the derived type.
  [[nodiscard]] static constexpr auto create_config(
      const Spatial& spatial, const AxisConfig& axis_config) -> Derived {
    constexpr size_t num_axes = InterpolationTraits<Derived>::num_axes;

    if constexpr (num_axes == 0) {
      // Bivariate case
      return Derived{spatial};
    } else if constexpr (num_axes == 1) {
      // Trivariate case
      return Derived{spatial, axis_config};
    } else if constexpr (num_axes == 2) {
      // Quadrivariate case
      return Derived{spatial, axis_config, axis_config};
    }
  }
};

/// Bivariate interpolation configuration (2D).
struct Bivariate : BivariateBase<Bivariate> {
  Spatial spatial;  /// Spatial interpolation configuration
  Common common;    /// Common interpolation configuration

  /// @brief Default constructor
  constexpr Bivariate() = default;

  /// @brief Constructor with spatial and common configurations
  /// @param[in] spatial Spatial interpolation configuration
  /// @param[in] common Common interpolation configuration (optional)
  constexpr explicit Bivariate(const Spatial& spatial,
                               const Common& common = {})
      : spatial(spatial), common(common) {}
};

/// Trivariate interpolation configuration (3D).
struct Trivariate : BivariateBase<Trivariate> {
  Spatial spatial;        /// Spatial interpolation configuration
  AxisConfig third_axis;  /// Third axis interpolation configuration
  Common common;          /// Common interpolation configuration

  /// @brief Default constructor
  constexpr Trivariate() = default;

  /// @brief Constructor with spatial, third axis, and common configurations
  /// @param[in] spatial Spatial interpolation configuration
  /// @param[in] third_axis Third axis interpolation configuration
  /// @param[in] common Common interpolation configuration (optional)
  constexpr Trivariate(const Spatial& spatial, const AxisConfig& third_axis,
                       const Common& common = {})
      : spatial(spatial), third_axis(third_axis), common(common) {}

  /// @brief Update the third-axis configuration.
  /// @param[in] config New third-axis configuration.
  /// @return Updated `Trivariate` instance.
  [[nodiscard]] constexpr auto with_third_axis(const AxisConfig& config) const
      -> Trivariate {
    auto copy = *this;
    copy.third_axis = config;
    return copy;
  }
};

/// Quadrivariate interpolation configuration (4D).
struct Quadrivariate : BivariateBase<Quadrivariate> {
  Spatial spatial;         /// Spatial interpolation configuration
  AxisConfig third_axis;   /// Third axis interpolation configuration
  AxisConfig fourth_axis;  /// Fourth axis interpolation configuration
  Common common;           /// Common interpolation configuration

  /// @brief Default constructor
  constexpr Quadrivariate() = default;

  /// @brief Constructor with spatial, third axis, fourth axis, and common
  /// configurations
  /// @param[in] spatial Spatial interpolation configuration
  /// @param[in] third_axis Third axis interpolation configuration
  /// @param[in] fourth_axis Fourth axis interpolation configuration
  /// @param[in] common Common interpolation configuration (optional)
  constexpr Quadrivariate(const Spatial& spatial, const AxisConfig& third_axis,
                          const AxisConfig& fourth_axis,
                          const Common& common = {})
      : spatial(spatial),
        third_axis(third_axis),
        fourth_axis(fourth_axis),
        common(common) {}

  /// @brief Update the third-axis configuration.
  /// @param[in] config New third-axis configuration.
  /// @return Updated `Quadrivariate` instance.
  [[nodiscard]] constexpr auto with_third_axis(const AxisConfig& config) const
      -> Quadrivariate {
    auto copy = *this;
    copy.third_axis = config;
    return copy;
  }

  /// @brief Update the fourth-axis configuration.
  /// @param[in] config New fourth-axis configuration.
  /// @return Updated `Quadrivariate` instance.
  [[nodiscard]] constexpr auto with_fourth_axis(const AxisConfig& config) const
      -> Quadrivariate {
    auto copy = *this;
    copy.fourth_axis = config;
    return copy;
  }
};

}  // namespace geometric
}  // namespace pyinterp::pybind::config

#pragma once

#include <stdexcept>
#include <string_view>
#include <variant>

#include "pyinterp/math/axis.hpp"
#include "pyinterp/math/interpolate/factory.hpp"
#include "pyinterp/pybind/config/common.hpp"

namespace pyinterp::pybind::config {
namespace windowed {

/// Type alias for boundary mode
using BoundaryMode = math::axis::Boundary;

/// Type alias for Spline fitting model
using Spline = math::interpolate::univariate::Method;

/// Type alias for Bicubic fitting model
using Bicubic = math::interpolate::bivariate::Method;

/// @brief Known fitting models
using SpatialMethod = std::variant<Spline, Bicubic>;

/// @brief Parser for a boundary mode name.
/// @param[in] boundary_name Name of the boundary mode (case-sensitive)
/// @return Corresponding `BoundaryMode` enum value.
/// @throws std::invalid_argument if the boundary name is unknown.
[[nodiscard]] inline auto parse_boundary_mode(std::string_view boundary_name)
    -> BoundaryMode {
  if (boundary_name == "expand") {
    return BoundaryMode::kExpand;
  }
  if (boundary_name == "wrap") {
    return BoundaryMode::kWrap;
  }
  if (boundary_name == "sym") {
    return BoundaryMode::kSym;
  }
  if (boundary_name == "undef") {
    return BoundaryMode::kUndef;
  }
  throw std::invalid_argument("Unknown boundary mode: " +
                              std::string(boundary_name));
}

/// @brief Parser for a fitting model name.
/// @param[in] model_name Name of the fitting model (case-sensitive)
/// @return Corresponding `SpatialMethod` variant value.
/// @throws std::invalid_argument if the model name is unknown.
[[nodiscard]] inline auto parse_fitting_model(std::string_view model_name)
    -> SpatialMethod {
  if (model_name == "akima") {
    return Spline::kAkima;
  }
  if (model_name == "akima_periodic") {
    return Spline::kAkimaPeriodic;
  }
  if (model_name == "c_spline") {
    return Spline::kCSpline;
  }
  if (model_name == "c_spline_not_a_knot") {
    return Spline::kCSplineNotAKnot;
  }
  if (model_name == "c_spline_periodic") {
    return Spline::kCSplinePeriodic;
  }
  if (model_name == "linear") {
    return Spline::kLinear;
  }
  if (model_name == "steffen") {
    return Spline::kSteffen;
  }
  if (model_name == "polynomial") {
    return Spline::kPolynomial;
  }
  if (model_name == "bilinear") {
    return Bicubic::kBilinear;
  }
  if (model_name == "bicubic") {
    return Bicubic::kBicubic;
  }
  throw std::invalid_argument("Unknown fitting model: " +
                              std::string(model_name));
}

/// @brief Configuration for 2D spatial interpolation.
struct Spatial {
  /// @brief Interpolation method
  SpatialMethod method{Bicubic::kBicubic};

  /// @brief Boundary mode
  BoundaryMode boundary_mode{BoundaryMode::kUndef};

  /// Window size in x direction
  int window_size_x{3};

  /// Window size in y direction
  int window_size_y{3};

  /// @brief Default constructor
  constexpr Spatial() = default;

  /// @brief Constructor with an explicit interpolation method
  /// @param[in] method Interpolation method to use
  constexpr explicit Spatial(SpatialMethod method) : method(method) {}

  /// @brief Create the interpolator instance.
  /// @tparam T Data type handled by the interpolator.
  /// @return Unique pointer to the interpolator instance.
  template <typename T>
  [[nodiscard]] auto factory() const
      -> std::unique_ptr<math::interpolate::BivariateBase<T>> {
    if (std::holds_alternative<Bicubic>(method)) {
      return math::interpolate::factory<T>(std::get<Bicubic>(method));
    }
    return math::interpolate::factory<T>(std::get<Spline>(method));
  }

  /// @brief Create a configuration for bilinear interpolation.
  /// @return `Spatial` configured with the bilinear interpolation method.
  [[nodiscard]] static constexpr auto bilinear() -> Spatial {
    return Spatial{Bicubic::kBilinear};
  }

  /// @brief Create a configuration for bicubic interpolation.
  /// @return `Spatial` configured with the bicubic interpolation method.
  [[nodiscard]] static constexpr auto bicubic() -> Spatial {
    return Spatial{Bicubic::kBicubic};
  }

  /// @brief Create a configuration for linear interpolation.
  /// @return `Spatial` configured with the linear interpolation method.
  [[nodiscard]] static constexpr auto linear() -> Spatial {
    return Spatial{Spline::kLinear};
  }

  /// @brief Create a configuration for Akima spline interpolation.
  /// @return `Spatial` configured with the Akima spline interpolation method.
  [[nodiscard]] static constexpr auto akima() -> Spatial {
    return Spatial{Spline::kAkima};
  }

  /// @brief Create a configuration for Akima periodic spline interpolation.
  /// @return `Spatial` configured with the Akima periodic spline method.
  [[nodiscard]] static constexpr auto akima_periodic() -> Spatial {
    return Spatial{Spline::kAkimaPeriodic};
  }

  /// @brief Create a configuration for a C-spline interpolation.
  /// @return `Spatial` configured with the C-spline interpolation method.
  [[nodiscard]] static constexpr auto c_spline() -> Spatial {
    return Spatial{Spline::kCSpline};
  }

  /// @brief Create a configuration for C-spline (not-a-knot) interpolation.
  /// @return `Spatial` configured with the not-a-knot C-spline method.
  [[nodiscard]] static constexpr auto c_spline_not_a_knot() -> Spatial {
    return Spatial{Spline::kCSplineNotAKnot};
  }

  /// @brief Create a configuration for C-spline periodic interpolation.
  [[nodiscard]] static constexpr auto c_spline_periodic() -> Spatial {
    return Spatial{Spline::kCSplinePeriodic};
  }

  /// @brief Create a configuration for Steffen spline interpolation.
  /// @return `Spatial` configured with the Steffen spline method.
  [[nodiscard]] static constexpr auto steffen() -> Spatial {
    return Spatial{Spline::kSteffen};
  }

  /// @brief Create a configuration for polynomial spline interpolation.
  /// @return `Spatial` configured with the polynomial spline method.
  [[nodiscard]] static constexpr auto polynomial() -> Spatial {
    return Spatial{Spline::kPolynomial};
  }

  /// @brief Update the `boundary_mode` setting.
  /// @param[in] mode New boundary mode.
  /// @return Updated `Spatial` instance with the new setting.
  [[nodiscard]] constexpr auto with_boundary_mode(BoundaryMode mode) const
      -> Spatial {
    auto copy = *this;
    copy.boundary_mode = mode;
    return copy;
  }

  /// @brief Update window size in the x direction.
  /// @param[in] size New window size in x direction.
  /// @return Updated `Spatial` instance with the new window size.
  [[nodiscard]] constexpr auto with_window_size_x(int size) const -> Spatial {
    auto copy = *this;
    copy.window_size_x = size;
    return copy;
  }

  /// @brief Update window size in the y direction.
  /// @param[in] size New window size in y direction.
  /// @return Updated `Spatial` instance with the new window size.
  [[nodiscard]] constexpr auto with_window_size_y(int size) const -> Spatial {
    auto copy = *this;
    copy.window_size_y = size;
    return copy;
  }
};

// Forward declarations
struct Bivariate;
struct Trivariate;
struct Quadrivariate;

}  // namespace windowed

/// @brief Traits to determine the number of additional axes for the
/// `Bivariate` (windowed) configuration type.
template <>
struct InterpolationTraits<windowed::Bivariate> {
  static constexpr size_t num_axes = 0;
};

/// @brief Traits to determine the number of additional axes for the
/// `Trivariate` (windowed) configuration type.
template <>
struct InterpolationTraits<windowed::Trivariate> {
  static constexpr size_t num_axes = 1;
};

/// @brief Traits to determine the number of additional axes for the
/// `Quadrivariate` (windowed) configuration type.
template <>
struct InterpolationTraits<windowed::Quadrivariate> {
  static constexpr size_t num_axes = 2;
};

namespace windowed {

/// @brief Base class for multi-dimensional windowed interpolation
/// configurations using the Curiously Recurring Template Pattern (CRTP).
/// @tparam Derived The derived configuration class.
template <typename Derived>
struct WindowBase : public Base<Spatial, Derived> {
  /// @brief Create a configuration for akima spline interpolation
  /// @return Configuration with akima spline interpolation method
  [[nodiscard]] static constexpr auto akima() -> Derived {
    return create_config(Spatial::akima(), AxisConfig::linear());
  }

  /// @brief Create a configuration for Akima periodic spline interpolation
  /// @return Configuration with Akima periodic spline interpolation method
  [[nodiscard]] static constexpr auto akima_periodic() -> Derived {
    return create_config(Spatial::akima_periodic(), AxisConfig::linear());
  }

  /// @brief Create a configuration for C spline interpolation
  /// @return Configuration with C spline interpolation method
  [[nodiscard]] static constexpr auto c_spline() -> Derived {
    return create_config(Spatial::c_spline(), AxisConfig::linear());
  }

  /// @brief Create a configuration for C spline not-a-knot interpolation
  /// @return Configuration with C spline not-a-knot interpolation method
  [[nodiscard]] static constexpr auto c_spline_not_a_knot() -> Derived {
    return create_config(Spatial::c_spline_not_a_knot(), AxisConfig::linear());
  }

  /// @brief Create a configuration for C spline periodic interpolation
  /// @return Configuration with C spline periodic interpolation method
  [[nodiscard]] static constexpr auto c_spline_periodic() -> Derived {
    return create_config(Spatial::c_spline_periodic(), AxisConfig::linear());
  }

  /// @brief Create a configuration for steffen spline interpolation
  /// @return Configuration with steffen spline interpolation method
  [[nodiscard]] static constexpr auto steffen() -> Derived {
    return create_config(Spatial::steffen(), AxisConfig::linear());
  }

  /// @brief Create a configuration for linear interpolation
  /// @return Configuration with linear interpolation method
  [[nodiscard]] static constexpr auto linear() -> Derived {
    return create_config(Spatial::linear(), AxisConfig::linear());
  }

  /// @brief Create a configuration for polynomial spline interpolation
  /// @return Configuration with polynomial spline interpolation method
  [[nodiscard]] static constexpr auto polynomial() -> Derived {
    return create_config(Spatial::polynomial(), AxisConfig::linear());
  }

  /// @brief Create a configuration for bilinear interpolation
  /// @return Configuration with bilinear interpolation method
  [[nodiscard]] static constexpr auto bilinear() -> Derived {
    return create_config(Spatial::bilinear(), AxisConfig::linear());
  }

  /// @brief Create a configuration for bicubic interpolation
  /// @return Configuration with bicubic interpolation method
  [[nodiscard]] static constexpr auto bicubic() -> Derived {
    return create_config(Spatial::bicubic(), AxisConfig::linear());
  }

 private:
  /// @brief Helper to create a configuration with the appropriate number
  /// of axes for the derived type.
  [[nodiscard]] static constexpr auto create_config(
      const Spatial& spatial, const AxisConfig& axis_config) -> Derived {
    constexpr size_t num_axes = InterpolationTraits<Derived>::num_axes;

    if constexpr (num_axes == 0) {
      // 2D case
      return Derived{spatial};
    } else if constexpr (num_axes == 1) {
      // 3D case
      return Derived{spatial, axis_config};
    } else if constexpr (num_axes == 2) {
      // 4D case
      return Derived{spatial, axis_config, axis_config};
    }
  }
};

/// @brief Modifiers for spatial settings in windowed interpolation
/// configurations.
/// @tparam Derived The derived configuration class.
template <typename Derived>
struct WindowSpatialModifiers {
  /// @brief Update window size in the x direction on the derived config.
  /// @param[in] size New window size in x direction.
  /// @return Updated derived configuration instance.
  [[nodiscard]] constexpr auto with_window_size_x(int size) const -> Derived {
    Derived result = static_cast<const Derived&>(*this);
    result.spatial = result.spatial.with_window_size_x(size);
    return result;
  }

  /// @brief Update window size in the y direction on the derived config.
  /// @param[in] size New window size in y direction.
  /// @return Updated derived configuration instance.
  [[nodiscard]] constexpr auto with_window_size_y(int size) const -> Derived {
    Derived result = static_cast<const Derived&>(*this);
    result.spatial = result.spatial.with_window_size_y(size);
    return result;
  }

  /// @brief Update the boundary mode on the derived config.
  /// @param[in] mode New boundary mode.
  /// @return Updated derived configuration instance.
  [[nodiscard]] constexpr auto with_boundary_mode(BoundaryMode mode) const
      -> Derived {
    Derived result = static_cast<const Derived&>(*this);
    result.spatial = result.spatial.with_boundary_mode(mode);
    return result;
  }
};

/// @brief Window interpolation configuration (2D only)
struct Bivariate : WindowBase<Bivariate>, WindowSpatialModifiers<Bivariate> {
  Spatial spatial;  /// Spatial interpolation configuration
  Common common;    /// Common interpolation configuration

  /// @brief Default constructor
  constexpr Bivariate() = default;

  /// @brief Constructor with spatial and common configurations
  /// @param[in] spatial Spatial interpolation configuration
  /// @param[in] common Common interpolation configuration
  constexpr explicit Bivariate(const Spatial& spatial,
                               const Common& common = {})
      : spatial(spatial), common(common) {}
};

/// @brief Windows interpolation configuration (3D only)
struct Trivariate : WindowBase<Trivariate>, WindowSpatialModifiers<Trivariate> {
  Spatial spatial;        /// Spatial interpolation configuration
  AxisConfig third_axis;  /// Third axis interpolation configuration
  Common common;          /// Common interpolation configuration

  /// @brief Default constructor
  constexpr Trivariate() = default;

  /// @brief Constructor with spatial, third axis, and common configurations
  /// @param[in] spatial Spatial interpolation configuration
  /// @param[in] third_axis Third axis interpolation configuration
  /// @param[in] common Common interpolation configuration
  constexpr Trivariate(const Spatial& spatial, const AxisConfig& third_axis,
                       const Common& common = {})
      : spatial(spatial), third_axis(third_axis), common(common) {}

  /// @brief Update third axis configuration
  /// @param[in] config New third axis configuration
  /// @return Updated TrivariateConfig instance
  [[nodiscard]] constexpr auto with_third_axis(const AxisConfig& config) const
      -> Trivariate {
    auto copy = *this;
    copy.third_axis = config;
    return copy;
  }
};

/// @brief Windows interpolation configuration (4D only)
struct Quadrivariate : WindowBase<Quadrivariate>,
                       WindowSpatialModifiers<Quadrivariate> {
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
  /// @param[in] common Common interpolation configuration
  constexpr Quadrivariate(const Spatial& spatial, const AxisConfig& third_axis,
                          const AxisConfig& fourth_axis,
                          const Common& common = {})
      : spatial(spatial),
        third_axis(third_axis),
        fourth_axis(fourth_axis),
        common(common) {}

  /// @brief Update third axis configuration
  /// @param[in] config New third axis configuration
  /// @return Updated QuadrivariateConfig instance
  [[nodiscard]] constexpr auto with_third_axis(const AxisConfig& config) const
      -> Quadrivariate {
    auto copy = *this;
    copy.third_axis = config;
    return copy;
  }

  /// @brief Update fourth axis configuration
  /// @param[in] config New fourth axis configuration
  /// @return Updated QuadrivariateConfig instance
  [[nodiscard]] constexpr auto with_fourth_axis(const AxisConfig& config) const
      -> Quadrivariate {
    auto copy = *this;
    copy.fourth_axis = config;
    return copy;
  }
};

}  // namespace windowed
}  // namespace pyinterp::pybind::config

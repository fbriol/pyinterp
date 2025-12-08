#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace pyinterp::pybind::config {

/// Method for interpolation along a single axis.
enum class AxisMethod : uint8_t {
  /// Nearest-neighbor interpolation
  kNearest,
  /// Linear interpolation
  kLinear,
};

/// @brief Parser for an axis interpolation method name.
/// @param[in] method_name Name of the interpolation method (case-sensitive)
/// @return Corresponding AxisMethod enum value
/// @throws std::invalid_argument if the method name is unknown
[[nodiscard]] inline auto parse_axis_method(std::string_view method_name)
    -> AxisMethod {
  if (method_name == "nearest") {
    return AxisMethod::kNearest;
  }
  if (method_name == "linear") {
    return AxisMethod::kLinear;
  }
  throw std::invalid_argument("Unknown axis interpolation method: " +
                              std::string(method_name));
}

/// @brief Configuration for a single-axis interpolation.
class AxisConfig {
 public:
  /// @brief Default constructor
  constexpr AxisConfig() = default;

  /// @brief Constructor with an explicit interpolation method
  /// @param[in] method Interpolation method to use for the axis
  constexpr explicit AxisConfig(AxisMethod method) : method_(method) {}

  /// @brief Create a configuration for linear interpolation.
  /// @return AxisConfig configured with the linear interpolation method
  [[nodiscard]] static constexpr auto linear() -> AxisConfig {
    return AxisConfig{AxisMethod::kLinear};
  }

  /// @brief Create a configuration for nearest-neighbour interpolation.
  /// @return AxisConfig configured with the nearest-neighbour method
  [[nodiscard]] static constexpr auto nearest() -> AxisConfig {
    return AxisConfig{AxisMethod::kNearest};
  }

  /// @brief Get the interpolation method for the axis.
  /// @return Interpolation method
  [[nodiscard]] constexpr auto method() const -> AxisMethod { return method_; }

 private:
  /// Interpolation method for the axis
  AxisMethod method_{AxisMethod::kLinear};
};

/// @brief Configuration for thread management
class ThreadConfig {
 public:
  /// Default constructor
  constexpr ThreadConfig() = default;

  /// @brief Get the num_threads setting.
  /// @return Number of threads to use (0 means use all available threads).
  [[nodiscard]] constexpr auto num_threads() const -> size_t {
    return num_threads_;
  }

  /// @brief Update the `num_threads` setting.
  /// @param[in] value New value for `num_threads`.
  /// @return Updated `ThreadConfig` instance with the new setting.
  [[nodiscard]] constexpr auto with_num_threads(size_t value) const
      -> ThreadConfig {
    auto copy = *this;
    copy.num_threads_ = value;
    return copy;
  }

 private:
  /// Number of threads to use (0 means use all available threads).
  size_t num_threads_{0};
};

/// Common configuration shared across all dimensions
class Common : public ThreadConfig {
 public:
  /// Default constructor
  constexpr Common() = default;

  /// @brief Get the bounds_error setting.
  /// @return Whether to throw on out-of-bounds access.
  [[nodiscard]] constexpr auto bounds_error() const -> bool {
    return bounds_error_;
  }

  /// @brief Update the `bounds_error` setting.
  /// @param[in] value New value for `bounds_error`.
  /// @return Updated `Common` instance with the new setting.
  [[nodiscard]] constexpr auto with_bounds_error(bool value) const -> Common {
    auto copy = *this;
    copy.bounds_error_ = value;
    return copy;
  }

  /// @brief Update the `num_threads` setting.
  /// @param[in] value New value for `num_threads`.
  /// @return Updated `Common` instance with the new setting.
  [[nodiscard]] constexpr auto with_num_threads(size_t value) const -> Common {
    auto copy = *this;
    static_cast<ThreadConfig&>(copy) = ThreadConfig::with_num_threads(value);
    return copy;
  }

 private:
  /// Whether to throw on out-of-bounds access.
  bool bounds_error_{false};
};

/// @brief Base class for multi-dimensional interpolation configurations
/// using the Curiously Recurring Template Pattern (CRTP).
/// @tparam FittingModel The type of the fitting model used for spatial
/// interpolation.
/// @tparam Derived The derived configuration class.
/// @details
/// This base provides common functionality for all configuration types and
/// helps reduce code duplication across derived configuration classes.
template <typename FittingModel, typename Derived>
class Base {
 protected:
  /// @brief Get a reference to the derived class
  [[nodiscard]] constexpr auto derived() -> Derived& {
    return static_cast<Derived&>(*this);
  }

  /// @brief Get a const reference to the derived class
  [[nodiscard]] constexpr auto derived() const -> const Derived& {
    return static_cast<const Derived&>(*this);
  }

 public:
  /// @brief Update the `bounds_error` setting on the derived config.
  /// @param[in] value New value for `bounds_error`.
  /// @return Updated derived configuration instance.
  [[nodiscard]] constexpr auto with_bounds_error(bool value) const -> Derived {
    auto copy = derived();
    copy.common_ = copy.common_.with_bounds_error(value);
    return copy;
  }

  /// @brief Update the `num_threads` setting on the derived config.
  /// @param[in] value New value for `num_threads`.
  /// @return Updated derived configuration instance.
  [[nodiscard]] constexpr auto with_num_threads(size_t value) const -> Derived {
    auto copy = derived();
    copy.common_ = copy.common_.with_num_threads(value);
    return copy;
  }

  /// @brief Update the spatial fitting model configuration held by the
  /// derived config.
  /// @param[in] config New spatial fitting model configuration.
  /// @return Updated derived configuration instance.
  [[nodiscard]] constexpr auto with_spatial(const FittingModel& config) const
      -> Derived {
    auto copy = derived();
    copy.spatial_ = config;
    return copy;
  }
};

/// @brief Traits to determine the number of additional axes for each
/// configuration type.
template <typename T>
struct InterpolationTraits;

}  // namespace pyinterp::pybind::config

#pragma once

#include <cmath>
#include <concepts>
#include <numbers>
#include <utility>

namespace pyinterp::math {

/// @brief Convert degrees to radians
/// @tparam T type of the value
/// @param[in] degrees Angle in degrees
/// @return Angle in radians
template <typename T>
[[nodiscard]] constexpr auto radians(const T &degrees) noexcept -> T {
  return degrees * std::numbers::pi_v<T> / T{180};
}

/// @brief Convert radians to degrees
/// @tparam T type of the value
/// @param[in] radians Angle in radians
/// @return Angle in degrees
template <typename T>
[[nodiscard]] constexpr auto degrees(const T &radians) noexcept -> T {
  return radians * T{180} / std::numbers::pi_v<T>;
}

/// @brief Square a number.
///
/// @tparam T type of the value
/// @param[in] x The value to be squared.
/// @return \f$x^2\f$
template <typename T>
[[nodiscard]] constexpr auto sqr(const T &x) noexcept -> T {
  return x * x;
}

/// @brief Compute the cosine of an angle in degrees
/// @tparam T type of the value
/// @param[in] x Angle in degrees
/// @return Cosine of the angle
template <typename T>
[[nodiscard]] constexpr auto cosd(const T &x) noexcept -> T {
  return std::cos(radians(x));
}

/// @brief Compute the sine of an angle in degrees
/// @tparam T type of the value
/// @param[in] x Angle in degrees
/// @return Sine of the angle
template <typename T>
[[nodiscard]] constexpr auto sind(const T &x) noexcept -> T {
  return std::sin(radians(x));
}

/// @brief Compute the sine and cosine of an angle in degrees
/// @tparam T type of the value
/// @param[in] x Angle in degrees
/// @return A pair containing the sine and cosine of the angle
template <typename T>
[[nodiscard]] constexpr auto sincosd(const T &x) noexcept -> std::pair<T, T> {
  const T rad = radians(x);
  return {std::sin(rad), std::cos(rad)};
}

/// @brief Return the normalized sinc function
/// @tparam T type of the value
/// @param[in] x Input value
/// @return sinc(x) = sin(pi*x)/(pi*x)
template <typename T>
constexpr auto sinc(const T &x) noexcept -> T {
  return x == 0 ? T(1)
                : std::sin(std::numbers::pi_v<T> * x) /
                      (std::numbers::pi_v<T> * x);
}

/// @brief Computes the remainder of the operation x/y
/// @tparam T type of the value
/// @param[in] x Dividend
/// @param[in] y Divisor
/// @return a result with the same sign as its second operand
template <std::integral T>
constexpr auto remainder(const T &x, const T &y) noexcept -> T {
  auto result = x % y;
  return result != 0 && (result ^ y) < 0 ? result + y : result;
}

/// @brief Computes the remainder of the operation x/y
/// @tparam T type of the value
/// @param[in] x Dividend
/// @param[in] y Divisor
/// @return a result with the same sign as its second operand
template <std::floating_point T>
constexpr auto remainder(const T &x, const T &y) noexcept -> T {
  auto result = std::remainder(x, y);
  if (result < T(0)) {
    result += y;
  }
  return result;
}

/// @brief Normalize a value to a given period
/// @tparam T type of the value
/// @param[in] x The value to normalize.
/// @param[in] min Minimum value of the period
/// @param[in] period Period value
/// @return the value reduced to the range [min, period + min[
template <typename T>
constexpr auto normalize_period(const T &x, const T &min,
                                const T &period) noexcept -> T {
  return remainder(x - min, period) + min;
}

/// @brief Compare two integral values for equality within a given epsilon
/// @tparam T Integral type
/// @param[in] a first value
/// @param[in] b second value
/// @param[in] epsilon maximum allowed difference
/// @return true if the values are considered equal
template <std::integral T>
constexpr auto is_same(const T &a, const T &b, const T &epsilon) noexcept
    -> bool {
  return std::abs(a - b) <= epsilon;
}

/// @brief Compare two floating-point values for equality within a given epsilon
/// @tparam T Floating-point type
/// @param[in] a first value
/// @param[in] b second value
/// @param[in] epsilon maximum allowed difference
/// @return true if the values are considered equal
template <std::floating_point T>
constexpr auto is_same(const T &a, const T &b, const T &epsilon) noexcept
    -> bool {
  const auto diff = std::fabs(a - b);
  return diff <= epsilon ||
         diff < std::fmax(std::fabs(a), std::fabs(b)) * epsilon;
}

/// @brief Check if a value is almost zero within a given epsilon
/// @tparam T Floating point type
/// @param[in] a The value to check.
/// @param[in] epsilon maximum allowed difference (defaults to machine epsilon)
/// @return true if the value is considered almost zero
template <std::floating_point T>
constexpr auto is_almost_zero(
    T a, T epsilon = std::numeric_limits<T>::epsilon()) noexcept -> bool {
  return std::abs(a) <= epsilon;
}

}  // namespace pyinterp::math

#pragma once

#include <nanobind/nanobind.h>

#include "pyinterp/dateutils.hpp"
#include "pyinterp/period.hpp"
#include "pyinterp/pybind/numpy.hpp"

namespace pyinterp::pybind {

/// @brief A Period with associated datetime64 resolution.
class Period : public pyinterp::Period {
 public:
  /// @brief Create a null period with default resolution (datetime64[ns]).
  constexpr Period() = default;

  /// @brief Create a period with specified begin, end and resolution.
  /// @param[in] begin Start of the period (inclusive).
  /// @param[in] end End of the period.
  /// @param[in] resolution The numpy dtype resolution for this period.
  /// @param[in] within If true, end is inclusive; if false, end is exclusive.
  constexpr Period(int64_t begin, int64_t end, dateutils::DType resolution,
                   bool within = true) noexcept
      : pyinterp::Period(begin, end, within), resolution_(resolution) {}

  /// @brief Create a period with specified begin, end and resolution.
  /// @param[in] begin Start of the period (inclusive).
  /// @param[in] end End of the period.
  /// @param[in] within If true, end is inclusive; if false, end is exclusive.
  Period(const nanobind::object& begin, const nanobind::object& end,
         bool within);

  /// @brief Equality comparison with resolution promotion.
  [[nodiscard]] auto operator==(const Period& rhs) const -> bool;

  /// @brief Three-way comparison with resolution promotion.
  [[nodiscard]] auto operator<=>(const Period& rhs) const
      -> std::strong_ordering;

  /// @brief Check if a point is contained within the period.
  /// @param[in] point A numpy.datetime64 scalar.
  /// @return true if the point is within the period, false otherwise.
  [[nodiscard]] auto contains(const nanobind::object& point) const -> bool;

  /// @brief Check if this period fully conatins (or equals) the other period.
  /// @param[in] other The other period to check containment.
  /// @return true if this period contains the other, false otherwise.
  [[nodiscard]] auto contains(const Period& other) const -> bool;

  /// @brief Check if the entire period is after the given point
  /// (point < begin).
  /// @param[in] point A numpy.datetime64 scalar.
  /// @return true if the period is after the point, false otherwise.
  [[nodiscard]] auto is_after(const nanobind::object& point) const -> bool;

  /// @brief Check if the entire period is before the given point
  /// (last < point).
  /// @param[in] point A numpy.datetime64 scalar.
  /// @return true if the period is before the point, false otherwise.
  [[nodiscard]] auto is_before(const nanobind::object& point) const -> bool;

  /// @brief Check if the given date is within tolerance of the period.
  /// @param[in] date A numpy.datetime64 scalar.
  /// @param[in] tolerance The tolerance margin (inclusive) as a
  /// numpy.timedelta64 scalar.
  /// @return true if the date is within tolerance, false otherwise.
  [[nodiscard]] auto is_close(const nanobind::object& date,
                              const nanobind::object& tolerance) const -> bool;

  /// @brief Check if this period intersects with another period.
  /// @param[in] other The other period to check intersection with.
  /// @return true if the periods intersect, false otherwise.
  [[nodiscard]] auto intersects(const Period& other) const -> bool;

  /// @brief Check if this period is adjacent to another period.
  /// @param[in] other The other period to check adjacency with.
  /// @return true if the periods are adjacent, false otherwise.
  [[nodiscard]] auto is_adjacent(const Period& other) const -> bool;

  /// @brief Get the intersection of this period with another period.
  /// @param[in] other The other period to intersect with.
  /// @return The intersection period.
  [[nodiscard]] auto intersection(const Period& other) const -> Period;

  /// @brief Merge this period with another period.
  /// @param[in] other The other period to merge with.
  /// @return The merged period.
  [[nodiscard]] auto merge(const Period& other) const -> Period;

  /// @brief Extend the period to include the given point.
  /// @param[in] point A numpy.datetime64 scalar.
  /// @return A new Period extended to include the point.
  [[nodiscard]] auto extend(const nanobind::object& point) const -> Period;

  /// @brief Shift the period by a given timedelta.
  /// @param[in] offset A numpy.timedelta64 scalar.
  /// @return A new Period shifted by the offset.
  [[nodiscard]] auto shift(const nanobind::object& offset) const -> Period;

  /// @brief Convert the period to a string representation.
  /// @return String representation of the period.
  [[nodiscard]] explicit operator std::string() const;

  /// @brief Get the state of the object for pickling.
  /// @return A tuple representing the state of the object.
  [[nodiscard]] auto getstate() const
      -> std::tuple<int64_t, int64_t, std::string>;

  /// @brief Set the state of the object from pickling.
  /// @param[in] state A tuple representing the state of the object.
  static auto setstate(const std::tuple<int64_t, int64_t, std::string>& state)
      -> Period;

 private:
  /// The resolution of the period.
  dateutils::DType resolution_{};

  /// Create a Period from a base Period and a resolution.
  constexpr Period(const pyinterp::Period& period,
                   const dateutils::DType& resolution)
      : pyinterp::Period(period), resolution_(resolution) {}

  /// @brief Get the finer (more precise) resolution between two DTypes.
  /// @param[in] a First dtype.
  /// @param[in] b Second dtype.
  /// @return The finer resolution dtype.
  [[nodiscard]] static constexpr auto finer_resolution(
      const dateutils::DType& a, const dateutils::DType& b) noexcept
      -> dateutils::DType {
    return a.resolution() >= b.resolution() ? a : b;
  }

  /// @brief Convert this period to a different resolution.
  /// @param[in] target The target dtype resolution.
  /// @return A new Period converted to the target resolution.
  /// @throws std::overflow_error if conversion would overflow int64_t.
  [[nodiscard]] auto convert_to(const dateutils::DType& target) const
      -> Period {
    if (resolution_ == target) {
      return *this;
    }
    return Period{dateutils::convert(begin, resolution_, target),
                  dateutils::convert(last, resolution_, target), target};
  }
};

/// @brief Initialize the Period classes in the given module.
/// @param[in,out] m The nanobind module to initialize.
auto init_period(nanobind::module_& m) -> void;

}  // namespace pyinterp::pybind

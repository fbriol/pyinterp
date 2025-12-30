#include "pyinterp/pybind/period.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>

#include <string>

#include "pyinterp/period.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;

namespace pyinterp::pybind {

// Convert datetime64 scalar to int64_t
inline auto convert_datetime64(const std::string& param_name,
                               const nb::object& datetime64)
    -> std::pair<dateutils::DType, int64_t> {
  if (datetime64.is_none()) {
    throw std::invalid_argument(param_name + " cannot be None");
  }

  // If the object has no dtype attribute, it's not a numpy object
  if (!nb::hasattr(datetime64, "dtype")) {
    throw std::invalid_argument(param_name +
                                " must be a numpy.datetime64 scalar");
  }

  // Retrieve and validate the dtype of the input datetime64
  auto cxx_dtype = retrieve_dtype(param_name, datetime64);
  if (cxx_dtype.datetype() != dateutils::DType::DateType::kDatetime64) {
    throw std::invalid_argument(param_name +
                                " must be a numpy.datetime64 scalar");
  }

  // Convert scalar to int64 via .view() then extract the value with item()
  auto value =
      nb::cast<int64_t>(datetime64.attr("view")("int64").attr("item")());
  // Convert to target resolution if needed
  return {cxx_dtype, value};
}

// Convert timedelta64 scalar to int64_t
inline auto convert_timedelta64(const std::string& param_name,
                                const nb::object& timedelta64)
    -> std::pair<dateutils::DType, int64_t> {
  if (timedelta64.is_none()) {
    throw std::invalid_argument(param_name + " cannot be None");
  }

  // If the object has no dtype attribute, it's not a numpy object
  if (!nb::hasattr(timedelta64, "dtype")) {
    throw std::invalid_argument(param_name +
                                " must be a numpy.timedelta64 scalar");
  }

  // Retrieve and validate the dtype of the input timedelta64
  auto cxx_dtype = retrieve_dtype(param_name, timedelta64);
  if (cxx_dtype.datetype() != dateutils::DType::DateType::kTimedelta64) {
    throw std::invalid_argument(param_name +
                                " must be a numpy.timedelta64 scalar");
  }

  // Convert scalar to int64 via .view() then extract the value with item()
  auto value =
      nb::cast<int64_t>(timedelta64.attr("view")("int64").attr("item")());
  // Convert to target resolution if needed
  return {cxx_dtype, value};
}

Period::Period(const nanobind::object& begin, const nanobind::object& end,
               bool within) {
  auto [begin_resolution, begin_value] = convert_datetime64("begin", begin);
  auto [end_resolution, end_value] = convert_datetime64("end", end);
  const auto target = finer_resolution(begin_resolution, end_resolution);

  *this = Period(dateutils::convert(begin_value, begin_resolution, target),
                 dateutils::convert(end_value, end_resolution, target), target,
                 within);
}

auto Period::operator==(const Period& rhs) const -> bool {
  if (resolution_ == rhs.resolution_) {
    return begin == rhs.begin && last == rhs.last;
  }
  const auto target = finer_resolution(resolution_, rhs.resolution_);
  const auto lhs_conv = convert_to(target);
  const auto rhs_conv = rhs.convert_to(target);
  return lhs_conv.begin == rhs_conv.begin && lhs_conv.last == rhs_conv.last;
}

auto Period::operator<=>(const Period& rhs) const -> std::strong_ordering {
  if (resolution_ == rhs.resolution_) {
    if (auto cmp = begin <=> rhs.begin; cmp != 0) {
      return cmp;
    }
    return last <=> rhs.last;
  }
  const auto target = finer_resolution(resolution_, rhs.resolution_);
  const auto lhs_conv = convert_to(target);
  const auto rhs_conv = rhs.convert_to(target);
  if (auto cmp = lhs_conv.begin <=> rhs_conv.begin; cmp != 0) {
    return cmp;
  }
  return lhs_conv.last <=> rhs_conv.last;
}

auto Period::contains(const nanobind::object& point) const -> bool {
  auto [resolution, value] = convert_datetime64("point", point);
  const auto target = finer_resolution(resolution_, resolution);
  const auto self_conv = convert_to(target);
  const auto converted_value = dateutils::convert(value, resolution, target);
  return static_cast<const pyinterp::Period&>(self_conv).contains(
      converted_value);
}

auto Period::contains(const Period& other) const -> bool {
  const auto target = finer_resolution(resolution_, other.resolution_);
  const auto self_conv = convert_to(target);
  const auto other_conv = other.convert_to(target);
  return static_cast<const pyinterp::Period&>(self_conv).contains(
      static_cast<const pyinterp::Period&>(other_conv));
}

auto Period::is_after(const nanobind::object& point) const -> bool {
  auto [resolution, value] = convert_datetime64("point", point);
  const auto target = finer_resolution(resolution_, resolution);
  const auto self_conv = convert_to(target);
  const auto converted_value = dateutils::convert(value, resolution, target);
  return static_cast<const pyinterp::Period&>(self_conv).is_after(
      converted_value);
}

auto Period::is_before(const nanobind::object& point) const -> bool {
  auto [resolution, value] = convert_datetime64("point", point);
  const auto target = finer_resolution(resolution_, resolution);
  const auto self_conv = convert_to(target);
  const auto converted_value = dateutils::convert(value, resolution, target);
  return static_cast<const pyinterp::Period&>(self_conv).is_before(
      converted_value);
}

auto Period::is_close(const nanobind::object& date,
                      const nanobind::object& tolerance) const -> bool {
  auto [date_resolution, date_value] = convert_datetime64("date", date);
  auto [tolerance_resolution, tolerance_value] =
      convert_timedelta64("tolerance", tolerance);
  const auto target = finer_resolution(
      finer_resolution(resolution_, date_resolution), tolerance_resolution);
  const auto self_conv = convert_to(target);
  const auto converted_date =
      dateutils::convert(date_value, date_resolution, target);
  const auto converted_tolerance =
      dateutils::convert(tolerance_value, tolerance_resolution, target);
  return static_cast<const pyinterp::Period&>(self_conv).is_close(
      converted_date, converted_tolerance);
}

auto Period::intersects(const Period& other) const -> bool {
  const auto target = finer_resolution(resolution_, other.resolution_);
  return static_cast<const pyinterp::Period&>(convert_to(target))
      .intersects(
          static_cast<const pyinterp::Period&>(other.convert_to(target)));
}

auto Period::is_adjacent(const Period& other) const -> bool {
  const auto target = finer_resolution(resolution_, other.resolution_);
  return static_cast<const pyinterp::Period&>(convert_to(target))
      .is_adjacent(
          static_cast<const pyinterp::Period&>(other.convert_to(target)));
}

auto Period::intersection(const Period& other) const -> Period {
  const auto target = finer_resolution(resolution_, other.resolution_);
  const auto self_conv = convert_to(target);
  const auto other_conv = other.convert_to(target);
  return Period{static_cast<const pyinterp::Period&>(self_conv).intersection(
                    static_cast<const pyinterp::Period&>(other_conv)),
                target};
}

auto Period::merge(const Period& other) const -> Period {
  const auto target = finer_resolution(resolution_, other.resolution_);
  const auto self_conv = convert_to(target);
  const auto other_conv = other.convert_to(target);
  return Period{static_cast<const pyinterp::Period&>(self_conv).merge(
                    static_cast<const pyinterp::Period&>(other_conv)),
                target};
}

auto Period::extend(const nanobind::object& point) const -> Period {
  auto [resolution, value] = convert_datetime64("point", point);
  const auto target = finer_resolution(resolution_, resolution);
  const auto self_conv = convert_to(target);
  const auto converted_value = dateutils::convert(value, resolution, target);
  return Period{
      static_cast<const pyinterp::Period&>(self_conv).extend(converted_value),
      target};
}

auto Period::shift(const nanobind::object& offset) const -> Period {
  auto [resolution, value] = convert_timedelta64("offset", offset);
  const auto target = finer_resolution(resolution_, resolution);
  const auto self_conv = convert_to(target);
  const auto converted_value = dateutils::convert(value, resolution, target);
  return Period{
      static_cast<const pyinterp::Period&>(self_conv).shift(converted_value),
      target};
}

Period::operator std::string() const {
  return std::format("[{}, {})",
                     dateutils::datetime64_to_string(begin, resolution_),
                     dateutils::datetime64_to_string(last, resolution_));
}

auto Period::getstate() const -> std::tuple<int64_t, int64_t, std::string> {
  return {begin, last, std::string(resolution_)};
}

auto Period::setstate(const std::tuple<int64_t, int64_t, std::string>& state)
    -> Period {
  auto begin = std::get<0>(state);
  auto last = std::get<1>(state);
  auto resolution = dateutils::DType(std::get<2>(state));
  return Period{begin, last, resolution, true};
}

constexpr const char* const kPeriodInit = R"(
A Period object representing a time interval.

Args:
    begin: A numpy.datetime64 scalar representing the start of the period.
    end: A numpy.datetime64 scalar representing the end of the period.
    within: If True, end is inclusive; if False, end is exclusive.
)";

constexpr const char* const kPeriodContains = R"(
Check if a point is within the period.

Args:
    point_or_period: A numpy.datetime64 scalar or another Period.
Returns:
    True if the point/period is within this period, False otherwise.
)";

constexpr const char* const kPeriodIsAfter = R"(
Check if the entire period is after the given point.

Args:
    point: A numpy.datetime64 scalar.
Returns:
    True if the period is after the point, False otherwise.
)";

constexpr const char* const kPeriodIsBefore = R"(
Check if the entire period is before the given point.

Args:
    point: A numpy.datetime64 scalar.
Returns:
    True if the period is before the point, False otherwise.
)";

constexpr const char* const kPeriodIsClose = R"(
Check if the given date is within tolerance of the period.

Args:
    date: A numpy.datetime64 scalar.
    tolerance: The tolerance margin (inclusive) as a numpy.timedelta64 scalar.
Returns:
    True if the date is within tolerance, False otherwise.
)";

constexpr const char* const kPeriodIntersects = R"(
Check if this period intersects with another period.

Args:
    other: The other Period to check intersection with.
Returns:
    True if the periods intersect, False otherwise.
)";

constexpr const char* const kPeriodIsAdjacent = R"(
Check if this period is adjacent to another period.

Args:
    other: The other Period to check adjacency with.
Returns:
    True if the periods are adjacent, False otherwise.
)";

constexpr const char* const kPeriodIntersection = R"(
Get the intersection of this period with another period.

Args:
    other: The other Period to intersect with.
Returns:
    The intersection Period.
)";

constexpr const char* const kPeriodMerge = R"(
Merge this period with another period.

Args:
    other: The other Period to merge with.
Returns:
    The merged Period.
)";

constexpr const char* const kPeriodExtend = R"(
Extend the period to include the given point.

Args:
    point: A numpy.datetime64 scalar.
Returns:
    A new Period extended to include the point.
)";

constexpr const char* const kPeriodShift = R"(
Shift the period by a given timedelta.

Args:
    offset: A numpy.timedelta64 scalar.
Returns:
    A new Period shifted by the offset.
)";

auto init_period(nanobind::module_& m) -> void {
  nb::class_<Period>(m, "Period", "A Period.")
      .def(nb::init<const nb::object&, const nb::object&, bool>(), kPeriodInit,
           "begin"_a, "end"_a, "within"_a = true)

      .def("__eq__", &Period::operator==,
           "Equality comparison with resolution "
           "promotion.")
      .def(
          "__ne__",
          [](const Period& self, const Period& other) {
            return !(self == other);
          },
          "Inequality comparison with resolution promotion.")
      .def(
          "__lt__",
          [](const Period& self, const Period& other) {
            return (self <=> other) == std::strong_ordering::less;
          },
          "Less-than comparison with resolution promotion.")
      .def(
          "__le__",
          [](const Period& self, const Period& other) {
            auto cmp = self <=> other;
            return cmp == std::strong_ordering::less ||
                   cmp == std::strong_ordering::equal;
          },
          "Less-than-or-equal comparison with resolution promotion.")
      .def(
          "__gt__",
          [](const Period& self, const Period& other) {
            return (self <=> other) == std::strong_ordering::greater;
          },
          "Greater-than comparison with resolution promotion.")
      .def(
          "__ge__",
          [](const Period& self, const Period& other) {
            auto cmp = self <=> other;
            return cmp == std::strong_ordering::greater ||
                   cmp == std::strong_ordering::equal;
          },
          "Greater-than-or-equal comparison with resolution promotion.")

      .def("__getstate__", &Period::getstate, "Get the state for pickling.")
      .def(
          "__setstate__",
          [](Period& self,
             const std::tuple<int64_t, int64_t, std::string>& state) {
            nanobind::gil_scoped_release release;
            return new (&self) Period(Period::setstate(state));
          },
          nanobind::arg("state"),
          "Set the state of the instance from pickling.")

      .def(
          "contains",
          [](const Period& self, const nb::object& point_or_period) {
            // Check if the argument is a Period object
            if (nb::isinstance<Period>(point_or_period)) {
              return self.contains(nb::cast<const Period&>(point_or_period));
            }
            // Otherwise, treat it as a datetime64 scalar
            return self.contains(point_or_period);
          },
          kPeriodContains, "point_or_period"_a)

      .def(
          "is_after",
          [](const Period& self, const nb::object& point) {
            return self.is_after(point);
          },
          kPeriodIsAfter, "point"_a)

      .def(
          "is_before",
          [](const Period& self, const nb::object& point) {
            return self.is_before(point);
          },
          kPeriodIsBefore, "point"_a)

      .def(
          "is_close",
          [](const Period& self, const nb::object& date,
             const nb::object& tolerance) {
            return self.is_close(date, tolerance);
          },
          kPeriodIsClose, "date"_a, "tolerance"_a)

      .def("intersects", &Period::intersects, kPeriodIntersects, "other"_a)

      .def("is_adjacent", &Period::is_adjacent, kPeriodIsAdjacent, "other"_a)

      .def("intersection", &Period::intersection, kPeriodIntersection,
           "other"_a)

      .def("merge", &Period::merge, kPeriodMerge, "other"_a)

      .def("extend", &Period::extend, kPeriodExtend, "point"_a)

      .def("shift", &Period::shift, kPeriodShift, "offset"_a)

      .def(
          "__str__",
          [](const Period& self) { return static_cast<std::string>(self); },
          "String representation of the Period.");
}

}  // namespace pyinterp::pybind

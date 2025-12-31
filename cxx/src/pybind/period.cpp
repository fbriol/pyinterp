#include "pyinterp/pybind/period.hpp"

#include <nanobind/make_iterator.h>
#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>

#include <string>
#include <utility>
#include <vector>

#include "pyinterp/dateutils.hpp"
#include "pyinterp/period.hpp"
#include "pyinterp/pybind/ndarray_serialization.hpp"
#include "pyinterp/pybind/numpy.hpp"

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
  const auto target =
      dateutils::finer_resolution(begin_resolution, end_resolution);

  *this = Period(dateutils::convert(begin_value, begin_resolution, target),
                 dateutils::convert(end_value, end_resolution, target), target,
                 within);
}

auto Period::operator==(const Period& rhs) const -> bool {
  if (resolution_ == rhs.resolution_) {
    return begin == rhs.begin && last == rhs.last;
  }
  const auto target = dateutils::finer_resolution(resolution_, rhs.resolution_);
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
  const auto target = dateutils::finer_resolution(resolution_, rhs.resolution_);
  const auto lhs_conv = convert_to(target);
  const auto rhs_conv = rhs.convert_to(target);
  if (auto cmp = lhs_conv.begin <=> rhs_conv.begin; cmp != 0) {
    return cmp;
  }
  return lhs_conv.last <=> rhs_conv.last;
}

auto Period::contains(const nanobind::object& point) const -> bool {
  auto [resolution, value] = convert_datetime64("point", point);
  const auto target = dateutils::finer_resolution(resolution_, resolution);
  const auto self_conv = convert_to(target);
  const auto converted_value = dateutils::convert(value, resolution, target);
  return static_cast<const pyinterp::Period&>(self_conv).contains(
      converted_value);
}

auto Period::contains(const Period& other) const -> bool {
  const auto target =
      dateutils::finer_resolution(resolution_, other.resolution_);
  const auto self_conv = convert_to(target);
  const auto other_conv = other.convert_to(target);
  return static_cast<const pyinterp::Period&>(self_conv).contains(
      static_cast<const pyinterp::Period&>(other_conv));
}

auto Period::is_after(const nanobind::object& point) const -> bool {
  auto [resolution, value] = convert_datetime64("point", point);
  const auto target = dateutils::finer_resolution(resolution_, resolution);
  const auto self_conv = convert_to(target);
  const auto converted_value = dateutils::convert(value, resolution, target);
  return static_cast<const pyinterp::Period&>(self_conv).is_after(
      converted_value);
}

auto Period::is_before(const nanobind::object& point) const -> bool {
  auto [resolution, value] = convert_datetime64("point", point);
  const auto target = dateutils::finer_resolution(resolution_, resolution);
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
  const auto target = dateutils::finer_resolution(
      dateutils::finer_resolution(resolution_, date_resolution),
      tolerance_resolution);
  const auto self_conv = convert_to(target);
  const auto converted_date =
      dateutils::convert(date_value, date_resolution, target);
  const auto converted_tolerance =
      dateutils::convert(tolerance_value, tolerance_resolution, target);
  return static_cast<const pyinterp::Period&>(self_conv).is_close(
      converted_date, converted_tolerance);
}

auto Period::intersects(const Period& other) const -> bool {
  const auto target =
      dateutils::finer_resolution(resolution_, other.resolution_);
  return static_cast<const pyinterp::Period&>(convert_to(target))
      .intersects(
          static_cast<const pyinterp::Period&>(other.convert_to(target)));
}

auto Period::is_adjacent(const Period& other) const -> bool {
  const auto target =
      dateutils::finer_resolution(resolution_, other.resolution_);
  return static_cast<const pyinterp::Period&>(convert_to(target))
      .is_adjacent(
          static_cast<const pyinterp::Period&>(other.convert_to(target)));
}

auto Period::intersection(const Period& other) const -> std::optional<Period> {
  const auto target =
      dateutils::finer_resolution(resolution_, other.resolution_);
  const auto self_conv = convert_to(target);
  const auto other_conv = other.convert_to(target);
  const auto intersection =
      static_cast<const pyinterp::Period&>(self_conv).intersection(
          static_cast<const pyinterp::Period&>(other_conv));
  if (intersection.is_null()) {
    return std::nullopt;
  }
  return Period{intersection, target};
}

auto Period::merge(const Period& other) const -> std::optional<Period> {
  const auto target =
      dateutils::finer_resolution(resolution_, other.resolution_);
  const auto self_conv = convert_to(target);
  const auto other_conv = other.convert_to(target);
  const auto merged = static_cast<const pyinterp::Period&>(self_conv).merge(
      static_cast<const pyinterp::Period&>(other_conv));
  if (merged.is_null()) {
    return std::nullopt;
  }
  return Period{merged, target};
}

auto Period::extend(const nanobind::object& point) const -> Period {
  auto [resolution, value] = convert_datetime64("point", point);
  const auto target = dateutils::finer_resolution(resolution_, resolution);
  const auto self_conv = convert_to(target);
  const auto converted_value = dateutils::convert(value, resolution, target);
  return Period{
      static_cast<const pyinterp::Period&>(self_conv).extend(converted_value),
      target};
}

auto Period::shift(const nanobind::object& offset) const -> Period {
  auto [resolution, value] = convert_timedelta64("offset", offset);
  const auto target = dateutils::finer_resolution(resolution_, resolution);
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

PeriodList::PeriodList(const nanobind::list& periods) : pyinterp::PeriodList() {
  if (periods.is_none()) {
    return;
  }
  reserve(periods.size());
  for (const auto& item : periods) {
    auto period = nb::cast<Period>(item);
    append(period);
  }
}

auto PeriodList::append(const Period& period) -> void {
  if (empty()) {
    resolution_ = period.resolution();
    pyinterp::PeriodList::push_back(
        static_cast<const pyinterp::Period&>(period));
  } else if (period.resolution() == resolution_) {
    pyinterp::PeriodList::push_back(
        static_cast<const pyinterp::Period&>(period));
  } else {
    pyinterp::PeriodList::push_back(
        static_cast<const pyinterp::Period&>(period.convert_to(resolution_)));
  }
}

auto PeriodList::setitem(size_t index, const Period& period) -> void {
  if (period.resolution() == resolution_) {
    pyinterp::PeriodList::operator[](index) =
        static_cast<const pyinterp::Period&>(period);
  } else {
    pyinterp::PeriodList::operator[](index) =
        static_cast<const pyinterp::Period&>(period.convert_to(resolution_));
  }
}

auto PeriodList::is_close(const nanobind::object& date,
                          const nanobind::object& tolerance) const -> bool {
  auto [date_resolution, date_value] = convert_datetime64("date", date);
  auto [tolerance_resolution, tolerance_value] =
      convert_timedelta64("tolerance", tolerance);
  const auto target = dateutils::finer_resolution(
      dateutils::finer_resolution(resolution_, date_resolution),
      tolerance_resolution);
  const auto converted_date =
      dateutils::convert(date_value, date_resolution, target);
  const auto converted_tolerance =
      dateutils::convert(tolerance_value, tolerance_resolution, target);
  return pyinterp::PeriodList::is_close(converted_date, converted_tolerance);
}

auto PeriodList::find_containing_index(const nanobind::object& date) const
    -> int64_t {
  auto [resolution, value] = convert_datetime64("date", date);
  const auto target = dateutils::finer_resolution(resolution_, resolution);
  const auto converted_value = dateutils::convert(value, resolution, target);
  return pyinterp::PeriodList::find_containing_index(converted_value);
}

auto PeriodList::find_containing(const nanobind::object& date) const
    -> std::optional<Period> {
  const auto index = find_containing_index(date);
  if (index == -1) {
    return std::nullopt;
  }
  return Period{(*this)[static_cast<size_t>(index)], resolution_};
}

auto PeriodList::getstate() const -> nanobind::tuple {
  serialization::Writer state;
  state.write(kMagicNumber);
  state.write(static_cast<uint8_t>(resolution_.resolution()));
  state.write(static_cast<const std::vector<pyinterp::Period>&>(*this));
  return nanobind::make_tuple(writer_to_ndarray(std::move(state)));
}

auto PeriodList::setstate(const nanobind::tuple& state) -> PeriodList {
  if (state.size() != 1) {
    throw std::invalid_argument("Invalid state: expected a tuple of size 1");
  }
  auto array = nanobind::cast<NanobindArray1DUInt8>(state[0]);
  auto reader = reader_from_ndarray(array);
  auto magic_number = reader.read<uint32_t>();
  if (magic_number != kMagicNumber) {
    throw std::invalid_argument("Invalid axis state (bad magic number).");
  }
  auto resolution =
      static_cast<dateutils::DType::Resolution>(reader.read<uint8_t>());
  pyinterp::PeriodList periods(reader.read_vector<pyinterp::Period>());
  return {
      std::move(periods),
      dateutils::DType(dateutils::DType::DateType::kDatetime64, resolution)};
}

PeriodList::operator std::string() const {
  return std::format("PeriodList(resolution='{}', size={})",
                     std::string(resolution_), size());
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

constexpr const char* const kPeriodListInit = R"(
A PeriodList object representing a list of Periods.

Args:
    periods: If None (or not provided), creates an empty PeriodList. Otherwise,
        a Python list of Period objects to initialize the PeriodList.
)";

constexpr const char* const kPeriodListAppend = R"(
Append a Period to the PeriodList.

Args:
    period: The Period to append.
)";

constexpr const char* const kPeriodListIsClose = R"(
Check if the given date is within tolerance of any period in the list.

Args:
    date: A numpy.datetime64 scalar.
    tolerance: The tolerance margin as a numpy.timedelta64 scalar.
Returns:
    True if the date is within tolerance of any period, False otherwise.
)";

constexpr const char* const kPeriodListFindContaining = R"(
Find the Period containing the given date.

Args:
    date: A numpy.datetime64 scalar.
Returns:
    The containing Period, or None if not found.
)";

constexpr const char* const kPeriodListIsSortedAndDisjoint = R"(
Check if the periods in the list are sorted and non-overlapping.

Returns:
    True if the periods are sorted and disjoint, False otherwise.
)";

constexpr const char* const kPeriodListTotalDuration = R"(
Calculate the total duration covered by the periods in the list.

Returns:
    The total duration as a numpy.timedelta64 scalar.
)";

auto init_period(nanobind::module_& m) -> void {
  nb::class_<Period>(m, "Period", "A Period.")
      .def(nb::init<const nb::object&, const nb::object&, bool>(), kPeriodInit,
           "begin"_a, "end"_a, "within"_a = true)

      .def_prop_ro(
          "begin",
          [](const Period& self) {
            return make_scalar(self.begin, self.resolution());
          },
          "The start date of the period.")
      .def_prop_ro(
          "last",
          [](const Period& self) {
            return make_scalar(self.last, self.resolution());
          },
          "The last date of the period (inclusive).")
      .def_prop_ro(
          "resolution",
          [](const Period& self) { return to_dtype(self.resolution()); },
          "The resolution dtype of the period.")

      .def(
          "end",
          [](const Period& self) {
            return make_scalar(self.end(), self.resolution());
          },
          "Get the exclusive end date of the period (one past the last "
          "included date).")
      .def(
          "duration",
          [](const Period& self) {
            return make_scalar(self.duration(),
                               self.resolution().as_timedelta64());
          },
          "Get the duration of the period.")

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
            new (&self) Period(Period::setstate(state));
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

  nb::class_<PeriodList>(m, "PeriodList", "A list of Period objects.")
      .def(nb::init<nb::list>(), kPeriodListInit, "periods"_a = nb::none())
      .def(
          "append",
          [](PeriodList& self, const Period& period) { self.append(period); },
          kPeriodListAppend, "period"_a)
      .def(
          "is_close",
          [](const PeriodList& self, const nb::object& date,
             const nb::object& tolerance) {
            return self.is_close(date, tolerance);
          },
          kPeriodListIsClose, "date"_a, "tolerance"_a)
      .def(
          "find_containing",
          [](const PeriodList& self, const nb::object& date) {
            return self.find_containing(date);
          },
          kPeriodListFindContaining, "date"_a)
      .def("is_sorted_and_disjoint", &PeriodList::is_sorted_and_disjoint,
           kPeriodListIsSortedAndDisjoint)
      .def(
          "total_duration",
          [](const PeriodList& self) {
            auto duration = self.total_duration();
            return make_scalar(duration, self.resolution().as_timedelta64());
          },
          kPeriodListTotalDuration)
      .def("__getstate__", &PeriodList::getstate, "Get the state for pickling.")
      .def(
          "__setstate__",
          [](PeriodList& self, const nb::tuple& state) {
            new (&self) PeriodList(PeriodList::setstate(state));
          },
          nanobind::arg("state"),
          "Set the state of the instance from pickling.")
      .def(
          "__str__",
          [](const PeriodList& self) { return static_cast<std::string>(self); },
          "String representation of the PeriodList.")
      .def(
          "insert",
          [](PeriodList& self, int64_t index, const Period& period) {
            if (index < 0) {
              index += static_cast<int64_t>(self.size());
            }
            if (index < 0 || static_cast<size_t>(index) > self.size()) {
              throw nb::index_error();
            }
            self.insert(static_cast<const pyinterp::PeriodList&>(self).begin() +
                            static_cast<ptrdiff_t>(index),
                        static_cast<const pyinterp::Period&>(period));
          },
          "index"_a, "period"_a)
      .def(
          "pop",
          [](PeriodList& self, int64_t index) -> Period {
            if (index < 0) {
              index += static_cast<int64_t>(self.size());
            }
            if (index < 0 || static_cast<size_t>(index) >= self.size()) {
              throw nb::index_error();
            }
            auto it = static_cast<const pyinterp::PeriodList&>(self).begin() +
                      static_cast<ptrdiff_t>(index);
            Period period{*it, self.resolution()};
            self.erase(it);
            return period;
          },
          "index"_a = -1)
      .def(
          "extend",
          [](PeriodList& self, const PeriodList& periods) {
            for (const auto& period : periods) {
              self.append(Period{period, self.resolution()});
            }
          },
          "periods"_a)
      .def(
          "__iter__",
          [](const PeriodList& v) {
            return nb::make_iterator(nb::type<PeriodList>(), "iterator",
                                     v.begin(), v.end());
          },
          nb::keep_alive<0, 1>())
      .def("__len__", [](const PeriodList& self) { return self.size(); })
      .def("__getitem__",
           [](const PeriodList& self, size_t index) -> Period {
             if (index >= self.size()) {
               throw nb::index_error();
             }
             return self[index];
           })
      .def(
          "__getitem__",
          [](const PeriodList& self, const nb::slice& slice) -> PeriodList* {
            auto [start, stop, step, slicelength] = slice.compute(self.size());
            auto* result = new PeriodList();
            result->reserve(slicelength);
            for (size_t i = 0; i < slicelength; ++i) {
              result->push_back(self[start + i * step]);
            }
            return result;
          },
          "index_slice"_a)
      .def(
          "__setitem__",
          [](PeriodList& self, size_t index, const Period& value) {
            if (index >= self.size()) {
              throw nb::index_error();
            }
            self.setitem(index, value);
          },
          "index"_a, "value"_a)
      .def(
          "__setitem__",
          [](PeriodList& self, const nb::slice& slice,
             const PeriodList& values) {
            auto [start, stop, step, slicelength] = slice.compute(self.size());
            if (slicelength != values.size()) {
              throw nb::index_error(
                  "Slice length and values size do not match");
            }
            for (size_t i = 0; i < slicelength; ++i) {
              self.setitem(start + i * step, values[i]);
            }
          },
          "index_slice"_a, "values"_a)
      .def(
          "__delitem__",
          [](PeriodList& self, size_t index) {
            if (index >= self.size()) {
              throw nb::index_error();
            }
            self.erase(static_cast<const pyinterp::PeriodList&>(self).begin() +
                       static_cast<ptrdiff_t>(index));
          },
          "index"_a)
      .def(
          "__delitem__",
          [](PeriodList& self, const nb::slice& slice) {
            auto [start, stop, step, slicelength] = slice.compute(self.size());
            if (slicelength == 0) {
              return;
            }
            stop = start + (slicelength - 1) * step;
            if (start >= stop) {
              std::swap(start, stop);
              step = -step;
            }
            if (step == 1) {
              self.erase(
                  static_cast<const pyinterp::PeriodList&>(self).begin() +
                      static_cast<ptrdiff_t>(start),
                  static_cast<const pyinterp::PeriodList&>(self).begin() +
                      static_cast<ptrdiff_t>(stop + 1));
            } else {
              for (size_t ix = 0; ix < slicelength; ++ix) {
                self.erase(
                    static_cast<const pyinterp::PeriodList&>(self).begin() +
                    static_cast<ptrdiff_t>(stop));
                stop -= step;
              }
            }
          },
          "index_slice"_a)
      .def_prop_ro(
          "resolution",
          [](const PeriodList& self) { return to_dtype(self.resolution()); },
          "The resolution dtype of the periods in the list.");
}

}  // namespace pyinterp::pybind

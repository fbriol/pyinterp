// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#pragma once

#include <array>
#include <cmath>
#include <cstdint>
#include <format>
#include <regex>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>

#include "pyinterp/eigen.hpp"

namespace pyinterp::dateutils {

/// @brief Epoch year
constexpr int64_t kEpoch = 1970;
/// @brief Number of days in a week
constexpr int64_t kDaysInWeek = 7;
/// @brief Number of hours in a day
constexpr int64_t kHoursInDay = 24;
/// @brief Number of minutes in an hour
constexpr int64_t kMinutesInHour = 60;
/// @brief Number of minutes in a day
constexpr int64_t kMinutesInDay = kMinutesInHour * kHoursInDay;
/// @brief Number of months in a year
constexpr int64_t kMonthsInYear = 12;
/// @brief Number of weeks in a year
constexpr int64_t kWeeksInYear = 52;
/// @brief Number of seconds in a minute
constexpr int64_t kSecondsInMinute = 60;
/// @brief Number of seconds in an hour
constexpr int64_t kSecondsInHour = kSecondsInMinute * kMinutesInHour;
/// @brief Number of seconds in a day
constexpr int64_t kSecondsInDay = kSecondsInHour * kHoursInDay;
/// @brief One second
constexpr int64_t kSecond = 1;
/// @brief One millisecond
constexpr int64_t kMillisecond = 1'000;
/// @brief One microsecond
constexpr int64_t kMicrosecond = 1'000'000;
/// @brief One nanosecond
constexpr int64_t kNanosecond = 1'000'000'000;
/// @brief One picosecond
constexpr int64_t kPicoSecond = 1'000'000'000'000;
/// @brief One femtosecond
constexpr int64_t kFemtoSecond = 1'000'000'000'000'000;
/// @brief One attosecond
constexpr int64_t kAttosecond = 1'000'000'000'000'000'000;
/// @brief ISO week starts on Monday
constexpr int kIsoWeekStartWDay = 1;
/// @brief ISO week first week contains the first Thursday of the year
constexpr int kIsoWeekFirstWDay = 4;
/// @brief Minimum year day value
constexpr int kYDayMinimum = -366;
/// @brief Number of days in each month
constexpr std::array<int, 13> kDaysInMonth{-1, 31, 28, 31, 30, 31, 30,
                                           31, 31, 30, 31, 30, 31};

/// @brief Handles numpy encoded dates.
class DType {
 public:
  /// @brief Date type
  enum class DateType : uint8_t {
    kDatetime64,
    kTimedelta64,
  };

  /// @brief Clock resolution
  enum class Resolution : uint8_t {
    kYear = 0,
    kMonth = 1,
    kWeek = 2,
    kDay = 3,
    kHour = 4,
    kMinute = 5,
    kSecond = 6,
    kMillisecond = 7,
    kMicrosecond = 8,
    kNanosecond = 9,
    kPicosecond = 10,
    kFemtosecond = 11,
    kAttosecond = 12,
  };

  /// @brief Constructor from numpy dtype string
  /// @param[in] dtype Numpy date type string (e.g., "datetime64[ms]")
  explicit DType(std::string_view dtype) {
    std::smatch match;
    std::string dtype_str(dtype);  // regex needs std::string

    if (!std::regex_search(dtype_str, match, pattern_)) [[unlikely]] {
      throw std::invalid_argument(
          std::format("unknown numpy date type: {}", dtype));
    }

    datetype_ = match[1] == "datetime64" ? DateType::kDatetime64
                                         : DateType::kTimedelta64;
    resolution_ = parse_unit(match[2].str());
  }

  /// @brief Build a DType from a date type and a resolution
  /// @param[in] datetype Date type
  /// @param[in] resolution Clock resolution
  constexpr DType(const DateType datetype, const Resolution resolution) noexcept
      : datetype_{datetype}, resolution_{resolution} {}

  /// @brief Get the clock resolution name
  ///
  /// @return Clock resolution name
  [[nodiscard]] constexpr auto clock_name() const noexcept -> std::string_view {
    switch (resolution_) {
      case Resolution::kYear:
        return "year";
      case Resolution::kMonth:
        return "month";
      case Resolution::kWeek:
        return "week";
      case Resolution::kDay:
        return "day";
      case Resolution::kHour:
        return "hour";
      case Resolution::kMinute:
        return "minute";
      case Resolution::kSecond:
        return "second";
      case Resolution::kMillisecond:
        return "millisecond";
      case Resolution::kMicrosecond:
        return "microsecond";
      case Resolution::kNanosecond:
        return "nanosecond";
      case Resolution::kPicosecond:
        return "picosecond";
      case Resolution::kFemtosecond:
        return "femtosecond";
      case Resolution::kAttosecond:
        return "attosecond";
    }
    return "attosecond";  // Default case
  }

  /// @brief Get the clock unit
  [[nodiscard]] constexpr auto unit() const noexcept -> std::string_view {
    switch (resolution_) {
      case Resolution::kYear:
        return "Y";
      case Resolution::kMonth:
        return "M";
      case Resolution::kWeek:
        return "W";
      case Resolution::kDay:
        return "D";
      case Resolution::kHour:
        return "h";
      case Resolution::kMinute:
        return "m";
      case Resolution::kSecond:
        return "s";
      case Resolution::kMillisecond:
        return "ms";
      case Resolution::kMicrosecond:
        return "us";
      case Resolution::kNanosecond:
        return "ns";
      case Resolution::kPicosecond:
        return "ps";
      case Resolution::kFemtosecond:
        return "fs";
      case Resolution::kAttosecond:
        return "as";
    }
    return "as";  // Default case
  }

  /// @brief Get the clock resolution handled by this instance
  [[nodiscard]] constexpr auto resolution() const noexcept -> Resolution {
    return resolution_;
  }

  /// @brief Get the date type handled by this instance
  [[nodiscard]] constexpr auto datetype() const noexcept -> DateType {
    return datetype_;
  }

  /// @brief Get the date type name
  [[nodiscard]] constexpr auto datetype_name() const noexcept
      -> std::string_view {
    return datetype_ == DateType::kDatetime64 ? "datetime64" : "timedelta64";
  }

  /// @brief Return the string representation of the numpy data type
  [[nodiscard]] explicit operator std::string() const {
    return std::format("{}[{}]", datetype_name(), unit());
  }

  /// @brief Get the order of magnitude of the resolution between seconds and
  /// the clock resolution
  [[nodiscard]] constexpr auto order_of_magnitude() const -> int64_t {
    switch (resolution_) {
      case Resolution::kSecond:
        return kSecond;
      case Resolution::kMillisecond:
        return kMillisecond;
      case Resolution::kMicrosecond:
        return kMicrosecond;
      case Resolution::kNanosecond:
        return kNanosecond;
      case Resolution::kPicosecond:
        return kPicoSecond;
      case Resolution::kFemtosecond:
        return kFemtoSecond;
      case Resolution::kAttosecond:
        return kAttosecond;
      default:
        throw std::invalid_argument("The date resolution must be >= second");
    }
  }

  /// @brief Compare two instances
  [[nodiscard]] constexpr auto operator<=>(const DType& other) const noexcept =
      default;

  /// @brief Transform this instance into a new instance handling a timedelta64
  /// type with the same resolution
  [[nodiscard]] constexpr auto as_timedelta64() const noexcept -> DType {
    return {DateType::kTimedelta64, resolution_};
  }

 private:
  /// @brief Regex pattern to parse numpy date type strings
  static inline const std::regex pattern_{
      R"((datetime64|timedelta64)\[(Y|M|W|D|h|m|s|ms|us|ns|ps|fs|as)\])"};
  /// @brief Date type
  DateType datetype_;
  /// @brief Clock resolution
  Resolution resolution_;

  /// @brief Parse the resolution unit from a string
  /// @param[in] unit Unit string
  /// @return Corresponding Resolution value
  [[nodiscard]] static auto parse_unit(std::string_view unit) -> Resolution {
    static const std::unordered_map<std::string_view, Resolution> unit_map{
        {"Y", Resolution::kYear},         {"M", Resolution::kMonth},
        {"W", Resolution::kWeek},         {"D", Resolution::kDay},
        {"h", Resolution::kHour},         {"m", Resolution::kMinute},
        {"s", Resolution::kSecond},       {"ms", Resolution::kMillisecond},
        {"us", Resolution::kMicrosecond}, {"ns", Resolution::kNanosecond},
        {"ps", Resolution::kPicosecond},  {"fs", Resolution::kFemtosecond},
        {"as", Resolution::kAttosecond},
    };

    if (auto it = unit_map.find(unit); it != unit_map.end()) [[likely]] {
      return it->second;
    }
    throw std::invalid_argument(std::format("invalid date unit: {}", unit));
  }
};

/// @brief Handle a date encoded in a 64-bit integer for a given clock
/// resolution (Clock resolution must be in range Second to Attosecond)
class FractionalSeconds {
 public:
  /// @brief Constructor from DType
  /// @param[in] dtype Numpy data type
  explicit constexpr FractionalSeconds(const DType& dtype)
      : order_of_magnitude_{dtype.order_of_magnitude()} {}

  /// @brief Constructor from numpy data type string
  /// @param[in] dtype Numpy data type string
  explicit FractionalSeconds(std::string_view dtype)
      : order_of_magnitude_{DType(dtype).order_of_magnitude()} {}

  /// @brief Get the number of days, seconds and fractional part elapsed since
  /// 1970
  /// @param[in] datetime64 Date encoded in a 64-bit integer
  /// @return A tuple (days, seconds, fractional)
  [[nodiscard]] constexpr auto days_since_epoch(const int64_t datetime64)
      const noexcept -> std::tuple<int64_t, int64_t, int64_t> {
    auto [seconds, fractional] = epoch(datetime64);
    auto days = seconds / kSecondsInDay;
    if (seconds % kSecondsInDay < 0) {
      --days;
    }
    return {days, seconds, fractional};
  }

  /// @brief Get the maximum number of digits for the fractional part
  [[nodiscard]] constexpr auto ndigits() const noexcept -> int {
    return static_cast<int>(
        std::log10(static_cast<double>(order_of_magnitude_)));
  }

  /// @brief Cast a fractional part to a different scale
  [[nodiscard]] constexpr auto cast(const int64_t frac,
                                    const int64_t scale) const noexcept
      -> int64_t {
    return order_of_magnitude_ <= scale ? (scale / order_of_magnitude_) * frac
                                        : frac / (order_of_magnitude_ / scale);
  }

  /// @brief Get the order of magnitude of the resolution
  [[nodiscard]] constexpr auto order_of_magnitude() const noexcept -> int64_t {
    return order_of_magnitude_;
  }

  /// @brief Get the numpy resolution
  [[nodiscard]] constexpr auto resolution() const noexcept
      -> DType::Resolution {
    switch (order_of_magnitude_) {
      case kAttosecond:
        return DType::Resolution::kAttosecond;
      case kFemtoSecond:
        return DType::Resolution::kFemtosecond;
      case kPicoSecond:
        return DType::Resolution::kPicosecond;
      case kNanosecond:
        return DType::Resolution::kNanosecond;
      case kMicrosecond:
        return DType::Resolution::kMicrosecond;
      case kMillisecond:
        return DType::Resolution::kMillisecond;
      default:
        return DType::Resolution::kSecond;
    }
  }

 private:
  /// @brief Order of magnitude of the resolution
  int64_t order_of_magnitude_;

  /// @brief Get the number of seconds elapsed since 1970
  /// @param[in] datetime64 Date encoded in a 64-bit integer
  /// @return Number of seconds elapsed since 1970
  [[nodiscard]] constexpr auto seconds(const int64_t datetime64) const noexcept
      -> int64_t {
    return datetime64 / order_of_magnitude_;
  }

  /// @brief Get the fractional part of the date
  /// @param[in] datetime64 Date encoded in a 64-bit integer
  /// @return Fractional part of the date
  [[nodiscard]] constexpr auto fractional(
      const int64_t datetime64) const noexcept -> int64_t {
    return datetime64 % order_of_magnitude_;
  }

  /// @brief Get the number of seconds elapsed since 1970 and the fractional
  /// part
  /// @param[in] datetime64 Date encoded in a 64-bit integer
  /// @return A tuple (seconds, fractional)
  [[nodiscard]] constexpr auto epoch(const int64_t datetime64) const noexcept
      -> std::tuple<int64_t, int64_t> {
    auto sec = seconds(datetime64);
    auto frac = fractional(datetime64);
    if (frac < 0) {
      frac += order_of_magnitude_;
      sec -= 1;
    }
    return {sec, frac};
  }
};

/// @brief Represents a year, month, day in a calendar.
struct Date {
  int year;
  unsigned month;
  unsigned day;
};

/// @brief Represents a local time in a day, independent of any particular day.
struct Time {
  unsigned hour;
  unsigned minute;
  unsigned second;
};

/// @brief Represents an ISO calendar date specified by year, week, and day of
/// week.
struct ISOCalendar {
  int year;
  unsigned week;
  unsigned weekday;
};

/// @brief Get the number of hours, minutes and seconds elapsed in the day
/// @param[in] seconds Number of seconds since midnight
/// @return Time structure
[[nodiscard]] constexpr auto time_from_seconds(int64_t seconds) noexcept
    -> Time {
  auto seconds_in_day = seconds % kSecondsInDay;
  if (seconds_in_day < 0) {
    seconds_in_day += kSecondsInDay;
  }
  const auto seconds_in_hour = seconds_in_day % kSecondsInHour;

  return {.hour = static_cast<unsigned>(seconds_in_day / kSecondsInHour),
          .minute = static_cast<unsigned>(seconds_in_hour / kSecondsInMinute),
          .second = static_cast<unsigned>(seconds_in_hour % kSecondsInMinute)};
}

/// @brief Get the date from the number of years since 1970-01-01
/// @param[in] years Number of years since 1970-01-01
/// @return Date structure
[[nodiscard]] constexpr auto date_from_years(const int64_t years) noexcept
    -> Date {
  return {.year = static_cast<int>(kEpoch + years), .month = 1, .day = 1};
}

/// @brief Get the date from the number of months since 1970-01-01
/// @param[in] months Number of months since 1970-01-01
/// @return Date structure
[[nodiscard]] constexpr auto date_from_months(int64_t months) noexcept -> Date {
  auto year = months / kMonthsInYear;
  auto month = months % kMonthsInYear;
  if (month != 0 && months < 0) {
    --year;
    month += kMonthsInYear;
  }
  return {.year = static_cast<int>(kEpoch + year),
          .month = static_cast<unsigned>(month + 1),
          .day = 1};
}

/// @brief Gets year, month, day in civil calendar (Rata Die algorithm)
/// @param[in] days Number of days since 1970-01-01
/// @return Date structure
[[nodiscard]] constexpr auto date_from_days(int64_t days) noexcept -> Date {
  days += 719468LL;
  // era: 400 year period
  const auto era =
      static_cast<int>((days >= 0 ? days : days - 146096LL) / 146097LL);
  // day of era [0, 146096]
  const auto doe = static_cast<unsigned>(days - era * 146097LL);
  // year of era [0, 399]
  const auto yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  // day of year [0, 365]
  const auto doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  // month number (March=0, ..., February=11)
  const auto mon = (5 * doy + 2) / 153;
  const auto day = doy - (153 * mon + 2) / 5 + 1;
  const auto month = mon < 10 ? mon + 3 : mon - 9;

  return {.year = (static_cast<int>(yoe) + era * 400) +
                  static_cast<int>(month <= 2),
          .month = month,
          .day = day};
}

/// @brief Calculate the number of days from the epoch (1970-01-01) for a given
/// date.
/// @note This is the inverse of date_from_days.
/// @param[in] date The date structure
/// @return Number of days since 1970-01-01
[[nodiscard]] constexpr auto days_from_date(const Date& date) noexcept
    -> int64_t {
  auto y = date.year;
  auto m = date.month;
  auto d = date.day;

  // Adjust for the algorithm (March is 0, ..., February is 11 of prev year)
  if (m <= 2) {
    y -= 1;
    m += 12;
  }

  const auto era = (y >= 0 ? y : y - 399) / 400;
  const auto yoe = static_cast<unsigned>(y - era * 400);
  const auto doy = (153 * (m - 3) + 2) / 5 + d - 1;
  const auto doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;

  return era * 146097 + static_cast<int64_t>(doe) - 719468;
}

/// @brief Get the date from the number of weeks since 1970-01-01
/// @param[in] weeks Number of weeks since 1970-01-01
/// @return Date structure
[[nodiscard]] constexpr auto date_from_weeks(const int64_t weeks) noexcept
    -> Date {
  return date_from_days(weeks * kDaysInWeek);
}

/// @brief Get the date and time from the number of hours since 1970-01-01
/// @param[in] hours Number of hours since 1970-01-01
/// @return Tuple containing Date and Time structures
[[nodiscard]] constexpr auto datetime_from_hours(int64_t hours) noexcept
    -> std::tuple<Date, Time> {
  auto days = hours / kHoursInDay;
  auto hour = hours % kHoursInDay;
  if (hour != 0 && hours < 0) {
    --days;
    hour += kHoursInDay;
  }
  return {date_from_days(days),
          {.hour = static_cast<unsigned>(hour), .minute = 0, .second = 0}};
}

/// @brief Get the date and time from the number of minutes since 1970-01-01
/// @param[in] minutes Number of minutes since 1970-01-01
/// @return Tuple containing Date and Time structures
[[nodiscard]] constexpr auto datetime_from_minutes(int64_t minutes) noexcept
    -> std::tuple<Date, Time> {
  auto days = minutes / kMinutesInDay;
  auto minute = minutes % kMinutesInDay;
  if (minute != 0 && minutes < 0) {
    --days;
    minute += kMinutesInDay;
  }
  return {date_from_days(days), time_from_seconds(minute * kSecondsInMinute)};
}

/// @brief Convert a datetime64 to an ISO 8601 string
/// @param[in] value The datetime64 value
/// @param[in] dtype The data type describing the resolution of the datetime64
/// @return ISO 8601 formatted string
[[nodiscard]] inline auto datetime64_to_string(const int64_t value,
                                               const DType& dtype)
    -> std::string {
  Date date{};
  Time time{};

  switch (dtype.resolution()) {
    case DType::Resolution::kYear: {
      date = date_from_years(value);
      return std::format("{}", date.year);
    }
    case DType::Resolution::kMonth: {
      date = date_from_months(value);
      return std::format("{}-{:02}", date.year, date.month);
    }
    case DType::Resolution::kWeek:
    case DType::Resolution::kDay: {
      date = dtype.resolution() == DType::Resolution::kWeek
                 ? date_from_weeks(value)
                 : date_from_days(value);
      return std::format("{}-{:02}-{:02}", date.year, date.month, date.day);
    }
    case DType::Resolution::kHour: {
      std::tie(date, time) = datetime_from_hours(value);
      return std::format("{}-{:02}-{:02}T{:02}", date.year, date.month,
                         date.day, time.hour);
    }
    case DType::Resolution::kMinute: {
      std::tie(date, time) = datetime_from_minutes(value);
      return std::format("{}-{:02}-{:02}T{:02}:{:02}", date.year, date.month,
                         date.day, time.hour, time.minute);
    }
    default: {
      // Fractional seconds (second through attosecond)
      const auto frac = FractionalSeconds(dtype);
      const auto [days, seconds, fractional] = frac.days_since_epoch(value);

      date = date_from_days(days);
      time = time_from_seconds(seconds);

      if (const int ndigits = frac.ndigits(); ndigits > 0) {
        return std::format("{}-{:02}-{:02}T{:02}:{:02}:{:02}.{:0{}}", date.year,
                           date.month, date.day, time.hour, time.minute,
                           time.second, fractional, ndigits);
      }
      return std::format("{}-{:02}-{:02}T{:02}:{:02}:{:02}", date.year,
                         date.month, date.day, time.hour, time.minute,
                         time.second);
    }
  }
}

/// @brief Convert a timedelta64 to a string representation
/// @param[in] value The timedelta64 value
/// @param[in] dtype The data type describing the resolution of the timedelta64
/// @return String representation of the timedelta64 value
[[nodiscard]] inline auto timedelta64_to_string(const int64_t value,
                                                const DType& dtype)
    -> std::string {
  switch (dtype.resolution()) {
    case DType::Resolution::kYear: {
      return std::format("{} years", value);
    }
    case DType::Resolution::kMonth: {
      return std::format("{} months", value);
    }
    case DType::Resolution::kWeek: {
      return std::format("{} weeks", value);
    }
    case DType::Resolution::kDay: {
      return std::format("{} days", value);
    }
    case DType::Resolution::kHour: {
      return std::format("{} hours", value);
    }
    case DType::Resolution::kMinute: {
      return std::format("{} minutes", value);
    }
    case DType::Resolution::kSecond: {
      return std::format("{} seconds", value);
    }
    case DType::Resolution::kMillisecond: {
      return std::format("{} milliseconds", value);
    }
    case DType::Resolution::kMicrosecond: {
      return std::format("{} microseconds", value);
    }
    case DType::Resolution::kNanosecond: {
      return std::format("{} nanoseconds", value);
    }
    case DType::Resolution::kPicosecond: {
      return std::format("{} picoseconds", value);
    }
    case DType::Resolution::kFemtosecond: {
      return std::format("{} femtoseconds", value);
    }
    case DType::Resolution::kAttosecond: {
      return std::format("{} attoseconds", value);
    }
    default:
      std::unreachable();
  }
}

/// @brief True if leap year, else false
[[nodiscard]] constexpr auto is_leap_year(const int year) noexcept -> bool {
  return (year & 3) == 0 && ((year % 25) != 0 || (year & 15) == 0);
}

/// @brief Get the number of days since the first January
/// @param[in] date The date structure
/// @return Number of days since the first January
[[nodiscard]] constexpr auto days_since_january(const Date& date) noexcept
    -> unsigned {
  unsigned result = date.day - 1;

  if (date.month > 2) {
    result += is_leap_year(date.year);
  }

  for (unsigned ix = 1; ix < date.month; ++ix) {
    result += kDaysInMonth[ix];
  }

  return result;
}

/// @brief Get the weekday; Sunday is 0 ... Saturday is 6
/// @param[in] days Number of days since epoch
/// @return Weekday as an unsigned integer
[[nodiscard]] constexpr auto weekday(const int64_t days) noexcept -> unsigned {
  return static_cast<unsigned>(days >= -4 ? (days + 4) % 7
                                          : (days + 5) % 7 + 6);
}

/// @brief The number of days from the first day of the first ISO week to the
/// year day
/// @param[in] yday The day of the year
/// @param[in] wday The day of the week
/// @return Number of days from the first day of the first ISO week to the year
/// day
[[nodiscard]] constexpr auto iso_week_days(const int yday,
                                           const int wday) noexcept -> int {
  constexpr int big_enough_multiple_of_7 = (-kYDayMinimum / 7 + 2) * 7;
  return yday -
         (yday - wday + kIsoWeekFirstWDay + big_enough_multiple_of_7) % 7 +
         kIsoWeekFirstWDay - kIsoWeekStartWDay;
}

/// @brief Return the ISO calendar (year, week, weekday)
/// The first ISO week contains the year's first Thursday; Monday is 1 ...
/// Sunday is 7
/// @param[in] days_since_epoch Number of days since epoch
/// @return ISOCalendar structure
[[nodiscard]] constexpr auto isocalendar(
    const int64_t days_since_epoch) noexcept -> ISOCalendar {
  auto date = date_from_days(days_since_epoch);
  const auto yday = days_since_january(date);
  const auto wday = weekday(days_since_epoch);
  auto days = iso_week_days(static_cast<int>(yday), static_cast<int>(wday));

  // This ISO week belongs to the previous year?
  if (days < 0) {
    --date.year;
    days =
        iso_week_days(static_cast<int>(yday) + (365 + is_leap_year(date.year)),
                      static_cast<int>(wday));
  } else {
    const int week_days =
        iso_week_days(static_cast<int>(yday) - (365 + is_leap_year(date.year)),
                      static_cast<int>(wday));
    // This ISO week belongs to the next year?
    if (week_days >= 0) {
      ++date.year;
      days = week_days;
    }
  }

  return {.year = date.year,
          .week = static_cast<unsigned>(days / 7 + 1),
          .weekday = (wday - 1 + 7) % 7 + 1};
}

/// @brief Convert date types in place
/// @param[in, out] coordinates Array of date values to convert
/// @param[in] source Source date type
/// @param[in] target Target date type
inline auto convert(Eigen::Ref<Vector<int64_t>> coordinates,
                    const DType& source, const DType& target) -> void {
  // If resolutions are identical, no conversion is needed.
  if (source.resolution() == target.resolution()) {
    return;
  }

  // Pre-calculate source helpers if source is sub-second resolution
  std::optional<FractionalSeconds> src_frac_handler;
  if (source.resolution() >= DType::Resolution::kSecond) {
    src_frac_handler.emplace(source);
  }

  // Pre-calculate target helpers if target is sub-second resolution
  int64_t tgt_magnitude = 0;
  if (target.resolution() >= DType::Resolution::kSecond) {
    tgt_magnitude = DType(target).order_of_magnitude();
  }

  for (int64_t& value : coordinates.array()) {
    // Step 1: Normalize Source to Intermediate Representation (Days, Seconds,
    // Fractional)
    int64_t days = 0;
    int64_t seconds = 0;
    int64_t fractional = 0;

    switch (source.resolution()) {
      case DType::Resolution::kYear:
        days = days_from_date(date_from_years(value));
        break;
      case DType::Resolution::kMonth:
        days = days_from_date(date_from_months(value));
        break;
      case DType::Resolution::kWeek:
        days = value * kDaysInWeek;
        break;
      case DType::Resolution::kDay:
        days = value;
        break;
      case DType::Resolution::kHour:
        days = value / kHoursInDay;
        seconds = (value % kHoursInDay) * kSecondsInHour;
        // Handle negative hours correctly
        if (seconds < 0) {
          days--;
          seconds += kSecondsInDay;
        }
        break;
      case DType::Resolution::kMinute:
        days = value / kMinutesInDay;
        seconds = (value % kMinutesInDay) * kSecondsInMinute;
        // Handle negative minutes correctly
        if (seconds < 0) {
          days--;
          seconds += kSecondsInDay;
        }
        break;
      default:
        // Handle Seconds through Attoseconds
        if (src_frac_handler) {
          std::tie(days, seconds, fractional) =
              src_frac_handler->days_since_epoch(value);
        }
        break;
    }

    // Step 2: Convert Intermediate Representation to Target
    switch (target.resolution()) {
      case DType::Resolution::kYear: {
        auto date = date_from_days(days);
        value = date.year - kEpoch;
        break;
      }
      case DType::Resolution::kMonth: {
        auto date = date_from_days(days);
        value = (date.year - kEpoch) * kMonthsInYear + (date.month - 1);
        break;
      }
      case DType::Resolution::kWeek:
        value = days / kDaysInWeek;
        break;
      case DType::Resolution::kDay:
        value = days;
        break;
      case DType::Resolution::kHour:
        value = days * kHoursInDay + seconds / kSecondsInHour;
        break;
      case DType::Resolution::kMinute:
        value = days * kMinutesInDay + seconds / kSecondsInMinute;
        break;
      default:
        // Handle Seconds through Attoseconds
        // value = days * 86400 * scale + seconds * scale + cast(frac)

        // We calculate components separately to avoid overflow as much as
        // possible, though strictly large ranges of days * attoseconds will
        // overflow int64. This behavior is consistent with numpy's overflow
        // behavior.

        value =
            (days * kSecondsInDay * tgt_magnitude) + (seconds * tgt_magnitude);

        // Convert the fractional part from source scale to target scale
        if (src_frac_handler) {
          value += src_frac_handler->cast(fractional, tgt_magnitude);
        }
        break;
    }
  }
}

/// @brief Convert a single date value
/// @param[in] value Date value to convert
/// @param[in] source Source date type
/// @param[in] target Target date type
/// @return Converted date value
inline auto convert(const int64_t value, const DType& source,
                    const DType& target) -> int64_t {
  Eigen::Array<int64_t, 1, 1> array{value};
  convert(array, source, target);
  return array(0);
}

}  // namespace pyinterp::dateutils

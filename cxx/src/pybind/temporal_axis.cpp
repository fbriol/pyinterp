#include "pyinterp/pybind/temporal_axis.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>

#include <mutex>
#include <ranges>

#include "pyinterp/math/temporal_axis.hpp"
#include "pyinterp/pybind/ndarray_serialization.hpp"

namespace nb = nanobind;

namespace pyinterp::pybind {

// singleton to hold numpy module reference
struct NumpyContext {
  nb::object module;  /// Reference to numpy module

  // Get the singleton instance
  static auto get() -> const NumpyContext & {
    static NumpyContext ctx;
    static std::once_flag init_flag;

    std::call_once(init_flag,
                   []() { ctx.module = nb::module_::import_("numpy"); });
    return ctx;
  }
};

// Convert and validate the dtype of a numpy datetime64/timedelta64 array
// @param[in] name Variable name
// @param[in] array Numpy array to validate
// @return The datetime64 or timedelta64 dtype of the array
inline auto retrieve_dtype(const std::string &name, const nb::object &array)
    -> dateutils::DType {
  nb::object arr_dtype = array.attr("dtype");
  auto arr_kind = nb::cast<std::string>(nb::str(arr_dtype));
  try {
    return dateutils::DType(arr_kind);
  } catch (const std::invalid_argument &e) {
    throw std::invalid_argument(
        name + " must be a numpy.datetime64 or numpy.timedelta64 array, got " +
        arr_kind);
  }
}

// Convert a numpy datetime64/timedelta64 array to an Eigen vector of
// int64_t values
// @param[in] array Numpy array to convert
// @return Eigen vector of int64_t values
inline auto numpy_to_vector(const nb::object &array) -> Vector<int64_t> {
  auto viewed =
      nb::cast<nb::ndarray<nb::numpy, int64_t, nb::ndim<1>, nb::c_contig>>(
          array.attr("view")("int64"));
  return Eigen::Map<const Vector<int64_t>>(
      viewed.data(), static_cast<int64_t>(viewed.shape(0)));
}

// Convert timedelta64 scalar to int64_t with resolution validation
// @param[in] param_name Name of the parameter (for error messages)
// @param[in] timedelta Numpy timedelta64 scalar
// @param[in] target_dtype Target dtype for conversion
// @return Converted int64_t value or std::nullopt if input is None
inline auto convert_timedelta64(const std::string &param_name,
                                const nb::object &timedelta,
                                const dateutils::DType &target_dtype)
    -> std::optional<int64_t> {
  if (timedelta.is_none()) {
    return std::nullopt;
  }

  // If the object has no dtype attribute, it's not a numpy object
  if (!nb::hasattr(timedelta, "dtype")) {
    throw std::invalid_argument(param_name +
                                " must be a numpy.timedelta64 scalar or None");
  }

  // Retrieve and validate the dtype of the input timedelta64
  auto cxx_dtype = retrieve_dtype(param_name, timedelta);
  if (cxx_dtype.datetype() != dateutils::DType::DateType::kTimedelta64) {
    throw std::invalid_argument(param_name +
                                " must be a numpy.timedelta64 scalar");
  }

  // The input resolution must not be finer than the target resolution
  if (cxx_dtype > target_dtype.as_timedelta64()) {
    throw std::invalid_argument(
        std::format("{} resolution {} is finer than points resolution {}",
                    param_name, static_cast<std::string>(cxx_dtype),
                    static_cast<std::string>(target_dtype.as_timedelta64())));
  }

  // Input resolution is coarser than target resolution: issue a warning
  if (cxx_dtype < target_dtype.as_timedelta64()) {
    auto msg =
        std::format("converting {} from {} to {}", param_name,
                    static_cast<std::string>(cxx_dtype),
                    static_cast<std::string>(target_dtype.as_timedelta64()));
    if (PyErr_WarnEx(PyExc_RuntimeWarning, msg.c_str(), 2) == -1) {
      nb::raise_python_error();
    }
  }

  // Convert scalar to int64 via .view() then extract the value with item()
  auto data = nb::cast<int64_t>(timedelta.attr("view")("int64").attr("item")());
  // Convert to target resolution if needed
  return dateutils::convert(data, cxx_dtype, target_dtype.as_timedelta64());
}

// Return a numpy datetime64/timedelta64 array from an Eigen vector of
// int64_t values
inline auto vector_to_numpy(Vector<int64_t> &&vector, dateutils::DType dtype)
    -> nb::object {
  // Create an int64 ndarray
  auto size = static_cast<size_t>(vector.size());
  auto ptr = std::make_unique<Vector<int64_t>>(std::move(vector));

  auto *data = ptr->data();
  nb::capsule capsule(ptr.get(), [](void *data) noexcept {
    delete static_cast<Vector<int64_t> *>(data);
  });
  ptr.release();

  nb::ndarray<nb::numpy, int64_t, nb::ndim<1>> arr(data, {size}, capsule);

  // Convert to numpy object and view as datetime64/timedelta64
  return arr.cast().attr("view")(std::string(dtype));
}

// Return the numpy dtype of the axis (datetime64 or timedelta64)
inline auto to_dtype(const dateutils::DType &dtype) -> nb::object {
  auto np = NumpyContext::get().module;
  return np.attr("dtype")(std::string(dtype));
}

// Return a datetime64 scalar from int64
inline auto make_datetime64_scalar(int64_t value, dateutils::DType dtype)
    -> nb::object {
  auto np = NumpyContext::get().module;
  auto datetime64_attr = np.attr("datetime64");
  return datetime64_attr(value, dtype.unit().data());
}

// Return a timedelta64 scalar from int64
inline auto make_timedelta64_scalar(int64_t value, dateutils::DType dtype)
    -> nb::object {
  auto np = NumpyContext::get().module;
  auto timedelta64_attr = np.attr("timedelta64");
  return timedelta64_attr(value, dtype.unit().data());
}

// Return the datetime64/timedelta64 scalar from int64
inline auto make_scalar(int64_t value, dateutils::DType dtype) -> nb::object {
  if (dtype.datetype() == dateutils::DType::DateType::kDatetime64) {
    return make_datetime64_scalar(value, dtype);
  }
  return make_timedelta64_scalar(value, dtype);
}

// Convert input temporal array to internal resolution (int64_t)
// @param[in] name Name of the parameter (for error messages)
// @param[in] coordinates Numpy array of datetime64/timedelta64 values
// @param[in] target The target axis dtype for conversion
// @return Eigen vector of int64_t values converted to axis resolution
inline auto cast_to_axis_resolution(const std::string &name,
                                    const nb::object &coordinates,
                                    const dateutils::DType &target)
    -> Vector<int64_t> {
  auto source = retrieve_dtype(name, coordinates);

  if (source.datetype() != target.datetype()) {
    throw std::invalid_argument(
        std::format("{} must be of type {}, got {}", name,
                    static_cast<std::string>(target.datetype_name()),
                    static_cast<std::string>(source.datetype_name())));
  }

  auto result = numpy_to_vector(coordinates);
  // If the source resolution is different from the target resolution, perform
  // conversion
  if (source != target) {
    dateutils::convert(result, source, target);
  }
  return result;
}

TemporalAxis::TemporalAxis(const nb::object &points, const nb::object &epsilon,
                           const nb::object &period)
    : math::TemporalAxis() {
  auto dtype = retrieve_dtype("points", points);
  auto mapped_integer_values = numpy_to_vector(points);
  auto integer_epsilon =
      convert_timedelta64("epsilon", epsilon, dtype).value_or(0);
  auto integer_period = convert_timedelta64("period", period, dtype);
  {
    nb::gil_scoped_release release;
    new (static_cast<math::TemporalAxis *>(this)) math::TemporalAxis(
        dtype, mapped_integer_values, integer_epsilon, integer_period);
  }
}

auto TemporalAxis::dtype() const -> nb::object {
  return to_dtype(math::TemporalAxis::dtype());
}

auto TemporalAxis::front() const -> nb::object {
  return make_scalar(math::TemporalAxis::front(), math::TemporalAxis::dtype());
}

auto TemporalAxis::back() const -> nb::object {
  return make_scalar(math::TemporalAxis::back(), math::TemporalAxis::dtype());
}

auto TemporalAxis::min_value() const -> nb::object {
  return make_scalar(math::TemporalAxis::min_value(),
                     math::TemporalAxis::dtype());
}

auto TemporalAxis::max_value() const -> nb::object {
  return make_scalar(math::TemporalAxis::max_value(),
                     math::TemporalAxis::dtype());
}

auto TemporalAxis::increment() const -> nb::object {
  return make_timedelta64_scalar(math::TemporalAxis::increment(),
                                 math::TemporalAxis::dtype().as_timedelta64());
}

auto TemporalAxis::period() const -> nanobind::object {
  auto period = math::TemporalAxis::period();
  if (period.has_value()) {
    return make_timedelta64_scalar(
        period.value(), math::TemporalAxis::dtype().as_timedelta64());
  }
  return nanobind::none();
}

auto TemporalAxis::coordinate_value(int64_t index) const -> nanobind::object {
  return make_scalar(math::TemporalAxis::coordinate_value(index),
                     math::TemporalAxis::dtype());
}

auto TemporalAxis::coordinate_values(const nanobind::slice &slice) const
    -> nanobind::object {
  auto [start, stop, step, slicelength] = slice.compute(size());

  Vector<int64_t> values(slicelength);
  {
    nb::gil_scoped_release release;

    for (int64_t ix = 0; ix < static_cast<int64_t>(slicelength); ++ix) {
      values[ix] = math::TemporalAxis::coordinate_value(
          static_cast<int64_t>(start + ix * step));
    }
  }
  return vector_to_numpy(std::move(values), math::TemporalAxis::dtype());
}

auto TemporalAxis::find_index(const nanobind::object &coordinates,
                              bool bounded) const -> Vector<int64_t> {
  auto integer_coordinates = cast_to_axis_resolution(
      "coordinates", coordinates, math::TemporalAxis::dtype());
  auto size = integer_coordinates.size();
  auto result = Vector<int64_t>(size);

  {
    nb::gil_scoped_release release;

    for (auto ix : std::views::iota(int64_t{0}, static_cast<int64_t>(size))) {
      result(ix) =
          math::TemporalAxis::find_index(integer_coordinates(ix), bounded);
    }
  }
  return result;
}

auto TemporalAxis::find_indexes(const nanobind::object &coordinates) const
    -> Eigen::Matrix<int64_t, Eigen::Dynamic, 2, Eigen::RowMajor> {
  auto integer_coordinates = cast_to_axis_resolution(
      "coordinates", coordinates, math::TemporalAxis::dtype());
  auto size = integer_coordinates.size();
  auto result =
      Eigen::Matrix<int64_t, Eigen::Dynamic, 2, Eigen::RowMajor>(size, 2);

  {
    nb::gil_scoped_release release;

    for (auto ix : std::views::iota(int64_t{0}, size)) {
      if (auto indexes =
              math::TemporalAxis::find_indexes(integer_coordinates(ix))) {
        const auto [i0, i1] = *indexes;
        result(ix, 0) = i0;
        result(ix, 1) = i1;
      } else {
        result.row(ix).setConstant(-1);
      }
    }
  }
  return result;
}

auto TemporalAxis::cast_to_int64(const nanobind::object &array) const
    -> Vector<int64_t> {
  const auto &dtype = math::TemporalAxis::dtype();
  return cast_to_axis_resolution("array", array, dtype);
}

auto TemporalAxis::cast_to_temporal_axis(const nanobind::object &array) const
    -> nanobind::object {
  return vector_to_numpy(cast_to_int64(array), math::TemporalAxis::dtype());
}

auto TemporalAxis::getstate() const -> nb::tuple {
  serialization::Writer state;
  {
    nanobind::gil_scoped_release release;
    state = math::TemporalAxis::pack();
  }
  return nanobind::make_tuple(writer_to_ndarray(std::move(state)));
}

auto TemporalAxis::setstate(const nb::tuple &state) -> TemporalAxis {
  if (state.size() != 1) {
    throw std::invalid_argument("Invalid state");
  }
  auto array = nanobind::cast<NanobindArray1DUInt8>(state[0]);
  auto reader = reader_from_ndarray(array);
  {
    nanobind::gil_scoped_release release;
    return TemporalAxis(math::TemporalAxis::unpack(reader));
  }
}

}  // namespace pyinterp::pybind

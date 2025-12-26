#pragma once

#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h>

#include <Eigen/Core>
#include <mutex>

#include "pyinterp/dateutils.hpp"
#include "pyinterp/eigen.hpp"

namespace pyinterp::pybind {

/// @brief Singleton context for numpy operations
struct NumpyContext {
  nanobind::object module;  /// Reference to numpy module

  /// @brief Get the singleton instance
  /// @return The singleton instance
  static auto get() -> const NumpyContext & {
    static NumpyContext ctx;
    static std::once_flag init_flag;

    std::call_once(init_flag,
                   []() { ctx.module = nanobind::module_::import_("numpy"); });
    return ctx;
  }
};

/// @brief Convert and validate the dtype of a numpy datetime64/timedelta64
/// array
/// @param[in] name Variable name
/// @param[in] array Numpy array to validate
/// @return The datetime64 or timedelta64 dtype of the array
inline auto retrieve_dtype(const std::string &name,
                           const nanobind::object &array) -> dateutils::DType {
  nanobind::object arr_dtype = array.attr("dtype");
  auto arr_kind = nanobind::cast<std::string>(nanobind::str(arr_dtype));
  try {
    return dateutils::DType(arr_kind);
  } catch (const std::invalid_argument &e) {
    throw std::invalid_argument(
        name + " must be a numpy.datetime64 or numpy.timedelta64 array, got " +
        arr_kind);
  }
}
/// @brief Convert a numpy datetime64/timedelta64 array to an Eigen vector of
/// int64_t values
/// @param[in] array Numpy array to convert
/// @return Eigen vector of int64_t values
inline auto numpy_to_vector(const nanobind::object &array) -> Vector<int64_t> {
  auto viewed =
      nanobind::cast<nanobind::ndarray<nanobind::numpy, int64_t,
                                       nanobind::ndim<1>, nanobind::c_contig>>(
          array.attr("view")("int64"));
  return Eigen::Map<const Vector<int64_t>>(
      viewed.data(), static_cast<int64_t>(viewed.shape(0)));
}

/// @brief Return a datetime64/timedelta64 numpy array from an Eigen vector of
/// int64_t values
/// @param[in] vector Eigen vector of int64_t values
/// @param[in] dtype Target numpy dtype (datetime64 or timedelta64)
/// @return Numpy array of datetime64/timedelta64 values
inline auto vector_to_numpy(Vector<int64_t> &&vector, dateutils::DType dtype)
    -> nanobind::object {
  // Create an int64 ndarray
  auto size = static_cast<size_t>(vector.size());
  auto ptr = std::make_unique<Vector<int64_t>>(std::move(vector));

  auto *data = ptr->data();
  nanobind::capsule capsule(ptr.get(), [](void *data) noexcept {
    delete static_cast<Vector<int64_t> *>(data);
  });
  ptr.release();

  nanobind::ndarray<nanobind::numpy, int64_t, nanobind::ndim<1>> arr(
      data, {size}, capsule);

  // Convert to numpy object and view as datetime64/timedelta64
  return arr.cast().attr("view")(std::string(dtype));
}

}  // namespace pyinterp::pybind

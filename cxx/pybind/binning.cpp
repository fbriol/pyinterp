#include "pyinterp/pybind/binning.hpp"

#include <nanobind/eigen/dense.h>
#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/tuple.h>

#include <optional>
#include <string_view>
#include <tuple>

#include "pyinterp/pybind/axis.hpp"
#include "pyinterp/pybind/dtype_to_str.hpp"

namespace pyinterp::pybind {

constexpr const char *kBinning2DDoc = R"(
Create a 2D binning for grouping values into bins on a grid.

Group a number of more or less continuous values into a smaller number of
"bins" located on a grid.

Args:
    x: Definition of the bin centers for the X axis of the grid.
    y: Definition of the bin centers for the Y axis of the grid.
    spheroid: Spheroid of the coordinate system used to manipulate geographic
        coordinates. If this parameter is not set, the handled coordinates will
        be considered as Cartesian coordinates. Otherwise, ``x`` and ``y`` are
        considered to represents the longitudes and latitudes.
)";

constexpr auto kBinning2DPushDoc = R"(
Push new samples into the defined bins.

Args:
    x: X coordinates of the values to push.
    y: Y coordinates of the values to push.
    z: New samples to push.
    simple: If true, a simple binning 2D is used otherwise a linear binning 2D
        is applied.
)";

constexpr const char *kBinning2DVarianceDoc = R"(
Compute the variance of values for points within each bin.

Args:
    ddof: Delta Degrees of Freedom. The divisor used in calculations is
        N - ``ddof``, where N represents the number of elements. By default
        ``ddof`` is zero.

Returns:
    Variance of values for points within each bin.
)";

constexpr const char *kBinning1DDoc = R"(
Create a 1D binning for grouping values into bins on a vector.

Group a number of more or less continuous values into a smaller number of
"bins" located on a vector.

Args:
    x: Definition of the bin centers for the X Axis.
    range: Range of the binning. If not provided, range is simply
        ``(x.min_value(), x.max_value())``.
)";

constexpr auto kBinning1DPushDoc = R"(
Push new samples into the defined bins.

Args:
    x: X coordinates of the values to push.
    z: New samples to push.
    weights: Weights of the new samples. Defaults to None.
)";

template <typename T>
auto init_binning(nanobind::module_ &m, std::string_view suffix) -> void {
  auto class_name = "Binning2D" + std::string(suffix);

  nanobind::class_<Binning2D<T>>(m, class_name.c_str(), kBinning2DDoc)
      .def(nanobind::init<Axis<double>, Axis<double>,
                          std::optional<geodetic::Spheroid>>(),
           nanobind::arg("x"), nanobind::arg("y"),
           nanobind::arg("spheroid") = std::nullopt)

      .def_prop_ro("x", &Binning2D<T>::x,
                   "Get the bin centers for the X Axis of the grid.")

      .def_prop_ro("y", &Binning2D<T>::y,
                   "Get the bin centers for the Y Axis of the grid.")

      .def_prop_ro(
          "spheroid", [](const Binning2D<T> &self) { return self.spheroid(); },
          "Get the spheroid used for geographic coordinates.")

      .def("clear", &Binning2D<T>::clear,
           "Reset the statistics and clear all bins.",
           nanobind::call_guard<nanobind::gil_scoped_release>())

      .def("count", &Binning2D<T>::count,
           "Compute the count of points within each bin.",
           nanobind::call_guard<nanobind::gil_scoped_release>())

      .def("kurtosis", &Binning2D<T>::kurtosis,
           "Compute the kurtosis of values for points within each bin.",
           nanobind::call_guard<nanobind::gil_scoped_release>())

      .def("max", &Binning2D<T>::max,
           "Compute the maximum of values for points within each bin.",
           nanobind::call_guard<nanobind::gil_scoped_release>())

      .def("mean", &Binning2D<T>::mean,
           "Compute the mean of values for points within each bin.",
           nanobind::call_guard<nanobind::gil_scoped_release>())

      .def("min", &Binning2D<T>::min,
           "Compute the minimum of values for points within each bin.",
           nanobind::call_guard<nanobind::gil_scoped_release>())

      .def("push", &Binning2D<T>::push, nanobind::arg("x"), nanobind::arg("y"),
           nanobind::arg("z"), nanobind::arg("simple") = true,
           kBinning2DPushDoc,
           nanobind::call_guard<nanobind::gil_scoped_release>())

      .def("sum", &Binning2D<T>::sum,
           "Compute the sum of values for points within each bin.",
           nanobind::call_guard<nanobind::gil_scoped_release>())

      .def("sum_of_weights", &Binning2D<T>::sum_of_weights,
           "Compute the sum of weights for points within each bin.",
           nanobind::call_guard<nanobind::gil_scoped_release>())

      .def("skewness", &Binning2D<T>::skewness,
           "Compute the skewness of values for points within each bin.",
           nanobind::call_guard<nanobind::gil_scoped_release>())

      .def("variance", &Binning2D<T>::variance, nanobind::arg("ddof") = 0,
           kBinning2DVarianceDoc,
           nanobind::call_guard<nanobind::gil_scoped_release>())

      .def("__iadd__", &Binning2D<T>::operator+=, nanobind::arg("other"),
           "Override the default behavior of the ``+=`` operator.",
           nanobind::call_guard<nanobind::gil_scoped_release>())

      .def(
          "__copy__",
          [](const Binning2D<T> &self) { return Binning2D<T>(self); },
          "Implement the shallow copy operation.",
          nanobind::call_guard<nanobind::gil_scoped_release>())

      .def(
          "__getstate__",
          [](const Binning2D<T> &self) {
            nanobind::gil_scoped_release release;
            return self.getstate();
          },
          "Get the state of the instance for pickling.")

      .def(
          "__setstate__",
          [](Binning2D<T> &self,
             const std::tuple<Axis<double>, Axis<double>, Vector<int8_t>,
                              std::optional<geodetic::Spheroid>> &state) {
            nanobind::gil_scoped_release release;
            return new (&self) Binning2D<T>(Binning2D<T>::setstate(state));
          },
          nanobind::arg("state"),
          "Set the state of the instance from pickling.");

  class_name = "Binning1D" + std::string(suffix);
  nanobind::class_<Binning1D<T>, Binning2D<T>>(m, class_name.c_str(),
                                               kBinning1DDoc)

      .def(nanobind::init<Axis<double>,
                          const std::optional<std::tuple<double, double>> &>(),
           nanobind::arg("x"), nanobind::arg("range") = std::nullopt)

      .def("push", &Binning1D<T>::push, nanobind::arg("x"), nanobind::arg("z"),
           nanobind::arg("weights") = std::nullopt, kBinning1DPushDoc,
           nanobind::call_guard<nanobind::gil_scoped_release>())

      .def("range", &Binning1D<T>::range, "Get the range of the binning.",
           nanobind::call_guard<nanobind::gil_scoped_release>())

      .def(
          "__getstate__",
          [](const Binning1D<T> &self) {
            nanobind::gil_scoped_release release;
            return self.getstate();
          },
          "Get the state of the instance for pickling.")

      .def(
          "__setstate__",
          [](Binning1D<T> &self,
             const std::tuple<Axis<double>, Axis<double>, Vector<int8_t>,
                              std::optional<geodetic::Spheroid>, double, double>
                 &state) {
            nanobind::gil_scoped_release release;
            return new (&self) Binning1D<T>(Binning1D<T>::setstate(state));
          },
          nanobind::arg("state"),
          "Set the state of the instance from pickling.");
}

/// @brief Binning2D factory function that accepts dtype parameter
/// @param[in] x Definition of the bin centers for the X axis of the grid.
/// @param[in] y Definition of the bin centers for the Y axis of the grid.
/// @param[in] spheroid WGS of the coordinate system used to manipulate
/// geographic coordinates. If this parameter is not set, the handled
/// coordinates will be considered as Cartesian coordinates. Otherwise, ``x``
/// and ``y`` are considered to represents the longitudes and latitudes.
/// @param[in] dtype Data type for internal storage, either 'float32' or
/// 'float64'. Determines precision and memory usage. Defaults to 'float64'.
/// @return nanobind::object containing the Binning2D instance.
auto binning_2d_factory(const Axis<double> &x, const Axis<double> &y,
                        const std::optional<geodetic::Spheroid> &spheroid,
                        const nanobind::object &dtype) -> nanobind::object {
  auto dtype_str = dtype_to_str(dtype).value_or("float64");

  // Create appropriate Binning2D based on dtype string
  if (dtype_str == "float32") {
    return nanobind::cast(Binning2D<float>(x, y, spheroid),
                          nanobind::rv_policy::move);
  }
  if (dtype_str == "float64") {
    return nanobind::cast(Binning2D<double>(x, y, spheroid),
                          nanobind::rv_policy::move);
  }
  throw std::invalid_argument("dtype must be 'float32' or 'float64', got: " +
                              dtype_str);
}

/// @brief Binning1D factory function that accepts dtype parameter
/// @param[in] x Definition of the bin centers for the X Axis.
/// @param[in] range Range of the binning. If not provided, range is simply
/// ``(x.min_value(), x.max_value())``.
/// @param[in] dtype Data type for internal storage, either 'float32' or
/// 'float64'. Determines precision and memory usage. Defaults to 'float64'.
/// @return nanobind::object containing the Binning1D instance.
auto binning_1d_factory(const Axis<double> &x,
                        const std::optional<std::tuple<double, double>> &range,
                        const nanobind::object &dtype) -> nanobind::object {
  auto dtype_str = dtype_to_str(dtype).value_or("float64");

  // Create appropriate Binning1D based on dtype string
  if (dtype_str == "float32") {
    return nanobind::cast(Binning1D<float>(x, range),
                          nanobind::rv_policy::move);
  }
  if (dtype_str == "float64") {
    return nanobind::cast(Binning1D<double>(x, range),
                          nanobind::rv_policy::move);
  }
  throw std::invalid_argument("dtype must be 'float32' or 'float64', got: " +
                              dtype_str);
}

constexpr const char *kBinning2DFactoryDoc = R"(
Create a 2D binning for grouping values into bins on a grid.

Group a number of more or less continuous values into a smaller number of
"bins" located on a grid.

Args:
    x: Definition of the bin centers for the X axis of the grid.
    y: Definition of the bin centers for the Y axis of the grid.
    spheroid: Spheroid of the coordinate system used to manipulate geographic
        coordinates. If this parameter is not set, the handled coordinates will
        be considered as Cartesian coordinates. Otherwise, ``x`` and ``y`` are
        considered to represents the longitudes and latitudes.
    dtype: Data type for internal storage, either 'float32' or 'float64'.
        Determines precision and memory usage. Defaults to 'float64'.

Examples:
    >>> import pyinterp
    >>> import numpy as np
    >>> x = pyinterp.Axis(np.arange(0, 360, 1), is_circle=True)
    >>> y = pyinterp.Axis(np.arange(-90, 90, 1))
    >>> binning = pyinterp.Binning2D(x, y)

    >>> # Create with float32 for reduced memory usage
    >>> binning = pyinterp.Binning2D(x, y, dtype='float32')
)";

constexpr const char *kBinning1DFactoryDoc = R"(
Create a 1D binning for grouping values into bins on a vector.

Group a number of more or less continuous values into a smaller number of
"bins" located on a vector.

Args:
    x: Definition of the bin centers for the X Axis.
    range: Range of the binning. If not provided, range is simply
        ``(x.min_value(), x.max_value())``.
    dtype: Data type for internal storage, either 'float32' or 'float64'.
        Determines precision and memory usage. Defaults to 'float64'.

Examples:
    >>> import pyinterp
    >>> import numpy as np
    >>> x = pyinterp.Axis(np.arange(0, 10, 0.1))
    >>> binning = pyinterp.Binning1D(x)

    >>> # Create with float32 for reduced memory usage
    >>> binning = pyinterp.Binning1D(x, dtype='float32')
)";

auto init_binning(nanobind::module_ &m) -> void {
  // Register the concrete Binning2D and Binning1D classes
  init_binning<float>(m, "Float32");
  init_binning<double>(m, "Float64");

  // Register the factory functions
  m.def("Binning2D", &binning_2d_factory, kBinning2DFactoryDoc,
        nanobind::arg("x"), nanobind::arg("y"),
        nanobind::arg("spheroid") = std::nullopt,
        nanobind::arg("dtype") = nanobind::none());

  m.def("Binning1D", &binning_1d_factory, kBinning1DFactoryDoc,
        nanobind::arg("x"), nanobind::arg("range") = std::nullopt,
        nanobind::arg("dtype") = nanobind::none());
}

}  // namespace pyinterp::pybind

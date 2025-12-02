#include "pyinterp/pybind/axis.hpp"

#include <nanobind/eigen/dense.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>

#include "nanobind/nanobind.h"

namespace nb = nanobind;

namespace pyinterp::pybind {

constexpr const char *const kAxisDoc =
    R"(Create a coordinate axis for variable values.

A coordinate axis defines the coordinates of a variable's values.
It accepts an array of floating-point values and optional parameters
for comparison tolerance and periodicity.

Parameters:
    values: Axis values as a numpy-compatible array of float64.
    epsilon: Maximum allowed difference between two real numbers to
        consider them equal. Defaults to 1e-6.
    period: Period of the axis for cyclic variables (e.g. 360 for
        degrees). Defaults to ``None``.

Examples:
    >>> import numpy as np
    >>> import pyinterp

    # Create a regular axis from 0 to 10 with step of 1
    >>> values = np.arange(0, 11, dtype='float64')
    >>> axis = pyinterp.Axis(values)

    # Create a regular periodic axis from 0° to 350° with a 10° increment.
    # Since 360 is equivalent to 0, it is intentionally omitted from the array.
    >>> values = np.arange(0, 351, 10, dtype='float64')
    >>> axis_periodic = pyinterp.Axis(
    ...     values,
    ...     epsilon=1e-5,  # Tolerance for floating-point comparisons
    ...     period=360.0   # Period of the axis
    ... )

    # Create an irregular axis with tolerance
    >>> values = np.array([0.0, 1.1, 2.5, 4.0, 7.3], dtype='float64')
    >>> axis_irregular = pyinterp.Axis(
    ...     values,
    ...     epsilon=1e-5  # Tolerance for floating-point comparisons
    ... )
)";

constexpr const char *const kFindIndexDoc =
    R"(Find the index of the axis element that contains a coordinate.

Search the axis for the grid index that contains each provided coordinate.
If a coordinate lies outside the axis range, behavior depends on the
'bounded' argument.

Args:
    coordinates: Positions in this coordinate system.
    bounded: If True, coordinates outside the axis range yield -1.
        If False, the index of the closest axis value is returned.

Returns:
    Index of the grid point containing the coordinate, or -1 for
    out-of-range coordinates when bounded is True.
)";

constexpr const char *const kFindIndexesDoc =
    R"(Find indexes of axis elements surrounding each coordinate.

For each coordinate in the input array, find the indexes ``i0`` and ``i1`` such
that ``self[i0] <= coordinate <= self[i1]``. Coordinates outside the axis range
are set to -1.

Args:
    coordinates: Positions in this coordinate system.

Returns:
    A matrix of shape ``(n, 2)``. The first column contains the indexes ``i0``
    and the second column contains the indexes ``i1`` found.
)";

constexpr const char *const kFlipDoc =
    R"(Reverse the order of elements in the axis.

Args:
    inplace: If True, modify this instance in place. If False, return a
        flipped copy. Defaults to False.

Returns:
    The flipped axis (self when inplace is True, otherwise a copy).
)";

/// Common axis operations for all axis types
template <class Axis>
auto implement_axis(nanobind::class_<Axis> &axis, const std::string &name) {
  axis.def(
          "__repr__",
          [](const Axis &self) -> std::string {
            return static_cast<std::string>(self);
          },
          "Return the string representation of this Axis.",
          nanobind::call_guard<nb::gil_scoped_release>())
      .def(
          "__copy__", [](const Axis &self) { return Axis(self); },
          "Implement the shallow copy operation.",
          nb::call_guard<nb::gil_scoped_release>())

      .def(
          "__getitem__",
          [](const Axis &self, size_t index) {
            return self.coordinate_value(index);
          },
          nb::arg("index"), nanobind::call_guard<nb::gil_scoped_release>())

      .def("__getitem__",
           [](const Axis &self, const nb::slice &slice) {
             return self.coordinate_values(slice);
           })

      .def(
          "__len__", [](const Axis &self) { return self.size(); },
          "Return the length of the axis.",
          nanobind::call_guard<nb::gil_scoped_release>())

      .def(
          "is_regular",
          [](const Axis &self) -> bool { return self.is_regular(); },
          "Check if the axis is regular.",
          nb::call_guard<nb::gil_scoped_release>())

      .def(
          "flip",
          [](Axis &self, const bool inplace) -> Axis {
            if (inplace) {
              self.flip();
              return self;
            }
            auto result = self;
            result.flip();
            return result;
          },
          nb::arg("inplace") = false, kFlipDoc,
          nb::call_guard<nb::gil_scoped_release>())

      .def(
          "is_ascending",
          [](const Axis &self) -> bool { return self.is_ascending(); },
          "Check if the axis values are in ascending order.",
          nb::call_guard<nb::gil_scoped_release>())

      .def(
          "__eq__",
          [](const Axis &self, const Axis &rhs) -> bool {
            return self.operator==(rhs);
          },
          nb::arg("other"),
          "Override the default behavior of the ``==`` operator.",
          nb::call_guard<nb::gil_scoped_release>())

      .def(
          "__ne__",
          [](const Axis &self, const Axis &rhs) -> bool { return self != rhs; },
          nb::arg("other"),
          "Override the default behavior of the ``!=`` operator.",
          nb::call_guard<nb::gil_scoped_release>())

      .def("__getstate__", &Axis::getstate, "Get the state for pickling.")
      .def(
          "__setstate__",
          [](Axis &self, nanobind::tuple &state) {
            return new (&self) Axis(std::move(Axis::setstate(state)));
          },
          nb::arg("state"), "Set the state for unpickling.");
}

template <typename T>
void init_axis(nb::module_ &m) {
  auto axis = nb::class_<Axis<T>>(m, "Axis", kAxisDoc);

  axis.def(nb::init<const Eigen::Ref<const Vector<T>> &, const T,
                    const std::optional<T> &>(),
           nb::arg("values"), nb::arg("epsilon") = 1e-6,
           nb::arg("period") = std::nullopt,
           "Initialize the axis with the given values, tolerance, and period.",
           nb::call_guard<nb::gil_scoped_release>())

      .def_prop_ro(
          "is_periodic",
          [](const Axis<T> &self) -> bool { return self.is_periodic(); },
          "True if this axis represents a periodic variable.",
          nb::call_guard<nb::gil_scoped_release>())

      .def_prop_ro(
          "period",
          [](const Axis<T> &self) -> std::optional<T> { return self.period(); },
          "Period value of this axis.",
          nb::call_guard<nb::gil_scoped_release>())

      .def(
          "increment",
          [](const Axis<T> &self) -> T { return self.increment(); },
          "Get the increment (step) between values in this axis.",
          nb::call_guard<nb::gil_scoped_release>())

      .def(
          "front", [](const Axis<T> &self) -> T { return self.front(); },
          "First value of this axis.", nb::call_guard<nb::gil_scoped_release>())
      .def(
          "back", [](const Axis<T> &self) -> T { return self.back(); },
          "Last value of this axis.", nb::call_guard<nb::gil_scoped_release>())

      .def(
          "min_value", [](const Axis<T> &self) { return self.min_value(); },
          "Minimum value of this axis.",
          nb::call_guard<nb::gil_scoped_release>())
      .def(
          "max_value", [](const Axis<T> &self) { return self.max_value(); },
          "Maximum value of this axis.",
          nb::call_guard<nb::gil_scoped_release>())

      .def(
          "find_index",
          [](const Axis<T> &self,
             const Eigen::Ref<const Vector<T>> &coordinates,
             const bool bounded) -> Vector<int64_t> {
            return self.find_index(coordinates, bounded);
          },
          nb::arg("coordinates"), nb::arg("bounded"), kFindIndexDoc,
          nb::call_guard<nb::gil_scoped_release>())

      .def(
          "find_indexes",
          [](const Axis<T> &self,
             const Eigen::Ref<const Vector<T>> &coordinates)
              -> Eigen::Matrix<int64_t, Eigen::Dynamic, 2, Eigen::RowMajor> {
            return self.find_indexes(coordinates);
          },
          nb::arg("coordinates"), kFindIndexesDoc,
          nb::call_guard<nb::gil_scoped_release>());
  implement_axis<Axis<T>>(axis, "Axis");
}

void init_axis(nanobind::module_ &m) { init_axis<double>(m); }

}  // namespace pyinterp::pybind

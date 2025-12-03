#include "pyinterp/pybind/config.hpp"

#include <nanobind/nanobind.h>

#include "pyinterp/pybind/config/common.hpp"
#include "pyinterp/pybind/config/geometric.hpp"
#include "pyinterp/pybind/config/windowed.hpp"

namespace nb = nanobind;

namespace pyinterp::pybind {
namespace config {

template <typename Class>
auto add_common_attributes(nb::class_<Class>& pyclass) -> nb::class_<Class>& {
  pyclass
      .def("bounds_error", &Class::with_bounds_error, nb::arg("value"),
           "Whether to raise an error when interpolated values are "
           "requested outside the domain defined by the input "
           "data.")
      .def("num_threads", &Class::with_num_threads, nb::arg("value"),
           "Number of threads to use for interpolation. A value of 0 means "
           "that all available cores will be used.");
  return pyclass;
}

namespace geometric {

template <typename Class>
inline auto add_methods(nb::class_<Class>& pyclass) -> nb::class_<Class>& {
  pyclass
      .def_static("bilinear", &Class::bilinear,
                  "Create a configuration for bilinear interpolation.",
                  nb::call_guard<nb::gil_scoped_release>())
      .def_static("idw", &Class::idw, nb::arg("exp") = 2,
                  "Create a configuration for inverse distance weighting "
                  "interpolation.",
                  nb::call_guard<nb::gil_scoped_release>())
      .def_static("nearest", &Class::nearest,
                  "Create a configuration for nearest-neighbor interpolation.",
                  nb::call_guard<nb::gil_scoped_release>());
  return pyclass;
}

inline auto bind(nb::module_& m) -> void {
  add_common_attributes(add_methods(
      nb::class_<Bivariate>(m, "Bivariate",
                            "Parameters controlling bivariate interpolation "
                            "on two-dimensional grids.")
          .def(nb::init<>(), "Default constructor.",
               nb::call_guard<nb::gil_scoped_release>())));

  add_common_attributes(add_methods(
      nb::class_<Trivariate>(m, "Trivariate",
                             "Parameters controlling trivariate interpolation "
                             "on three-dimensional grids.")
          .def(nb::init<>(), "Default constructor.",
               nb::call_guard<nb::gil_scoped_release>())));

  add_common_attributes(
      add_methods(nb::class_<Quadrivariate>(
                      m, "Quadrivariate",
                      "Parameters controlling quadrivariate interpolation "
                      "on four-dimensional grids.")
                      .def(nb::init<>(), "Default constructor.",
                           nb::call_guard<nb::gil_scoped_release>())));
}

}  // namespace geometric

namespace windowed {

template <typename Class>
auto add_methods(nb::class_<Class>& pyclass) -> nb::class_<Class>& {
  pyclass
      .def_static("bicubic", &Class::bicubic,
                  "Create a configuration for bicubic interpolation.",
                  nb::call_guard<nb::gil_scoped_release>())
      .def_static("bilinear", &Class::bilinear,
                  "Create a configuration for bilinear interpolation.",
                  nb::call_guard<nb::gil_scoped_release>())
      .def_static("akima", &Class::akima,
                  "Create a configuration for Akima spline interpolation.",
                  nb::call_guard<nb::gil_scoped_release>())
      .def_static("akima_periodic", &Class::akima_periodic,
                  "Create a configuration for Akima periodic spline "
                  "interpolation.",
                  nb::call_guard<nb::gil_scoped_release>())
      .def_static("c_spline", &Class::c_spline,
                  "Create a configuration for C spline interpolation.",
                  nb::call_guard<nb::gil_scoped_release>())
      .def_static("c_spline_not_a_knot", &Class::c_spline_not_a_knot,
                  "Create a configuration for C spline not-a-knot "
                  "interpolation.",
                  nb::call_guard<nb::gil_scoped_release>())
      .def_static("c_spline_periodic", &Class::c_spline_periodic,
                  "Create a configuration for C spline periodic "
                  "interpolation.",
                  nb::call_guard<nb::gil_scoped_release>())
      .def_static("steffen", &Class::steffen,
                  "Create a configuration for Steffen spline interpolation.",
                  nb::call_guard<nb::gil_scoped_release>())
      .def_static("linear", &Class::linear,
                  "Create a configuration for linear interpolation.",
                  nb::call_guard<nb::gil_scoped_release>())
      .def_static("polynomial", &Class::polynomial,
                  "Create a configuration for polynomial spline "
                  "interpolation.",
                  nb::call_guard<nb::gil_scoped_release>())
      .def("with_num_threads", &Class::with_num_threads, nb::arg("value"),
           "Number of threads to use for interpolation. A value of 0 means "
           "that all available cores will be used.",
           nb::call_guard<nb::gil_scoped_release>())
      .def("with_bounds_error", &Class::with_bounds_error, nb::arg("value"),
           "Whether to raise an error when interpolated values are "
           "requested outside the domain defined by the input "
           "data.",
           nb::call_guard<nb::gil_scoped_release>())
      .def("with_window_size_x", &Class::with_window_size_x,
           "Update window size in x direction.", nb::arg("size"),
           nb::call_guard<nb::gil_scoped_release>())
      .def("with_window_size_y", &Class::with_window_size_y,
           "Update window size in y direction.", nb::arg("size"),
           nb::call_guard<nb::gil_scoped_release>())
      .def("with_boundary_mode", &Class::with_boundary_mode,
           "Update boundary mode.", nb::arg("mode"),
           nb::call_guard<nb::gil_scoped_release>());
  return pyclass;
}

auto bind(nb::module_& m) -> void {
  nb::enum_<math::axis::Boundary>(m, "Boundary",
                                  "Type of boundary handling on an Axis.")
      .value("EXPAND", math::axis::Boundary::kExpand,
             "Expand the boundary as a constant.")
      .value("WRAP", math::axis::Boundary::kWrap,
             "Circular boundary conditions.")
      .value("SYM", math::axis::Boundary::kSym,
             "Symmetrical boundary conditions.")
      .value("UNDEF", math::axis::Boundary::kUndef,
             "Boundary violation is not defined.");

  nb::class_<AxisConfig>(m, "AxisConfig",
                         "Configuration for a single-axis interpolation.")
      .def(nb::init<>(), "Default constructor.",
           nb::call_guard<nb::gil_scoped_release>())
      .def_static("linear", &AxisConfig::linear,
                  "Create a configuration for linear interpolation.",
                  nb::call_guard<nb::gil_scoped_release>())
      .def_static("nearest", &AxisConfig::nearest,
                  "Create a configuration for nearest-neighbor interpolation.",
                  nb::call_guard<nb::gil_scoped_release>());

  add_common_attributes(add_methods(
      nb::class_<Bivariate>(m, "Bivariate",
                            "Parameters controlling the windowing "
                            "interpolation on two-dimensional grids.")
          .def(nb::init<>(), "Default constructor.",
               nb::call_guard<nb::gil_scoped_release>())));

  auto trivariate = add_common_attributes(add_methods(
      nb::class_<Trivariate>(m, "Trivariate",
                             "Parameters controlling the windowing "
                             "interpolation on three-dimensional grids.")
          .def(nb::init<>(), "Default constructor.",
               nb::call_guard<nb::gil_scoped_release>())));
  trivariate.def("with_third_axis", &Trivariate::with_third_axis,
                 nb::arg("config"), "Update third axis configuration.",
                 nb::call_guard<nb::gil_scoped_release>());

  auto quadrivariate = add_common_attributes(add_methods(
      nb::class_<Quadrivariate>(m, "Quadrivariate",
                                "Parameters controlling the windowing "
                                "interpolation on four-dimensional grids.")
          .def(nb::init<>(), "Default constructor.",
               nb::call_guard<nb::gil_scoped_release>())));
  quadrivariate
      .def("with_third_axis", &Quadrivariate::with_third_axis,
           nb::arg("config"), "Update third axis configuration.",
           nb::call_guard<nb::gil_scoped_release>())
      .def("with_fourth_axis", &Quadrivariate::with_fourth_axis,
           nb::arg("config"), "Update fourth axis configuration.",
           nb::call_guard<nb::gil_scoped_release>());
}

}  // namespace windowed
}  // namespace config

auto init_config(nb::module_& m) -> void {
  auto config = m.def_submodule("config", "Interpolation configurations.");
  auto geometric = config.def_submodule(
      "geometric", "Configuration for geometric interpolation.");
  auto windowed = config.def_submodule(
      "windowed", "Configuration for windowed interpolation.");
  config::geometric::bind(geometric);
  config::windowed::bind(windowed);
}

}  // namespace pyinterp::pybind

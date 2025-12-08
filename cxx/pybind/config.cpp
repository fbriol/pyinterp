#include "pyinterp/pybind/config.hpp"

#include <nanobind/nanobind.h>

#include "pyinterp/math/interpolate/rbf.hpp"
#include "pyinterp/pybind/config/common.hpp"
#include "pyinterp/pybind/config/geometric.hpp"
#include "pyinterp/pybind/config/rtree.hpp"
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

namespace rtree {

/// @brief Add common RTree methods (k, radius, num_threads) to a class
/// @tparam Class The configuration class type
/// @param pyclass The nanobind class wrapper
/// @return Reference to the modified class wrapper
template <typename Class>
auto add_rtree_methods(nb::class_<Class>& pyclass) -> nb::class_<Class>& {
  pyclass
      .def("with_k", &Class::with_k, nb::arg("value"),
           "Set the number of neighbors to consider for interpolation.",
           nb::call_guard<nb::gil_scoped_release>())
      .def("with_radius", &Class::with_radius, nb::arg("value"),
           "Set the search radius in meters (None for unlimited).",
           nb::call_guard<nb::gil_scoped_release>())
      .def("with_num_threads", &Class::with_num_threads, nb::arg("value"),
           "Number of threads to use for interpolation. A value of 0 means "
           "that all available cores will be used.",
           nb::call_guard<nb::gil_scoped_release>());
  return pyclass;
}

auto bind(nb::module_& m) -> void {
  // Bind RadialBasisFunction enum
  nb::enum_<math::interpolate::RadialBasisFunction>(
      m, "RadialBasisFunction",
      "Type of radial basis function for RadialBasisFunction "
      "interpolation.")
      .value("CUBIC", math::interpolate::RadialBasisFunction::kCubic,
             "Cubic radial basis function.")
      .value("GAUSSIAN", math::interpolate::RadialBasisFunction::kGaussian,
             "Gaussian radial basis function.")
      .value("INVERSE_MULTIQUADRIC",
             math::interpolate::RadialBasisFunction::kInverseMultiquadric,
             "Inverse multiquadric radial basis function.")
      .value("LINEAR", math::interpolate::RadialBasisFunction::kLinear,
             "Linear radial basis function.")
      .value("MULTIQUADRIC",
             math::interpolate::RadialBasisFunction::kMultiquadric,
             "Multiquadric radial basis function.")
      .value("THIN_PLATE", math::interpolate::RadialBasisFunction::kThinPlate,
             "Thin plate radial basis function.");

  // Bind CovarianceFunction enum
  nb::enum_<math::interpolate::CovarianceFunction>(
      m, "CovarianceFunction",
      "Type of covariance function for Kriging interpolation.")
      .value("MATERN_12", math::interpolate::CovarianceFunction::kMatern_12,
             "Matérn :math:`\\nu = 0.5` (exponential, C⁰).")
      .value("MATERN_32", math::interpolate::CovarianceFunction::kMatern_32,
             "Matérn :math:`\\nu = 1.5` (C¹).")
      .value("MATERN_52", math::interpolate::CovarianceFunction::kMatern_52,
             "Matérn :math:`\\nu = 2.5` (C²).")
      .value("CAUCHY", math::interpolate::CovarianceFunction::kCauchy,
             "Cauchy (heavy-tailed).")
      .value("SPHERICAL", math::interpolate::CovarianceFunction::kSpherical,
             "Spherical (compact support).")
      .value("GAUSSIAN", math::interpolate::CovarianceFunction::kGaussian,
             "Gaussian (:math:`C^\\infty`, can cause numerical issues).")
      .value(
          "WENDLAND", math::interpolate::CovarianceFunction::kWendland,
          "Wendland :math:`\\phi_{3,0}` (compact support, sparse matrices).");

  // Bind window::Function enum
  nb::enum_<math::interpolate::window::Function>(
      m, "WindowFunction",
      "Type of window function for WindowFunction interpolation.")
      .value("BLACKMAN", math::interpolate::window::Function::kBlackman,
             "Blackman window function.")
      .value("BLACKMAN_HARRIS",
             math::interpolate::window::Function::kBlackmanHarris,
             "Blackman-Harris window function.")
      .value("BOXCAR", math::interpolate::window::Function::kBoxcar,
             "Boxcar (rectangular) window function.")
      .value("FLAT_TOP", math::interpolate::window::Function::kFlatTop,
             "Flat top window function (used for accurate amplitude "
             "measurements).")
      .value("GAUSSIAN", math::interpolate::window::Function::kGaussian,
             "Gaussian window function.")
      .value("HAMMING", math::interpolate::window::Function::kHamming,
             "Hamming window function.")
      .value("LANCZOS", math::interpolate::window::Function::kLanczos,
             "Lanczos window function.")
      .value("NUTTALL", math::interpolate::window::Function::kNuttall,
             "Nuttall window function.")
      .value("PARZEN", math::interpolate::window::Function::kParzen,
             "Parzen window function.")
      .value("PARZEN_SWOT", math::interpolate::window::Function::kParzenSWOT,
             "Parzen SWOT window function.");

  // Bind DriftFunction enum
  nb::enum_<math::interpolate::DriftFunction>(
      m, "DriftFunction",
      "Type of drift function for Universal Kriging interpolation.")
      .value("LINEAR", math::interpolate::DriftFunction::kLinear,
             "Constant + linear terms (4 parameters).")
      .value("QUADRATIC", math::interpolate::DriftFunction::kQuadratic,
             "Constant + linear + quadratic terms (10 parameters).");

  // Bind InverseDistanceWeighting configuration
  add_rtree_methods(
      nb::class_<InverseDistanceWeighting>(
          m, "InverseDistanceWeighting",
          "Configuration for inverse distance weighting interpolation.")
          .def(nb::init<>(), "Default constructor.",
               nb::call_guard<nb::gil_scoped_release>())
          .def("with_p", &InverseDistanceWeighting::with_p, nb::arg("value"),
               "Set the power parameter (exponent) for distance weighting.",
               nb::call_guard<nb::gil_scoped_release>()));

  // Bind Kriging configuration
  add_rtree_methods(
      nb::class_<Kriging>(m, "Kriging",
                          "Configuration for kriging interpolation.")
          .def(nb::init<>(), "Default constructor.",
               nb::call_guard<nb::gil_scoped_release>())
          .def("with_sigma", &Kriging::with_sigma, nb::arg("value"),
               "Set the sill parameter (variance at infinity).",
               nb::call_guard<nb::gil_scoped_release>())
          .def("with_lambda", &Kriging::with_lambda, nb::arg("value"),
               "Set the range parameter (distance scale).",
               nb::call_guard<nb::gil_scoped_release>())
          .def("with_nugget", &Kriging::with_nugget, nb::arg("value"),
               "Set the nugget effect parameter (micro-scale variance).",
               nb::call_guard<nb::gil_scoped_release>())
          .def("with_covariance_model", &Kriging::with_covariance_model,
               nb::arg("value"), "Set the covariance function type.",
               nb::call_guard<nb::gil_scoped_release>())
          .def("with_drift_function", &Kriging::with_drift_function,
               nb::arg("value"), "Set the drift function.",
               nb::call_guard<nb::gil_scoped_release>()));

  // Bind RadialBasisFunction configuration
  add_rtree_methods(
      nb::class_<RadialBasisFunction>(
          m, "RadialBasisFunction",
          "Configuration for radial basis function interpolation.")
          .def(nb::init<>(), "Default constructor.",
               nb::call_guard<nb::gil_scoped_release>())
          .def("with_rbf", &RadialBasisFunction::with_rbf, nb::arg("value"),
               "Set the radial basis function type.",
               nb::call_guard<nb::gil_scoped_release>())
          .def("with_epsilon", &RadialBasisFunction::with_epsilon,
               nb::arg("value"),
               "Set the shape parameter epsilon (None for automatic).",
               nb::call_guard<nb::gil_scoped_release>())
          .def("with_smooth", &RadialBasisFunction::with_smooth,
               nb::arg("value"), "Set the smoothing parameter.",
               nb::call_guard<nb::gil_scoped_release>()));

  // Bind InterpolationWindow configuration
  add_rtree_methods(
      nb::class_<InterpolationWindow>(m, "InterpolationWindow",
                                      "Configuration for window function "
                                      "interpolation.")
          .def(nb::init<>(), "Default constructor.",
               nb::call_guard<nb::gil_scoped_release>())
          .def("with_wf", &InterpolationWindow::with_wf, nb::arg("value"),
               "Set the window function type.",
               nb::call_guard<nb::gil_scoped_release>())
          .def("with_arg", &InterpolationWindow::with_arg, nb::arg("value"),
               "Set the window function argument.",
               nb::call_guard<nb::gil_scoped_release>()));
}

}  // namespace rtree
}  // namespace config

auto init_config(nb::module_& m) -> void {
  auto config = m.def_submodule("config", "Interpolation configurations.");
  auto geometric = config.def_submodule(
      "geometric", "Configuration for geometric interpolation.");
  auto windowed = config.def_submodule(
      "windowed", "Configuration for windowed interpolation.");
  auto rtree =
      config.def_submodule("rtree", "Configuration for RTree interpolation.");
  config::geometric::bind(geometric);
  config::windowed::bind(windowed);
  config::rtree::bind(rtree);
}

}  // namespace pyinterp::pybind

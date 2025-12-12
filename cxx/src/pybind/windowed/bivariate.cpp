#include "pyinterp/pybind/windowed/bivariate.hpp"

#include <nanobind/nanobind.h>

#include "pyinterp/pybind/grid.hpp"
#include "pyinterp/pybind/grid_dispatch.hpp"

namespace pyinterp::pybind::windowed {

// Dummy point type for dispatcher (windowed doesn't use Point template)
template <typename T>
struct DummyPoint {};

namespace {

/// @brief Functor for windowed bivariate interpolation dispatch
struct BivariateInterpolator {
  /// @brief Call operator for 2D grids
  template <typename DataType, typename ResultType, typename GridType>
  auto operator()(const GridType& grid,
                  const Eigen::Ref<const Eigen::VectorXd>& x,
                  const Eigen::Ref<const Eigen::VectorXd>& y,
                  const config::windowed::Bivariate& config) const
      -> Vector<ResultType> {
    nanobind::gil_scoped_release release;
    return detail::bivariate<DataType, ResultType>(grid, x, y, config);
  }
};

}  // namespace

auto init_bivariate(nanobind::module_& m) -> void {
  namespace nb = nanobind;

  m.def(
      "bivariate",
      [](const GridHolder& grid, const Eigen::Ref<const Eigen::VectorXd>& x,
         const Eigen::Ref<const Eigen::VectorXd>& y,
         const config::windowed::Bivariate& config) -> nb::object {
        return GridDispatcher<DummyPoint>::dispatch_bivariate(
            grid, x, y, config, BivariateInterpolator{});
      },
      nb::arg("grid"), nb::arg("x"), nb::arg("y"), nb::arg("config"),
      detail::kBivariateDocstring);
}

}  // namespace pyinterp::pybind::windowed

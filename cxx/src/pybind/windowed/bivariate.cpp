#include "pyinterp/pybind/windowed/bivariate.hpp"

#include <nanobind/nanobind.h>

#include "pyinterp/pybind/grid.hpp"
#include "pyinterp/pybind/grid_dispatch.hpp"

namespace pyinterp::windowed::pybind {

// Dummy point type for dispatcher (windowed doesn't use Point template)
template <typename T>
struct DummyPoint {};

// Define GridHolder alias for convenience
using GridHolder = pyinterp::pybind::GridHolder;

// Define GridDispatcher alias for convenience
template <template <class> class PointType>
using GridDispatcher = pyinterp::pybind::GridDispatcher<PointType>;

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

}  // namespace pyinterp::windowed::pybind

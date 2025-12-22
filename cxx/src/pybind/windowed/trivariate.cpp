#include "pyinterp/pybind/windowed/trivariate.hpp"

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

/// @brief Functor for windowed trivariate interpolation dispatch
struct TrivariateInterpolator {
  /// @brief Call operator for 3D grids
  template <typename DataType, typename ResultType, typename ZType,
            typename GridType>
  auto operator()(const GridType& grid,
                  const Eigen::Ref<const Eigen::VectorXd>& x,
                  const Eigen::Ref<const Eigen::VectorXd>& y,
                  const Eigen::Ref<const Vector<ZType>>& z,
                  const config::windowed::Trivariate& config) const
      -> Vector<ResultType> {
    nanobind::gil_scoped_release release;
    return detail::trivariate<GridType, ResultType, ZType>(grid, x, y, z,
                                                           config);
  }
};

}  // namespace

auto init_trivariate(nanobind::module_& m) -> void {
  namespace nb = nanobind;

  m.def(
      "trivariate",
      [](const GridHolder& grid, const Eigen::Ref<const Eigen::VectorXd>& x,
         const Eigen::Ref<const Eigen::VectorXd>& y, const nb::object& z,
         const config::windowed::Trivariate& config) -> nb::object {
        return GridDispatcher<DummyPoint>::dispatch_trivariate(
            grid, x, y, z, config, TrivariateInterpolator{});
      },
      nb::arg("grid"), nb::arg("x"), nb::arg("y"), nb::arg("z"),
      nb::arg("config"), detail::kTrivariateDocstring);
}

}  // namespace pyinterp::windowed::pybind

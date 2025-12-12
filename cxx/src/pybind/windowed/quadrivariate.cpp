#include "pyinterp/pybind/windowed/quadrivariate.hpp"

#include <nanobind/nanobind.h>

#include "pyinterp/pybind/grid.hpp"
#include "pyinterp/pybind/grid_dispatch.hpp"

namespace pyinterp::pybind::windowed {

// Dummy point type for dispatcher (windowed doesn't use Point template)
template <typename T>
struct DummyPoint {};

namespace {

/// @brief Functor for windowed quadrivariate interpolation dispatch
struct QuadrivariateInterpolator {
  /// @brief Call operator for 4D grids
  template <typename DataType, typename ResultType, typename ZType,
            typename GridType>
  auto operator()(const GridType& grid,
                  const Eigen::Ref<const Vector<double>>& x,
                  const Eigen::Ref<const Vector<double>>& y,
                  const Eigen::Ref<const Vector<ZType>>& z,
                  const Eigen::Ref<const Vector<double>>& u,
                  const config::windowed::Quadrivariate& config) const
      -> Vector<ResultType> {
    nanobind::gil_scoped_release release;
    return detail::quadrivariate<GridType, ResultType, ZType>(grid, x, y, z, u,
                                                              config);
  }
};

}  // namespace

auto init_quadrivariate(nanobind::module_& m) -> void {
  namespace nb = nanobind;

  m.def(
      "quadrivariate",
      [](const GridHolder& grid, const Eigen::Ref<const Vector<double>>& x,
         const Eigen::Ref<const Vector<double>>& y, const nb::object& z,
         const Eigen::Ref<const Vector<double>>& u,
         const config::windowed::Quadrivariate& config) -> nb::object {
        return GridDispatcher<DummyPoint>::dispatch_quadrivariate(
            grid, x, y, z, u, config, QuadrivariateInterpolator{});
      },
      nb::arg("grid"), nb::arg("x"), nb::arg("y"), nb::arg("z"), nb::arg("u"),
      nb::arg("config"), detail::kQuadrivariateDocstring);
}

}  // namespace pyinterp::pybind::windowed

#include "pyinterp/pybind/geometric/trivariate.hpp"

#include <nanobind/nanobind.h>

#include "pyinterp/geometry/point.hpp"
#include "pyinterp/pybind/grid.hpp"
#include "pyinterp/pybind/grid_dispatch.hpp"

namespace pyinterp::pybind::geometric {

// Define Point alias for convenience
template <typename T>
using Point = geometry::SphericalPoint<T>;

namespace {

/// @brief Functor for trivariate interpolation dispatch
struct TrivariateInterpolator {
  /// @brief Call operator for spatial grids
  template <typename DataType, typename ResultType, typename ZType,
            typename GridType>
  auto operator()(const GridType& grid,
                  const Eigen::Ref<const Eigen::VectorXd>& x,
                  const Eigen::Ref<const Eigen::VectorXd>& y,
                  const Eigen::Ref<const Vector<ZType>>& z,
                  const config::geometric::Trivariate& config) const
      -> Vector<ResultType> {
    nanobind::gil_scoped_release release;
    return detail::trivariate<Point, GridType, ResultType, ZType>(grid, x, y, z,
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
         const config::geometric::Trivariate& config) -> nb::object {
        return GridDispatcher<Point>::dispatch_trivariate(
            grid, x, y, z, config, TrivariateInterpolator{});
      },
      nb::arg("grid"), nb::arg("x"), nb::arg("y"), nb::arg("z"),
      nb::arg("config"), detail::kTrivariateDocstring);
}

}  // namespace pyinterp::pybind::geometric

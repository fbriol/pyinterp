#include "pyinterp/pybind/geometric/quadrivariate.hpp"

#include <nanobind/nanobind.h>

#include "pyinterp/geometry/point.hpp"
#include "pyinterp/pybind/grid.hpp"
#include "pyinterp/pybind/grid_dispatch.hpp"

namespace pyinterp::pybind::geometric {

// Define Point alias for convenience
template <typename T>
using Point = geometry::SphericalPoint<T>;

namespace {

/// @brief Functor for quadrivariate interpolation dispatch
struct QuadrivariateInterpolator {
  /// @brief Call operator for 4D grids
  template <typename DataType, typename ResultType, typename ZType,
            typename GridType>
  auto operator()(const GridType& grid,
                  const Eigen::Ref<const Vector<double>>& x,
                  const Eigen::Ref<const Vector<double>>& y,
                  const Eigen::Ref<const Vector<ZType>>& z,
                  const Eigen::Ref<const Vector<double>>& u,
                  const config::geometric::Quadrivariate& config) const
      -> Vector<ResultType> {
    nanobind::gil_scoped_release release;
    return detail::quadrivariate<Point, GridType, ResultType, ZType>(
        grid, x, y, z, u, config);
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
         const config::geometric::Quadrivariate& config) -> nb::object {
        return GridDispatcher<Point>::dispatch_quadrivariate(
            grid, x, y, z, u, config, QuadrivariateInterpolator{});
      },
      nb::arg("grid"), nb::arg("x"), nb::arg("y"), nb::arg("z"), nb::arg("u"),
      nb::arg("config"), detail::kQuadrivariateDocstring);
}

}  // namespace pyinterp::pybind::geometric

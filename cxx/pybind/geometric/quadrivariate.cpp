#include "pyinterp/pybind/geometric/quadrivariate.hpp"

#include <nanobind/nanobind.h>

#include <cstdint>

#include "pyinterp/geometry/point.hpp"
#include "pyinterp/pybind/grid.hpp"

namespace pyinterp::pybind::geometric {

// Define Point alias for convenience
template <typename T>
using Point = geometry::SphericalPoint<T>;

auto init_quadrivariate(nanobind::module_& m) -> void {
  // Grid4D
  bind_quadrivariate<Point, Grid4D<double>, double>(m);
  bind_quadrivariate<Point, Grid4D<float>, float>(m);
  bind_quadrivariate<Point, Grid4D<int8_t>, float>(m);

  // TemporalGrid4D
  bind_quadrivariate<Point, TemporalGrid4D<double>, double>(m);
  bind_quadrivariate<Point, TemporalGrid4D<float>, float>(m);
  bind_quadrivariate<Point, TemporalGrid4D<int8_t>, float>(m);
}

}  // namespace pyinterp::pybind::geometric

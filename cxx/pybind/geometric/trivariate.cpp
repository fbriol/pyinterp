#include "pyinterp/pybind/geometric/trivariate.hpp"

#include <nanobind/nanobind.h>

#include <cstdint>

#include "pyinterp/geometry/point.hpp"
#include "pyinterp/pybind/grid.hpp"

namespace pyinterp::pybind::geometric {

// Define Point alias for convenience
template <typename T>
using Point = geometry::SphericalPoint<T>;

auto init_trivariate(nanobind::module_& m) -> void {
  // Grid3D
  bind_trivariate<Point, Grid3D<double>, double>(m);
  bind_trivariate<Point, Grid3D<float>, float>(m);
  bind_trivariate<Point, Grid3D<int8_t>, float>(m);

  // TemporalGrid3D
  bind_trivariate<Point, TemporalGrid3D<double>, double>(m);
  bind_trivariate<Point, TemporalGrid3D<float>, float>(m);
  bind_trivariate<Point, TemporalGrid3D<int8_t>, float>(m);
}

}  // namespace pyinterp::pybind::geometric

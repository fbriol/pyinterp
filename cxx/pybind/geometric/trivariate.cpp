#include "pyinterp/pybind/geometric/trivariate.hpp"

#include <nanobind/nanobind.h>

#include <cstdint>

#include "pyinterp/geometry/point.hpp"
#include "pyinterp/pybind/grid.hpp"

namespace pyinterp::pybind::geometric {

auto init_trivariate(nanobind::module_& m) -> void {
  // // Grid3D
  bind_trivariate<geometry::SphericalPoint, Grid3D<double>, double>(m);
  bind_trivariate<geometry::SphericalPoint, Grid3D<float>, float>(m);
  bind_trivariate<geometry::SphericalPoint, Grid3D<int8_t>, float>(m);

  // TemporalGrid3D
  bind_trivariate<geometry::SphericalPoint, TemporalGrid3D<double>, double>(m);
  bind_trivariate<geometry::SphericalPoint, TemporalGrid3D<float>, float>(m);
  bind_trivariate<geometry::SphericalPoint, TemporalGrid3D<int8_t>, float>(m);
}

}  // namespace pyinterp::pybind::geometric

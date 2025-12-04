#include "pyinterp/pybind/windowed/trivariate.hpp"

#include <nanobind/nanobind.h>

#include <cstdint>

#include "pyinterp/pybind/grid.hpp"

namespace pyinterp::pybind::windowed {

auto init_trivariate(nanobind::module_& m) -> void {
  bind_trivariate<Grid3D<double>, double>(m);
  bind_trivariate<Grid3D<float>, float>(m);
  bind_trivariate<Grid3D<int8_t>, float>(m);

  bind_trivariate<TemporalGrid3D<double>, double>(m);
  bind_trivariate<TemporalGrid3D<float>, float>(m);
  bind_trivariate<TemporalGrid3D<int8_t>, float>(m);
}

}  // namespace pyinterp::pybind::windowed

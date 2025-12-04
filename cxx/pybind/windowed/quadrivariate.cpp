#include "pyinterp/pybind/windowed/quadrivariate.hpp"

#include <nanobind/nanobind.h>

#include <cstdint>

#include "pyinterp/pybind/grid.hpp"

namespace pyinterp::pybind::windowed {

auto init_quadrivariate(nanobind::module_& m) -> void {
  bind_quadrivariate<Grid4D<double>, double>(m);
  bind_quadrivariate<Grid4D<float>, float>(m);
  bind_quadrivariate<Grid4D<int8_t>, float>(m);

  bind_quadrivariate<TemporalGrid4D<double>, double>(m);
  bind_quadrivariate<TemporalGrid4D<float>, float>(m);
  bind_quadrivariate<TemporalGrid4D<int8_t>, float>(m);
}

}  // namespace pyinterp::pybind::windowed

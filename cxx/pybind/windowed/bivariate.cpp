#include "pyinterp/pybind/windowed/bivariate.hpp"

#include <nanobind/nanobind.h>

namespace pyinterp::pybind::windowed {

auto init_bivariate(nanobind::module_& m) -> void {
  bind_bivariate<double, double>(m);
  bind_bivariate<float, float>(m);
  bind_bivariate<int8_t, float>(m);
}

}  // namespace pyinterp::pybind::windowed

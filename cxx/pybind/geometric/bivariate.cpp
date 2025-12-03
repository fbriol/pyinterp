#include "pyinterp/pybind/geometric/bivariate.hpp"

#include <nanobind/nanobind.h>

#include <cstdint>

#include "pyinterp/geometry/point.hpp"

namespace pyinterp::pybind::geometric {

auto init_bivariate(nanobind::module_& m) -> void {
  bind_bivariate<geometry::SphericalPoint, double, double>(m);
  bind_bivariate<geometry::SphericalPoint, float, float>(m);
  bind_bivariate<geometry::SphericalPoint, int8_t, float>(m);
}

}  // namespace pyinterp::pybind::geometric

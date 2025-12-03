#include <nanobind/nanobind.h>

#include "pyinterp/pybind/axis.hpp"
#include "pyinterp/pybind/config.hpp"
#include "pyinterp/pybind/grid.hpp"

NB_MODULE(core, m) {
  pyinterp::pybind::init_config(m);
  pyinterp::pybind::init_axis(m);
  pyinterp::pybind::init_grids(m);
}

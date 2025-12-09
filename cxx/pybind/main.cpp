#include <nanobind/nanobind.h>

#include "pyinterp/pybind/axis.hpp"
#include "pyinterp/pybind/config.hpp"
#include "pyinterp/pybind/geodetic.hpp"
#include "pyinterp/pybind/geometric.hpp"
#include "pyinterp/pybind/grid.hpp"
#include "pyinterp/pybind/rtree.hpp"
#include "pyinterp/pybind/windowed.hpp"

NB_MODULE(core, m) {
  pyinterp::pybind::init_config(m);
  pyinterp::pybind::init_axis(m);
  pyinterp::pybind::init_grids(m);
  pyinterp::pybind::geometric::init_bivariate(m);
  pyinterp::pybind::geometric::init_trivariate(m);
  pyinterp::pybind::geometric::init_quadrivariate(m);
  pyinterp::pybind::windowed::init_bivariate(m);
  pyinterp::pybind::windowed::init_trivariate(m);
  pyinterp::pybind::windowed::init_quadrivariate(m);
  pyinterp::pybind::init_geodetic(m);
  pyinterp::pybind::init_rtree_3d(m);
}

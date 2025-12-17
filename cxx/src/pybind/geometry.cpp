#include "pyinterp/pybind/geometry.hpp"

#include "pyinterp/pybind/geometry/cartesian.hpp"
#include "pyinterp/pybind/geometry/geographic.hpp"

namespace pyinterp::pybind {

void init_geometry(nanobind::module_& m) {
  auto geometry = m.def_submodule("geometry", "Geometry module");
  pyinterp::geometry::pybind::init_geographic(geometry);
  pyinterp::geometry::pybind::init_cartesian(geometry);
}

}  // namespace pyinterp::pybind

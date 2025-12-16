#include "pyinterp/pybind/geometry.hpp"

#include "pyinterp/pybind/geometry/geographic.hpp"

namespace pyinterp::pybind {

void init_geometry(nanobind::module_& m) {
  auto geometry = m.def_submodule("geometry", "Geometry module");
  pyinterp::geometry::pybind::init_geographic(geometry);
}

}  // namespace pyinterp::pybind

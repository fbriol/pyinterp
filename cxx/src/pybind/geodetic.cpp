#include "pyinterp/pybind/geodetic.hpp"

namespace pyinterp::pybind {

void init_geodetic(nanobind::module_& m) {
  auto geodetic_module = m.def_submodule("geodetic", "Geodetic calculations");
  geodetic::pybind::init_point(geodetic_module);
  geodetic::pybind::init_box(geodetic_module);
  geodetic::pybind::init_line_string(geodetic_module);
  geodetic::pybind::init_ring(geodetic_module);
  geodetic::pybind::init_polygon(geodetic_module);
  geodetic::pybind::init_multipolygon(geodetic_module);
  geodetic::pybind::init_spheroid(geodetic_module);
  geodetic::pybind::init_coordinates(geodetic_module);
}

}  // namespace pyinterp::pybind

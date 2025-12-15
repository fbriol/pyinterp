#include "pyinterp/pybind/geodetic.hpp"

namespace pyinterp::pybind {

void init_geodetic(nanobind::module_& m) {
  auto geodetic_module = m.def_submodule("geodetic", "Geodetic calculations");
  auto algorithms_module =
      geodetic_module.def_submodule("algorithms", "Geodetic algorithms");
  geodetic::pybind::init_point(geodetic_module);
  geodetic::pybind::init_box(geodetic_module);
  geodetic::pybind::init_linestring(geodetic_module);
  geodetic::pybind::init_segment(geodetic_module);
  geodetic::pybind::init_multilinestring(geodetic_module);
  geodetic::pybind::init_ring(geodetic_module);
  geodetic::pybind::init_polygon(geodetic_module);
  geodetic::pybind::init_multipoint(geodetic_module);
  geodetic::pybind::init_multipolygon(geodetic_module);
  geodetic::pybind::init_spheroid(geodetic_module);
  geodetic::pybind::init_coordinates(geodetic_module);
  geodetic::pybind::init_algorithms(algorithms_module);
}

}  // namespace pyinterp::pybind

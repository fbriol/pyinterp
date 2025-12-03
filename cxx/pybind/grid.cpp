#include "pyinterp/pybind/grid.hpp"

#include <cstdint>

#include "nanobind/nanobind.h"

namespace pyinterp::pybind {

auto init_grids(nanobind::module_& m) -> void {
  bind_grids<int8_t>(m, "Int8");
  bind_grids<float>(m, "Float32");
  bind_grids<double>(m, "Float64");
}

}  // namespace pyinterp::pybind

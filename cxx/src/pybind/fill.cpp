#include "pyinterp/pybind/fill.hpp"

#include "pyinterp/pybind/fill/loess.hpp"

namespace nb = nanobind;

namespace pyinterp::pybind {

auto init_fill(nb::module_& m) -> void {
  auto fill = m.def_submodule("fill", "Methods to fill missing data.");

  fill::bind_loess(fill);
}

}  // namespace pyinterp::pybind

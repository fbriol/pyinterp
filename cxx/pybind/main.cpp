#include <nanobind/nanobind.h>

#include "pyinterp/pybind/axis.hpp"

NB_MODULE(core, m) { pyinterp::pybind::init_axis(m); }

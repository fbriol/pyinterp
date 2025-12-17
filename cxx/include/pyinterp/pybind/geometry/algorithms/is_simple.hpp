#pragma once
#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

/// @brief Initialize the is_simple algorithm in the given module
/// @tparam NS Namespace of the geometries (cartesian or geographic)
/// @param[in,out] m Nanobind module
/// @param[in] docstring Documentation string for the is_simple function
template <GeometryNamespace NS>
inline auto init_is_simple(nanobind::module_& m, const char* const docstring)
    -> void {
  auto is_simple_impl = [](const auto& g) -> bool {
    nanobind::gil_scoped_release release;
    return boost::geometry::is_simple(g);
  };
  if constexpr (NS == GeometryNamespace::kCartesian) {
    geometry::pybind::define_for_geometries<decltype(is_simple_impl),
                                            GEOMETRY_TYPES(cartesian)>(
        m, "is_simple", docstring, std::move(is_simple_impl));
  } else {
    geometry::pybind::define_for_geometries<decltype(is_simple_impl),
                                            GEOMETRY_TYPES(geographic)>(
        m, "is_simple", docstring, std::move(is_simple_impl));
  }
}

}  // namespace pyinterp::geometry::pybind

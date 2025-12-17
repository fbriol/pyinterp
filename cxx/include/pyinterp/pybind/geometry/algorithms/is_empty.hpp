#pragma once
#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

/// @brief Initialize the is_empty algorithm in the given module
/// @tparam NS Namespace of the geometries (cartesian or geographic)
/// @param[in,out] m Nanobind module
/// @param[in] docstring Documentation string for the is_empty function
template <GeometryNamespace NS>
inline auto init_is_empty(nanobind::module_& m, const char* const docstring)
    -> void {
  auto is_empty_impl = [](const auto& g) -> bool {
    nanobind::gil_scoped_release release;
    return boost::geometry::is_empty(g);
  };
  if constexpr (NS == GeometryNamespace::kCartesian) {
    geometry::pybind::define_for_geometries<decltype(is_empty_impl),
                                            GEOMETRY_TYPES(cartesian)>(
        m, "is_empty", docstring, std::move(is_empty_impl));
  } else {
    geometry::pybind::define_for_geometries<decltype(is_empty_impl),
                                            GEOMETRY_TYPES(geographic)>(
        m, "is_empty", docstring, std::move(is_empty_impl));
  }
}

}  // namespace pyinterp::geometry::pybind

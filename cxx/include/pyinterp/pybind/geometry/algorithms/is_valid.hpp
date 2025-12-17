#pragma once
#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>
#include <string>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

/// @brief Initialize is_valid algorithm bindings for the specified geometry
/// namespace.
/// @tparam NS Geometry namespace (cartesian or geographic).
/// @param[in,out] m Nanobind module.
/// @param[in] docstring Documentation string for the is_valid function.
template <GeometryNamespace NS>
inline auto init_is_valid(nanobind::module_& m, const char* const docstring)
    -> void {
  auto is_valid_impl = [](const auto& geometry,
                          const bool return_reason) -> nb::object {
    bool valid;
    if (return_reason) {
      std::string reason;
      {
        nb::gil_scoped_release release;
        valid = boost::geometry::is_valid(geometry, reason);
      }
      return nb::make_tuple(valid, reason);
    } else {
      {
        nb::gil_scoped_release release;
        valid = boost::geometry::is_valid(geometry);
      }
      return nb::cast(valid);
    }
  };

  if constexpr (NS == GeometryNamespace::kCartesian) {
    ([&]<typename... Geometry>() {
      (...,
       m.def(
           "area",
           [=](const Geometry& g, const bool return_reason) -> nb::object {
             return is_valid_impl(g, return_reason);
           },
           "geometry"_a, nb::kw_only(), "return_reason"_a = false, docstring));
    }).template operator()<GEOMETRY_TYPES(cartesian)>();
  } else {
    ([&]<typename... Geometry>() {
      (...,
       m.def(
           "area",
           [=](const Geometry& g, const bool return_reason) -> nb::object {
             return is_valid_impl(g, return_reason);
           },
           "geometry"_a, nb::kw_only(), "return_reason"_a = false, docstring));
    }).template operator()<GEOMETRY_TYPES(geographic)>();
  }
}

}  // namespace pyinterp::geometry::pybind

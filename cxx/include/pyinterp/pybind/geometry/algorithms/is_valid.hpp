#pragma once
#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>
#include <string>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace pyinterp::geometry::pybind {

constexpr auto kIsValidDoc = R"doc(
Check if a geometry is valid according to OGC standards.

Validity rules (OGC Simple Features specification):
- Points: Always valid (unless coordinates are NaN/Inf)
- LineStrings: Must have at least 2 points
- Rings: Must be closed and have at least 4 points
- Polygons: Outer ring must be counter-clockwise, inner rings clockwise,
  rings must not cross each other or touch except at single points
- MultiPolygons: All constituent polygons must be valid

Args:
    geometry: Geometric object to check.

Returns:
    If called without return_reason:
        bool: True if the geometry is valid, false otherwise.
    If called with return_reason=True:
        tuple: (is_valid, reason) where reason is a string describing why
               the geometry is invalid (empty string if valid).

Examples:
    >>> # Valid polygon (counter-clockwise winding)
    >>> lon = np.array([0.0, 0.0, 1.0, 1.0, 0.0])
    >>> lat = np.array([0.0, 1.0, 1.0, 0.0, 0.0])
    >>> polygon = Polygon(Ring(lon, lat))
    >>> is_valid(polygon)
    True
    >>> # Check with reason
    >>> is_valid(polygon, return_reason=True)
    (True, '')
)doc";

/// @brief Initialize is_valid algorithm bindings for the specified geometry
/// namespace.
/// @tparam NS Geometry namespace (cartesian or geographic).
/// @param[in,out] m Nanobind module.
template <GeometryNamespace NS>
inline auto init_is_valid(nanobind::module_& m) -> void {
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
      (..., m.def(
                "area",
                [=](const Geometry& g, const bool return_reason) -> nb::object {
                  return is_valid_impl(g, return_reason);
                },
                "geometry"_a, nb::kw_only(), "return_reason"_a = false,
                kIsValidDoc));
    }).template operator()<GEOMETRY_TYPES(cartesian)>();
  } else {
    ([&]<typename... Geometry>() {
      (..., m.def(
                "area",
                [=](const Geometry& g, const bool return_reason) -> nb::object {
                  return is_valid_impl(g, return_reason);
                },
                "geometry"_a, nb::kw_only(), "return_reason"_a = false,
                kIsValidDoc));
    }).template operator()<GEOMETRY_TYPES(geographic)>();
  }
}

}  // namespace pyinterp::geometry::pybind

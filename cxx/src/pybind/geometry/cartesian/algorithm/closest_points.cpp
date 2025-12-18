#include <nanobind/nanobind.h>

#include <boost/geometry.hpp>

#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;

namespace pyinterp::geometry::cartesian::pybind {

constexpr auto kClosestPointsDoc = R"doc(
Calculate the closest points between two geometries.

The closest points are the pair of points, one on each geometry, that are
closest to each other.

Args:
    geometry1: First geometry.
    geometry2: Second geometry.

Returns:
    Closest points as a Segment.

Examples:
    >>> g1 = Polygon(...)  # Define first geometry
    >>> g2 = LineString(...)  # Define second geometry
    >>> closest_points(g1, g2)  # Closest points as a Segment
)doc";

auto init_closest_points(nb::module_& m) -> void {
  geometry::pybind::define_binary_predicate(
      m, "closest_points", kClosestPointsDoc,
      [](const auto& geometry1, const auto& geometry2) -> Segment {
        nb::gil_scoped_release release;
        Segment segment;
        boost::geometry::closest_points(geometry1, geometry2, segment);
        return segment;
      });
}

}  // namespace pyinterp::geometry::cartesian::pybind

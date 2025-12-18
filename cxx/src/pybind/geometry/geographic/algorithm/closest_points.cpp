#include "pyinterp/geometry/geographic/algorithms/closest_points.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>

#include <boost/geometry.hpp>
#include <optional>

#include "pyinterp/geometry/geographic/algorithms/strategy.hpp"
#include "pyinterp/geometry/geographic/spheroid.hpp"
#include "pyinterp/pybind/geometry/algorithm_binding_helpers.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;

namespace pyinterp::geometry::geographic::pybind {

constexpr auto kClosestPointsDoc = R"doc(
Calculate the closest points between two geometries.

The closest points are the pair of points, one on each geometry, that are
closest to each other.

Args:
    geometry1: First geometry.
    geometry2: Second geometry.
    spheroid: Optional spheroid for geodetic calculations. If not provided,
        uses WGS84 ellipsoid.
    strategy: Calculation strategy.

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
      [](const auto& geometry1, const auto& geometry2,
         const std::optional<Spheroid>& spheroid,
         const StrategyMethod& strategy) -> Segment {
        nb::gil_scoped_release release;
        return closest_points(geometry1, geometry2, spheroid, strategy);
      });
}

}  // namespace pyinterp::geometry::geographic::pybind

// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/geometry/geographic/crossover.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/optional.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>

#include <format>

#include "pyinterp/geometry/geographic/algorithms/strategy.hpp"
#include "pyinterp/geometry/geographic/linestring.hpp"
#include "pyinterp/geometry/geographic/point.hpp"
#include "pyinterp/geometry/geographic/spheroid.hpp"
#include "pyinterp/pybind/ndarray_serialization.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;

using pyinterp::pybind::NanobindArray1DUInt8;
using pyinterp::pybind::reader_from_ndarray;
using pyinterp::pybind::writer_to_ndarray;

namespace pyinterp::geometry::geographic::pybind {

constexpr auto kCrossoverClassDoc = R"doc(
Detects and retrieves crossover points between two geographic linestrings.

A Crossover identifies intersection points between two linestrings on the
ellipsoid surface, useful for finding where two paths cross each other in
geographic coordinates.
)doc";

constexpr auto kCrossoverInitDoc = R"doc(
Construct a crossover detector from two linestrings.

Args:
    line1: First linestring.
    line2: Second linestring.
)doc";

constexpr auto kFindUniqueDoc = R"doc(
Find a unique crossover point between the two linestrings.

Args:
    point: Reference point (currently not used in implementation).
    spheroid: Optional spheroid for geodetic calculations. If not provided,
        uses WGS84.
    strategy: Calculation strategy (default: Andoyer).

Returns:
    The unique crossover point if found; an undefined point otherwise.

Raises:
    RuntimeError: If multiple distinct crossover points are found.
)doc";

constexpr auto kFindAllDoc = R"doc(
Find all crossover points between the two linestrings.

Args:
    point: Reference point (currently not used in implementation).
    spheroid: Optional spheroid for geodetic calculations. If not provided,
        uses WGS84.
    strategy: Calculation strategy (default: Andoyer).

Returns:
    MultiPoint containing all crossover points found.
)doc";

constexpr auto kNearestDoc = R"doc(
Find the nearest vertices in both linestrings to a given point.

Uses the golden-section search algorithm to efficiently find the nearest
vertices in both linestrings to the specified point.

Args:
    point: The point to which the nearest vertices are sought.
    predicate: Maximum acceptable distance to consider a vertex as nearest.

Returns:
    Optional tuple containing the indices of the nearest vertices in both
    linestrings if they are within the specified predicate distance;
    None otherwise.
)doc";

auto init_crossover(nb::module_& m) -> void {
  nb::class_<Crossover>(m, "Crossover", kCrossoverClassDoc)
      .def(nb::init<LineString, LineString>(), "line1"_a, "line2"_a,
           kCrossoverInitDoc)

      .def_prop_ro("line1", &Crossover::line1, "Get the first linestring.")

      .def_prop_ro("line2", &Crossover::line2, "Get the second linestring.")

      .def(
          "find_unique",
          [](const Crossover& self, const Point& point,
             const std::optional<Spheroid>& spheroid,
             const StrategyMethod strategy) {
            nb::gil_scoped_release release;
            return self.find_unique(point, spheroid, strategy);
          },
          "point"_a, "spheroid"_a = std::nullopt,
          "strategy"_a = StrategyMethod::kAndoyer, kFindUniqueDoc)

      .def(
          "find_all",
          [](const Crossover& self, const Point& point,
             const std::optional<Spheroid>& spheroid,
             const StrategyMethod strategy) {
            nb::gil_scoped_release release;
            return self.find_all(point, spheroid, strategy);
          },
          "point"_a, "spheroid"_a = std::nullopt,
          "strategy"_a = StrategyMethod::kAndoyer, kFindAllDoc)

      .def(
          "nearest",
          [](const Crossover& self, const Point& point, double predicate) {
            nb::gil_scoped_release release;
            return self.nearest(point, predicate);
          },
          "point"_a, "predicate"_a, kNearestDoc)

      .def("__repr__",
           [](const Crossover& self) {
             return std::format("Crossover(line1={} points, line2={} points)",
                                self.line1().size(), self.line2().size());
           })

      // Pickle support
      .def("__getstate__",
           [](const Crossover& self) {
             serialization::Writer state;
             {
               nb::gil_scoped_release release;
               state = self.pack();
             }
             return nb::make_tuple(writer_to_ndarray(std::move(state)));
           })

      .def("__setstate__", [](Crossover* self, const nb::tuple& state) {
        if (state.size() != 1) {
          throw std::invalid_argument("Invalid state");
        }
        auto array = nanobind::cast<NanobindArray1DUInt8>(state[0]);
        auto reader = reader_from_ndarray(array);
        {
          nb::gil_scoped_release release;
          auto base = geometry::Crossover<Point>::unpack(reader);
          new (self) Crossover(std::move(base));
        }
      });
}

}  // namespace pyinterp::geometry::geographic::pybind

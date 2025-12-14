// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#include "pyinterp/geodetic/multipolygon.hpp"

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/tuple.h>
#include <nanobind/stl/vector.h>

#include <Eigen/Core>
#include <boost/geometry.hpp>
#include <format>
#include <sstream>
#include <stdexcept>

#include "pyinterp/geodetic/polygon.hpp"
#include "pyinterp/pybind/ndarray_serialization.hpp"

namespace nb = nanobind;
using nb::literals::operator""_a;

using pyinterp::pybind::NanobindArray1DUInt8;
using pyinterp::pybind::reader_from_ndarray;
using pyinterp::pybind::writer_to_ndarray;

namespace pyinterp::geodetic::pybind {

constexpr auto kMultiPolygonClassDoc = R"doc(
A collection of polygons in geographic coordinates.

Behaves like a Python container of `Polygon` objects.
)doc";

constexpr auto kMultiPolygonInitDoc = R"doc(
Construct a multipolygon from an optional sequence of polygons.

Args:
    polygons: Optional sequence of `Polygon` objects.
)doc";

// Proxy view exposing the underlying polygon container with Python semantics.
struct PolygonsView {
  MultiPolygon* mp;
  explicit PolygonsView(MultiPolygon* p) : mp(p) {}

  auto size() const -> size_t { return mp->size(); }
  auto get(Eigen::Index idx) -> Polygon& {
    if (idx < 0 || idx >= static_cast<Eigen::Index>(mp->size())) {
      throw std::out_of_range("MultiPolygon index out of range");
    }
    return (*mp)[static_cast<size_t>(idx)];
  }
  void set(Eigen::Index idx, const Polygon& poly) {
    if (idx < 0 || idx >= static_cast<Eigen::Index>(mp->size())) {
      throw std::out_of_range("MultiPolygon index out of range");
    }
    (*mp)[static_cast<size_t>(idx)] = poly;
  }
  void append(const Polygon& poly) { mp->push_back(poly); }
  void clear() { mp->clear(); }
};

auto init_multipolygon(nb::module_& m) -> void {
  nb::class_<MultiPolygon>(m, "MultiPolygon", kMultiPolygonClassDoc)
      .def(nb::init<>(), "Construct an empty multipolygon.")
      .def(
          "__init__",
          [](MultiPolygon* self, const std::vector<Polygon>& polygons) {
            new (self) MultiPolygon(polygons);
          },
          "polygons"_a = std::vector<Polygon>{}, kMultiPolygonInitDoc)

      // Container-like operations
      .def("__len__", &MultiPolygon::size, "Number of polygons.")

      .def(
          "__getitem__",
          [](MultiPolygon& self, Eigen::Index idx) -> Polygon& {
            if (idx < 0 || idx >= static_cast<Eigen::Index>(self.size())) {
              throw std::out_of_range("MultiPolygon index out of range");
            }
            return self[static_cast<size_t>(idx)];
          },
          nb::rv_policy::reference_internal, "Get polygon at index.")

      .def(
          "__setitem__",
          [](MultiPolygon& self, Eigen::Index idx, const Polygon& poly) {
            if (idx < 0 || idx >= static_cast<Eigen::Index>(self.size())) {
              throw std::out_of_range("MultiPolygon index out of range");
            }
            self[static_cast<size_t>(idx)] = poly;
          },
          "idx"_a, "poly"_a, "Set polygon at index.")

      .def("append", &MultiPolygon::push_back, "poly"_a,
           "Append a polygon to the collection.")

      .def("clear", &MultiPolygon::clear,
           "Remove all polygons from the collection.")

      .def(
          "__bool__", [](const MultiPolygon& self) { return !self.empty(); },
          "Return True if not empty.")

      .def(
          "__iter__",
          [](MultiPolygon& self) {
            nb::list items;
            for (size_t i = 0; i < self.size(); ++i) {
              items.append(self[static_cast<size_t>(i)]);
            }
            return items.attr("__iter__")();
          },
          "Iterate over polygons.")

      // View property over the underlying container
      .def_prop_rw(
          "polygons",
          [](MultiPolygon& self) {
            return nb::cast(PolygonsView(&self), nb::rv_policy::reference);
          },
          [](MultiPolygon& self, const nb::list& items) {
            std::vector<Polygon> polys;
            polys.reserve(items.size());
            for (size_t i = 0; i < items.size(); ++i) {
              polys.push_back(nb::cast<Polygon>(items[i]));
            }
            self.clear();
            for (const auto& p : polys) self.push_back(p);
          },
          nb::keep_alive<0, 1>(), nb::rv_policy::reference_internal,
          "View over polygons bound to multipolygon lifetime.")

      // Equality via boost geometry
      .def("__eq__",
           [](const MultiPolygon& a, const MultiPolygon& b) {
             return boost::geometry::equals(a, b);
           })
      .def("__ne__",
           [](const MultiPolygon& a, const MultiPolygon& b) {
             return !boost::geometry::equals(a, b);
           })

      // Repr/str
      .def("__repr__",
           [](const MultiPolygon& self) {
             return std::format("MultiPolygon({} polygons)", self.size());
           })
      .def("__str__",
           [](const MultiPolygon& self) {
             std::ostringstream oss;
             oss << "MultiPolygon[n=" << self.size() << "]";
             return oss.str();
           })

      // Pickle support
      .def("__getstate__",
           [](const MultiPolygon& self) {
             serialization::Writer state;
             {
               nb::gil_scoped_release release;
               state = self.pack();
             }
             return nb::make_tuple(writer_to_ndarray(std::move(state)));
           })
      .def("__setstate__", [](MultiPolygon* self, const nb::tuple& state) {
        if (state.size() != 1) {
          throw std::invalid_argument("Invalid state");
        }
        auto array = nanobind::cast<NanobindArray1DUInt8>(state[0]);
        auto reader = reader_from_ndarray(array);
        {
          nb::gil_scoped_release release;
          new (self) MultiPolygon(MultiPolygon::unpack(reader));
        }
      });

  // Bind view class
  nb::class_<PolygonsView>(m, "_PolygonsView")
      .def("__len__", &PolygonsView::size, "Number of polygons.")
      .def(
          "__getitem__",
          [](PolygonsView& view, Eigen::Index idx) -> Polygon& {
            return view.get(idx);
          },
          nb::rv_policy::reference_internal, "Get polygon at index.")
      .def(
          "__setitem__",
          [](PolygonsView& view, Eigen::Index idx, const Polygon& poly) {
            view.set(idx, poly);
          },
          "Set polygon at index.")
      .def("append", &PolygonsView::append, "Add a polygon.")
      .def("clear", &PolygonsView::clear, "Remove all polygons.")
      .def("__iter__", [](PolygonsView& view) {
        nb::list items;
        for (size_t i = 0; i < view.size(); ++i) {
          items.append(view.get(static_cast<Eigen::Index>(i)));
        }
        return items.attr("__iter__")();
      });
}

}  // namespace pyinterp::geodetic::pybind

#pragma once

#include "pyinterp/geometry/cartesian/point.hpp"
#include "pyinterp/geometry/multi_polygon.hpp"

namespace pyinterp::geometry::cartesian {

/// @brief MultiPolygon: collection of polygons.
///
/// A `MultiPolygon` represents a collection of polygons. It provides
/// basic container-like operations for constructing and iterating over
/// polygons.
using MultiPolygon = pyinterp::geometry::MultiPolygon<Point>;

}  // namespace pyinterp::geometry::cartesian

namespace boost::geometry::traits {

template <>
struct tag<pyinterp::geometry::cartesian::MultiPolygon> {
  using type = multi_polygon_tag;
};

}  // namespace boost::geometry::traits

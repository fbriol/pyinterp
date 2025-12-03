#pragma once

#include <boost/geometry.hpp>
#include <concepts>

#include "boost/geometry/core/cs.hpp"
#include "boost/geometry/geometries/point.hpp"

namespace pyinterp::geometry {

/// @brief 3D point in geodetic coordinates (longitude, latitude, altitude)
/// Longitude and latitude are expressed in degrees, altitude in meters
/// @tparam T Floating-point type
template <std::floating_point T>
using LLA = boost::geometry::model::point<
    T, 3, boost::geometry::cs::geographic<boost::geometry::degree>>;

/// @brief 3D point in Earth-Centered, Earth-Fixed (ECEF) Cartesian coordinates
/// x, y, z are expressed in meters
template <std::floating_point T>
using ECEF =
    boost::geometry::model::point<T, 3, boost::geometry::cs::cartesian>;

/// @brief 2D point in geodetic coordinates (longitude, latitude)
/// projected onto a spheroidal model for enhanced accuracy
/// Longitude and latitude are expressed in degrees
/// @tparam T Floating-point type
template <std::floating_point T>
using GeographicPoint = boost::geometry::model::point<
    T, 2, boost::geometry::cs::geographic<boost::geometry::degree>>;

/// @brief 2D point in spherical coordinates (longitude, latitude)
/// Uses a perfect sphere model for calculations
/// Longitude and latitude are expressed in degrees
/// @tparam T Floating-point type
template <std::floating_point T>
using SphericalPoint = boost::geometry::model::point<
    T, 2, boost::geometry::cs::spherical_equatorial<boost::geometry::degree>>;

}  // namespace pyinterp::geometry

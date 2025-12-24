#pragma once

#include <Eigen/Core>
#include <limits>
#include <optional>

#include "pyinterp/broadcast.hpp"
#include "pyinterp/geometry/cartesian/crossover.hpp"
#include "pyinterp/geometry/cartesian/linestring.hpp"
#include "pyinterp/geometry/cartesian/point.hpp"
#include "pyinterp/geometry/geographic/crossover.hpp"
#include "pyinterp/geometry/geographic/linestring.hpp"
#include "pyinterp/geometry/geographic/point.hpp"
#include "pyinterp/geometry/geographic/spheroid.hpp"
#include "pyinterp/math.hpp"

namespace pyinterp::geometry::satellite::algorithms {

/// @brief Result of a crossover detection
struct CrossoverResult {
  geographic::Point point;  ///< The crossover point
  size_t index1;            ///< Index of nearest vertex in first linestring
  size_t index2;            ///< Index of nearest vertex in second linestring
};

namespace detail {

/// @brief Check that coordinates of the half-orbits have same shape.
/// @param[in] lon1 Longitude array of the first half-orbit
/// @param[in] lat1 Latitude array of the first half-orbit
/// @param[in] lon2 Longitude array of the second half-orbit
/// @param[in] lat2 Latitude array of the second half-orbit
/// @throws std::invalid_argument if the shapes are not compatible
constexpr auto check_linestring_shapes(
    const Eigen::Ref<const Eigen::VectorXd>& lon1,
    const Eigen::Ref<const Eigen::VectorXd>& lat1,
    const Eigen::Ref<const Eigen::VectorXd>& lon2,
    const Eigen::Ref<const Eigen::VectorXd>& lat2, const double predicate)
    -> void {
  broadcast::check_eigen_shape("lon1", lon1, "lat1", lat1);
  broadcast::check_eigen_shape("lon2", lon2, "lat2", lat2);
  if (lon1.size() < 3) {
    throw std::invalid_argument(
        "The first linestring must have at least 3 points.");
  }
  if (lon2.size() < 3) {
    throw std::invalid_argument(
        "The second linestring must have at least 3 points.");
  }
  if (predicate <= 0.0 || !std::isfinite(predicate)) {
    throw std::invalid_argument(
        "The predicate distance must be a positive finite value.");
  }
}

/// @brief Check if a crossover point is acceptable based on the distance to
/// the nearest vertices in both linestrings.
/// @param[in] xover Crossover handler
/// @param[in] point Crossover point found
/// @param[in] predicate Maximum acceptable distance to consider a vertex as
/// nearest
/// @return The crossover point if acceptable; std::nullopt otherwise
inline auto filter_crossovers(const cartesian::Crossover& xover,
                              const cartesian::Point& point,
                              const double predicate)
    -> std::optional<CrossoverResult> {
  auto nearest = xover.nearest(point, std::numeric_limits<double>::max());
  if (!nearest) {
    return std::nullopt;
  }
  auto [ix1, ix2] = *nearest;
  // Convert the Cartesian crossover point to geographic coordinates in order
  // to compute distances to the nearest vertices.
  auto geographic_point = geographic::Point(
      math::normalize_period(point.get<0>(), -180.0, 360.0), point.get<1>());

  const auto& line1 = xover.line1();
  const auto& line2 = xover.line2();

  auto calculate_distance = [&geographic_point](
                                int64_t ix,
                                const cartesian::LineString& line) -> double {
    return boost::geometry::distance(
        geographic_point,
        geographic::Point(line[ix].get<0>(), line[ix].get<1>()));
  };

  auto distance = calculate_distance(ix1, line1);
  if (distance > predicate) {
    return std::nullopt;
  }
  distance = calculate_distance(ix2, line2);
  if (distance > predicate) {
    return std::nullopt;
  }
  return std::make_optional(
      CrossoverResult{.point = geographic_point, .index1 = ix1, .index2 = ix2});
}

}  // namespace detail

/// @brief Find a unique crossover point between two geographic linestrings.
/// @param[in] lon1 Longitude array of the first half-orbit
/// @param[in] lat1 Latitude array of the first half-orbit
/// @param[in] lon2 Longitude array of the second half-orbit
/// @param[in] lat2 Latitude array of the second half-orbit
/// @param[in] predicate Maximum acceptable distance to consider a vertex as
/// nearest
/// @param[in] strategy Calculation strategy
/// @param[in] spheroid Optional spheroid for geodetic calculations
/// @return An optional tuple containing the crossover point and the indices of
/// the nearest vertices in both linestrings if found; std::nullopt otherwise
/// @throws std::runtime_error if multiple crossover points are found
auto find_unique_crossover(const Eigen::Ref<const Eigen::VectorXd>& lon1,
                           const Eigen::Ref<const Eigen::VectorXd>& lat1,
                           const Eigen::Ref<const Eigen::VectorXd>& lon2,
                           const Eigen::Ref<const Eigen::VectorXd>& lat2,
                           const double predicate,
                           const geographic::StrategyMethod strategy,
                           const std::optional<geographic::Spheroid>& spheroid)
    -> std::optional<CrossoverResult> {
  detail::check_linestring_shapes(lon1, lat1, lon2, lat2, predicate);
  auto xover = geographic::Crossover(geographic::LineString(lon1, lat1),
                                     geographic::LineString(lon2, lat2));
  auto point = xover.find_unique(spheroid, strategy);
  if (!point) {
    return std::nullopt;
  }
  auto nearest = xover.nearest(*point, predicate);
  if (!nearest) {
    return std::nullopt;
  }

  return std::make_optional(CrossoverResult{.point = *point,
                                            .index1 = std::get<0>(*nearest),
                                            .index2 = std::get<1>(*nearest)});
}

/// @brief Find a unique crossover point between two cartesian linestrings.
///
/// The intersection search is performed on a Cartesian plane, which provides
/// faster results than geodetic calculations. However, this approach is only
/// appropriate when an approximate determination is acceptable. The accuracy
/// depends on the vertices of the linestrings being close to each other;
/// if they are widely spaced, the determined geographical point may be
/// significantly incorrect due to Cartesian approximation errors.
///
/// @param[in] lon1 Longitude array of the first half-orbit
/// @param[in] lat1 Latitude array of the first half-orbit
/// @param[in] lon2 Longitude array of the second half-orbit
/// @param[in] lat2 Latitude array of the second half-orbit
/// @param[in] predicate Maximum acceptable distance to consider a vertex as
/// nearest
/// @return An optional tuple containing the crossover point and the indices of
/// the nearest vertices in both linestrings if found; std::nullopt otherwise
/// @throws std::runtime_error if multiple crossover points are found
auto find_unique_crossover(const Eigen::Ref<const Eigen::VectorXd>& lon1,
                           const Eigen::Ref<const Eigen::VectorXd>& lat1,
                           const Eigen::Ref<const Eigen::VectorXd>& lon2,
                           const Eigen::Ref<const Eigen::VectorXd>& lat2,
                           const double predicate)
    -> std::optional<CrossoverResult> {
  detail::check_linestring_shapes(lon1, lat1, lon2, lat2, predicate);
  auto xover = cartesian::Crossover(cartesian::LineString(lon1, lat1),
                                    cartesian::LineString(lon2, lat2));
  auto cartesian_point = xover.find_unique();
  if (!cartesian_point) {
    return std::nullopt;
  }
  return detail::filter_crossovers(xover, *cartesian_point, predicate);
}

/// @brief Find all crossover points between two geographic linestrings.
/// @param[in] lon1 Longitude array of the first half-orbit
/// @param[in] lat1 Latitude array of the first half-orbit
/// @param[in] lon2 Longitude array of the second half-orbit
/// @param[in] lat2 Latitude array of the second half-orbit
/// @param[in] predicate Maximum acceptable distance to consider a vertex as
/// nearest
/// @param[in] strategy Calculation strategy
/// @param[in] spheroid Optional spheroid for geodetic calculations
/// @return All crossover points found as a MultiPoint object
auto find_all_crossovers(const Eigen::Ref<const Eigen::VectorXd>& lon1,
                         const Eigen::Ref<const Eigen::VectorXd>& lat1,
                         const Eigen::Ref<const Eigen::VectorXd>& lon2,
                         const Eigen::Ref<const Eigen::VectorXd>& lat2,
                         const double predicate,
                         const geographic::StrategyMethod strategy,
                         const std::optional<geographic::Spheroid>& spheroid)
    -> std::vector<CrossoverResult> {
  detail::check_linestring_shapes(lon1, lat1, lon2, lat2, predicate);
  auto xover = geographic::Crossover(geographic::LineString(lon1, lat1),
                                     geographic::LineString(lon2, lat2));
  auto result = std::vector<CrossoverResult>{};
  for (const auto& point : xover.find_all(spheroid, strategy)) {
    auto nearest = xover.nearest(point, predicate);
    if (nearest) {
      result.emplace_back(CrossoverResult{.point = point,
                                          .index1 = std::get<0>(*nearest),
                                          .index2 = std::get<1>(*nearest)});
    }
  }
  return result;
}

/// @brief Find all crossover points between two cartesian linestrings.
///
/// The intersection search is performed on a Cartesian plane, which provides
/// faster results than geodetic calculations. However, this approach is only
/// appropriate when an approximate determination is acceptable. The accuracy
/// depends on the vertices of the linestrings being close to each other;
/// if they are widely spaced, the determined geographical point may be
/// significantly incorrect due to Cartesian approximation errors.
///
/// @param[in] lon1 Longitude array of the first half-orbit
/// @param[in] lat1 Latitude array of the first half-orbit
/// @param[in] lon2 Longitude array of the second half-orbit
/// @param[in] lat2 Latitude array of the second half-orbit
/// @param[in] predicate Maximum acceptable distance to consider a vertex as
/// nearest
/// @return All crossover points found as a MultiPoint object
auto find_all_crossovers(const Eigen::Ref<const Eigen::VectorXd>& lon1,
                         const Eigen::Ref<const Eigen::VectorXd>& lat1,
                         const Eigen::Ref<const Eigen::VectorXd>& lon2,
                         const Eigen::Ref<const Eigen::VectorXd>& lat2,
                         const double predicate)
    -> std::vector<CrossoverResult> {
  detail::check_linestring_shapes(lon1, lat1, lon2, lat2, predicate);
  auto xover = cartesian::Crossover(cartesian::LineString(lon1, lat1),
                                    cartesian::LineString(lon2, lat2));
  auto result = std::vector<CrossoverResult>{};
  for (const auto& point : xover.find_all()) {
    auto filtered = detail::filter_crossovers(xover, point, predicate);
    if (filtered) {
      result.emplace_back(*filtered);
    }
  }
  return result;
}

}  // namespace pyinterp::geometry::satellite::algorithms

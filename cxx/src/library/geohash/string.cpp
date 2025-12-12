#include "pyinterp/geohash/string.hpp"

#include <algorithm>
#include <boost/geometry.hpp>
#include <ranges>
#include <string>
#include <unordered_set>

#include "pyinterp/broadcast.hpp"
#include "pyinterp/eigen.hpp"
#include "pyinterp/geohash/base32.hpp"
#include "pyinterp/math.hpp"
#include "pyinterp/parallel_for.hpp"

namespace pyinterp::geohash {

static Base32 encoder{};

auto encode(const Eigen::Ref<const Eigen::VectorXd>& lon,
            const Eigen::Ref<const Eigen::VectorXd>& lat, uint32_t precision)
    -> EncodedHashes {
  broadcast::check_eigen_shape("lon", lon, "lat", lat);

  auto size = lon.size();
  auto result = EncodedHashes{
      .buffer = std::vector<char>(size * precision),
      .precision = precision,
      .count = static_cast<size_t>(size),
  };

  for (auto [lon_item, lat_item, hash_span] :
       std::views::zip(lon, lat, result)) {
    encode({math::normalize_period(lon_item, -180.0, 360.0), lat_item},
           hash_span);
  }
  return result;
}

auto bounding_box(std::span<const char> geohash, uint32_t* precision)
    -> geodetic::Box {
  auto [integer_encoded, chars] = encoder.decode(geohash);
  if (precision != nullptr) {
    *precision = chars;
  }
  return int64::bounding_box(integer_encoded, 5 * chars);
}

auto neighbors(std::span<const char> hash) -> EncodedHashes {
  auto [integer_encoded, precision] = encoder.decode(hash);

  const auto integers = int64::neighbors(integer_encoded, precision * 5);
  auto result = EncodedHashes{
      .buffer = std::vector<char>(integers.size() * precision),
      .precision = precision,
      .count = integers.size(),
  };

  for (auto [integer, hash_span] : std::views::zip(integers, result)) {
    encoder.encode(integer, hash_span);
  }
  return result;
}

auto bounding_boxes(const std::optional<geodetic::Box>& box,
                    const uint32_t precision) -> EncodedHashes {
  // Number of bits
  auto bits = precision * 5;

  // Grid resolution in degrees
  const auto [lng_err, lat_err] = int64::error_with_precision(bits);

  // Property of the grid
  auto [hash_sw, lon_step, lat_step] = int64::grid_properties(
      box.value_or(geodetic::Box::global_bounding_box()), bits);

  // Prepare result
  auto result = EncodedHashes{
      .buffer = std::vector<char>(lon_step * lat_step * precision),
      .precision = precision,
      .count = static_cast<size_t>(lon_step * lat_step),
  };

  // Setup starting point (south-west corner)
  const auto point_sw = int64::decode(hash_sw, bits, false);

  // Fill the grid
  size_t ix = 0;
  for (size_t lat = 0; lat < lat_step; ++lat) {
    auto point =
        geodetic::Point(0, point_sw.lat() + static_cast<double>(lat) * lat_err);

    for (size_t lon = 0; lon < lon_step; ++lon) {
      point.lon() = point_sw.lon() + static_cast<double>(lon) * lng_err;

      Base32::encode(int64::encode(point, bits), result.get(ix++));
    }
  }
  return result;
}

namespace {

/// @brief Calculates a grid containing for each cell a boolean indicating if
/// the cell of the grid is enclosed or not in the geometry.
template <typename Geometry>
auto mask_cell(const geodetic::Box& envelope, const Geometry& geometry,
               double lng_err, double lat_err, const geodetic::Point& point_sw,
               size_t lon_step, size_t lat_step, uint32_t bits,
               size_t num_threads) -> Matrix<bool> {
  // Allocate the grid result
  auto result = Matrix<bool>(lon_step, lat_step);

  parallel_for(
      static_cast<int64_t>(lat_step),
      [&](int64_t start, int64_t end) {
        for (auto lat = start; lat < end; ++lat) {
          auto point = geodetic::Point(
              0, point_sw.lat() + static_cast<double>(lat) * lat_err);

          for (size_t lon = 0; lon < lon_step; ++lon) {
            point.lon() = point_sw.lon() + static_cast<double>(lon) * lng_err;
            result(lon, lat) = boost::geometry::intersects(
                int64::bounding_box(int64::encode(point, bits), bits),
                geometry);
          }
        }
      },
      static_cast<int64_t>(num_threads));

  return result;
}

/// @brief Return all GeoHash codes selected by the mask.
auto select_cell(double lng_err, double lat_err,
                 const geodetic::Point& point_sw, size_t lon_step,
                 size_t lat_step, uint32_t bits, uint32_t precision,
                 const Matrix<bool>& mask) -> EncodedHashes {
  // Count the number of cells that are enclosed by the polygon
  auto size = std::count(mask.data(), mask.data() + mask.size(), true);

  // Allocates the result
  auto result = EncodedHashes{
      .buffer = std::vector<char>(size * precision),
      .precision = precision,
      .count = static_cast<size_t>(size),
  };

  // For each cell of the grid, if it is selected, we add the code to the
  // result
  size_t result_ix = 0;
  for (size_t lat = 0; lat < lat_step; ++lat) {
    auto point =
        geodetic::Point(0, point_sw.lat() + static_cast<double>(lat) * lat_err);

    for (size_t lon = 0; lon < lon_step; ++lon) {
      if (mask(lon, lat)) {
        point.lon() = point_sw.lon() + static_cast<double>(lon) * lng_err;
        Base32::encode(int64::encode(point, bits), result.get(result_ix++));
      }
    }
  }

  return result;
}

/// @brief Common implementation for bounding_boxes with geometry
template <typename Geometry>
auto bounding_boxes_impl(const Geometry& geometry, uint32_t precision,
                         size_t num_threads) -> EncodedHashes {
  // Number of bits
  auto bits = precision * 5;

  // Bounding box of the grid to be created
  geodetic::Box envelope;
  boost::geometry::envelope(geometry, envelope);

  // Grid resolution in degrees
  const auto [lng_err, lat_err] = int64::error_with_precision(bits);

  // Property of the grid
  auto [hash_sw, lon_step, lat_step] = int64::grid_properties(envelope, bits);
  const auto point_sw = int64::decode(hash_sw, bits, false);

  // Calculates the intersection mask between the geometry and the GeoHash grid
  auto mask = mask_cell(envelope, geometry, lng_err, lat_err, point_sw,
                        lon_step, lat_step, bits, num_threads);

  // Finally, selects the geohashes that are enclosed in the geometry
  return select_cell(lng_err, lat_err, point_sw, lon_step, lat_step, bits,
                     precision, mask);
}

}  // anonymous namespace

auto bounding_boxes(const geodetic::Polygon& polygon, uint32_t precision,
                    size_t num_threads) -> EncodedHashes {
  return bounding_boxes_impl(polygon, precision, num_threads);
}

auto bounding_boxes(const geodetic::MultiPolygon& multipolygon,
                    uint32_t precision, size_t num_threads) -> EncodedHashes {
  return bounding_boxes_impl(multipolygon, precision, num_threads);
}

auto where(const EncodedHashes& hash, size_t rows, size_t cols)
    -> std::unordered_map<std::string,
                          std::tuple<std::tuple<int64_t, int64_t>,
                                     std::tuple<int64_t, int64_t>>> {
  // Index shifts of neighboring pixels
  static constexpr auto shift_row =
      std::array<int64_t, 8>{-1, -1, -1, 0, 1, 0, 1, 1};
  static constexpr auto shift_col =
      std::array<int64_t, 8>{-1, 1, 0, -1, -1, 1, 0, 1};

  auto result = std::unordered_map<
      std::string,
      std::tuple<std::tuple<int64_t, int64_t>, std::tuple<int64_t, int64_t>>>();

  for (int64_t ix = 0; ix < static_cast<int64_t>(rows); ++ix) {
    for (int64_t jx = 0; jx < static_cast<int64_t>(cols); ++jx) {
      auto current_span = hash.get(ix * cols + jx);
      auto current_code = std::string(current_span.begin(), current_span.end());

      auto it = result.find(current_code);
      if (it == result.end()) {
        result.emplace(current_code, std::make_tuple(std::make_tuple(ix, ix),
                                                     std::make_tuple(jx, jx)));
        continue;
      }

      for (int64_t kx = 0; kx < 8; ++kx) {
        const auto i = ix + shift_row[kx];
        const auto j = jx + shift_col[kx];

        if (i >= 0 && i < static_cast<int64_t>(rows) && j >= 0 &&
            j < static_cast<int64_t>(cols)) {
          auto neighboring_span = hash.get(i * cols + j);
          auto neighboring_code =
              std::string(neighboring_span.begin(), neighboring_span.end());

          if (current_code == neighboring_code) {
            auto& first = std::get<0>(it->second);
            std::get<0>(first) = std::min(std::get<0>(first), i);
            std::get<1>(first) = std::max(std::get<1>(first), i);

            auto& second = std::get<1>(it->second);
            std::get<0>(second) = std::min(std::get<0>(second), j);
            std::get<1>(second) = std::max(std::get<1>(second), j);
          }
        }
      }
    }
  }

  return result;
}

namespace {

/// @brief Zoom in from lower to higher precision
auto zoom_in(const EncodedHashes& hash, uint32_t to_precision)
    -> EncodedHashes {
  // Number of bits need to zoom in
  auto bits = to_precision * 5;

  // Calculation of the number of items needed for the result.
  auto size_in = hash.count * (static_cast<size_t>(2)
                               << (5 * (to_precision - hash.precision) - 1));

  // Allocates the result
  auto result = EncodedHashes{
      .buffer = std::vector<char>(size_in * to_precision),
      .precision = to_precision,
      .count = size_in,
  };

  size_t result_ix = 0;
  for (const auto& hash_span : hash) {
    auto bbox = bounding_box(hash_span);
    auto codes = int64::bounding_boxes(bbox, bits);
    for (uint64_t code : codes) {
      Base32::encode(code, result.get(result_ix++));
    }
  }

  return result;
}

/// @brief Zoom out from higher to lower precision
auto zoom_out(const EncodedHashes& hash, uint32_t to_precision)
    -> EncodedHashes {
  // Use unordered_set for O(1) insertions instead of O(log n) with std::set
  auto zoom_out_codes = std::unordered_set<uint64_t>();
  zoom_out_codes.reserve(hash.count);  // Reserve space to avoid rehashing

  auto to_bits = to_precision * 5;
  auto from_bits = hash.precision * 5;
  auto shift = from_bits - to_bits;

  for (const auto& hash_span : hash) {
    // Decode to integer, shift right to reduce precision, and collect unique
    // values
    auto [integer_hash, chars] = encoder.decode(hash_span);
    auto zoomed_out = integer_hash >> shift;
    zoom_out_codes.insert(zoomed_out);
  }

  auto result = EncodedHashes{
      .buffer = std::vector<char>(zoom_out_codes.size() * to_precision),
      .precision = to_precision,
      .count = zoom_out_codes.size(),
  };

  size_t ix = 0;
  for (auto code : zoom_out_codes) {
    encoder.encode(code, result.get(ix++));
  }

  return result;
}

}  // anonymous namespace

auto transform(const EncodedHashes& hash, uint32_t precision) -> EncodedHashes {
  if (hash.precision == precision) {
    return hash;
  }
  if (hash.precision > precision) {
    return zoom_out(hash, precision);
  }
  return zoom_in(hash, precision);
}

}  // namespace pyinterp::geohash

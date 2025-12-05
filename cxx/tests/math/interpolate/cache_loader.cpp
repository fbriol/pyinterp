#include "pyinterp/math/interpolate/cache_loader.hpp"

#include <gtest/gtest.h>
#include <sys/types.h>

#include "pyinterp/math/axis.hpp"

namespace pyinterp::math::interpolate {

// A simple 2D grid for testing purposes
struct Grid {
  // Number of dimensions
  static constexpr size_t kNDim = 2;

  // Out-of-bounds error message
  static constexpr const char* kOutOfBoundsMessage = "@Out of bounds@";

  // Build a grid with two axes and a simple matrix of values.
  //
  // The longitude axis is periodic (0 to 359 degrees) and the latitude axis is
  // non-periodic (-60 to 60 degrees).
  Grid() {
    std::get<0>(axes_) = math::Axis<double>(0, 359, 360, 1e-6, 360.0);
    std::get<1>(axes_) = math::Axis<double>(-60, 60, 121, 1e-6);
    matrix_.resize(360, 121);
    for (int64_t i = 0; i < 360; ++i) {
      for (int64_t j = 0; j < 121; ++j) {
        matrix_(i, j) = static_cast<uint8_t>((i + j) % 256);
      }
    }
  }

  // Construct an out-of-bounds error description for a given axis
  template <size_t I>
  [[nodiscard]] auto construct_bounds_error_description(
      const double& coordinate) const -> std::string {
    return kOutOfBoundsMessage;
  }

  // Return the axis at index I
  template <size_t I>
  [[nodiscard]] constexpr auto axis() const noexcept
      -> const math::Axis<double>& {
    return std::get<I>(axes_);
  }

  // Return the value at the given indices
  template <typename... Index>
  [[nodiscard]] auto value(Index&&... indices) const noexcept
      -> const uint8_t& {
    return matrix_(std::forward<Index>(indices)...);
  }

 private:
  // The axes of the grid
  std::tuple<math::Axis<double>, math::Axis<double>> axes_{};
  // The matrix of values
  Matrix<uint8_t> matrix_{};
};

// Update the cache if needed based on the query coordinates
TEST(CacheLoaderTest, LoadCacheGeneric) {
  Grid grid;
  InterpolationCache<double, double, double> cache(2, 2);

  // First load
  std::tuple<double, double> query_coords{359.5, 0.5};
  auto cached = update_cache_if_needed(cache, grid, query_coords,
                                       axis::Boundary::kWrap, true);
  EXPECT_TRUE(cached.success);
  EXPECT_TRUE(cached.was_updated);
  EXPECT_FALSE(cached.error_message.has_value());
  auto& lon = cache.coords<0>();
  EXPECT_EQ(lon[0], 358);
  EXPECT_EQ(lon[3], 361);

  // Second load with same coordinates
  cached = update_cache_if_needed(cache, grid, query_coords,
                                  axis::Boundary::kWrap, true);
  EXPECT_TRUE(cached.success);
  EXPECT_FALSE(cached.was_updated);
  EXPECT_FALSE(cached.error_message.has_value());

  // Third load with different coordinates inside the cached domain
  query_coords = std::make_tuple(359.51, .51);
  cached = update_cache_if_needed(cache, grid, query_coords,
                                  axis::Boundary::kWrap, true);
  EXPECT_TRUE(cached.success);
  EXPECT_FALSE(cached.was_updated);
  EXPECT_FALSE(cached.error_message.has_value());

  // Fourth load with out-of-bounds coordinates
  query_coords = std::make_tuple(0.0, -61.0);
  cached = update_cache_if_needed(cache, grid, query_coords,
                                  axis::Boundary::kWrap, true);
  EXPECT_FALSE(cached.success);
  EXPECT_TRUE(cached.was_updated);
  ASSERT_TRUE(cached.error_message.has_value());
  EXPECT_EQ(cached.error_message.value(), Grid::kOutOfBoundsMessage);

  // Fifth load with x coordinate within the periodic domain
  query_coords = std::make_tuple(-10.1, 0.1);
  cached = update_cache_if_needed(cache, grid, query_coords,
                                  axis::Boundary::kWrap, true);
  EXPECT_TRUE(cached.success);
  EXPECT_TRUE(cached.was_updated);
  EXPECT_FALSE(cached.error_message.has_value());
  lon = cache.coords<0>();
  EXPECT_NEAR(lon[0], -12, 1e-6);
  EXPECT_NEAR(lon[3], -9, 1e-6);

  // Sixth load with coordinates inside the cached domain (the cache domain
  // should have been shifted by the periodicity)
  cached = update_cache_if_needed(cache, grid, query_coords,
                                  axis::Boundary::kWrap, true);
  EXPECT_TRUE(cached.success);
  EXPECT_FALSE(cached.was_updated);
  EXPECT_FALSE(cached.error_message.has_value());
}

}  // namespace pyinterp::math::interpolate

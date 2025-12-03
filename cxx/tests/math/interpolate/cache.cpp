#include "pyinterp/math/interpolate/cache.hpp"

#include <gtest/gtest.h>

#include <cmath>

#include "pyinterp/math/interpolate/cache.hpp"

namespace pyinterp::math::interpolate {

// Helper to ensure correct macro expansion for multi-index operator[]
// (prevents issues with comma operator in EXPECT macros)
auto expect_float_eq(float a, float b) -> void { EXPECT_FLOAT_EQ(a, b); }

TEST(IndependentCache1D, Construction) {
  // 1D cache: only X dimension, Y is ignored
  InterpolationCache<float, double> cache_linear(1, 1);
  EXPECT_EQ(cache_linear.x_half_window(), 1);
  EXPECT_EQ(cache_linear.x_points(), 2);

  InterpolationCache<float, double> cache_cubic(2, 2);
  EXPECT_EQ(cache_cubic.x_half_window(), 2);
  EXPECT_EQ(cache_cubic.x_points(), 4);
}

TEST(IndependentCache1D, CoordinateAccess) {
  InterpolationCache<float, double> cache(2, 2);

  cache.set_coord<0>(0, 1.0);
  cache.set_coord<0>(1, 2.0);
  cache.set_coord<0>(2, 3.0);
  cache.set_coord<0>(3, 4.0);

  EXPECT_DOUBLE_EQ(cache.coord<0>(0), 1.0);
  EXPECT_DOUBLE_EQ(cache.coord<0>(3), 4.0);
}

// ==================== 2D Cache Tests ====================

TEST(IndependentCache2D, SymmetricWindow) {
  // Same window size for both dimensions (like before)
  InterpolationCache<float, double, double> cache(2, 2);

  EXPECT_EQ(cache.x_half_window(), 2);
  EXPECT_EQ(cache.y_half_window(), 2);
  EXPECT_EQ(cache.x_points(), 4);
  EXPECT_EQ(cache.y_points(), 4);
  EXPECT_EQ(cache.values_flat().size(), 16);  // 4×4
}

TEST(IndependentCache2D, AsymmetricWindow) {
  // Different window sizes: cubic in X, linear in Y
  InterpolationCache<float, double, double> cache(2, 1);

  EXPECT_EQ(cache.x_half_window(), 2);
  EXPECT_EQ(cache.y_half_window(), 1);
  EXPECT_EQ(cache.x_points(), 4);
  EXPECT_EQ(cache.y_points(), 2);
  EXPECT_EQ(cache.values_flat().size(), 8);  // 4×2
}

TEST(IndependentCache2D, ReverseAsymmetric) {
  // Different window sizes: linear in X, cubic in Y
  InterpolationCache<float, double, double> cache(1, 2);

  EXPECT_EQ(cache.x_half_window(), 1);
  EXPECT_EQ(cache.y_half_window(), 2);
  EXPECT_EQ(cache.x_points(), 2);
  EXPECT_EQ(cache.y_points(), 4);
  EXPECT_EQ(cache.values_flat().size(), 8);  // 2×4
}

TEST(IndependentCache2D, ValueAccessAsymmetric) {
  // 4×2 cache (cubic X, linear Y)
  InterpolationCache<float, double, double> cache(2, 1);

  // Set all values
  for (size_t i = 0; i < 4; ++i) {
    for (size_t j = 0; j < 2; ++j) {
      cache[i, j] = static_cast<float>(i * 10 + j);
    }
  }

  // Verify
  expect_float_eq(cache[0, 0], 0.0f);
  expect_float_eq(cache[0, 1], 1.0f);
  expect_float_eq(cache[3, 0], 30.0f);
  expect_float_eq(cache[3, 1], 31.0f);
}

TEST(IndependentCache2D, MatrixViewAsymmetric) {
  // 4×2 cache
  InterpolationCache<float, double, double> cache(2, 1);

  for (size_t i = 0; i < 4; ++i) {
    for (size_t j = 0; j < 2; ++j) {
      cache[i, j] = static_cast<float>(i + j);
    }
  }

  auto mat = cache.matrix();

  EXPECT_EQ(mat.rows(), 4);
  EXPECT_EQ(mat.cols(), 2);
  EXPECT_FLOAT_EQ(mat(2, 1), 3.0f);  // i=2, j=1: 2+1=3
}

TEST(IndependentCache2D, DomainTrackingAsymmetric) {
  // Cubic X, Linear Y
  InterpolationCache<float, double, double> cache(2, 1);

  // Set X coordinates (4 points)
  for (size_t i = 0; i < 4; ++i) {
    cache.set_coord<0>(i, static_cast<double>(i) * 10.0);
  }

  // Set Y coordinates (2 points)
  for (size_t j = 0; j < 2; ++j) {
    cache.set_coord<1>(j, static_cast<double>(j) * 100.0);
  }

  cache.finalize();

  EXPECT_TRUE(cache.has_domain());

  // X domain: window=2, points [0,10,20,30], domain=[10,20]
  // Y domain: window=1, points [0,100], domain=[0,100]
  EXPECT_TRUE(cache.contains(15.0, 50.0));
  EXPECT_TRUE(cache.contains(10.0, 0.0));
  EXPECT_TRUE(cache.contains(20.0, 100.0));
  EXPECT_FALSE(cache.contains(5.0, 50.0));    // X out of domain
  EXPECT_FALSE(cache.contains(15.0, 150.0));  // Y out of domain
}

// ==================== 3D Cache Tests ====================

TEST(IndependentCache3D, SymmetricXY) {
  // X and Y symmetric, Z always 4 points
  InterpolationCache<float, double, double, double> cache(2, 2);

  EXPECT_EQ(cache.x_points(), 4);
  EXPECT_EQ(cache.y_points(), 4);
  EXPECT_EQ(cache.points_per_dim(2), 4);      // Z always 4
  EXPECT_EQ(cache.values_flat().size(), 64);  // 4×4×4
}

TEST(IndependentCache3D, AsymmetricXY) {
  // X cubic, Y linear, Z always 4
  InterpolationCache<float, double, double, double> cache(2, 1);

  EXPECT_EQ(cache.x_points(), 4);
  EXPECT_EQ(cache.y_points(), 2);
  EXPECT_EQ(cache.points_per_dim(2), 4);      // Z always 4
  EXPECT_EQ(cache.values_flat().size(), 32);  // 4×2×4
}

TEST(IndependentCache3D, MatrixSliceAsymmetric) {
  // 4×2×4 cache (cubic X, linear Y, cubic Z)
  InterpolationCache<float, double, double, double> cache(2, 1);

  // Fill z=2 slice
  for (size_t i = 0; i < 4; ++i) {
    for (size_t j = 0; j < 2; ++j) {
      cache[i, j, 2] = static_cast<float>(i * 10 + j);
    }
  }

  auto slice = cache.matrix(2);

  EXPECT_EQ(slice.rows(), 4);  // X dimension
  EXPECT_EQ(slice.cols(), 2);  // Y dimension
  EXPECT_FLOAT_EQ(slice(1, 0), 10.0f);
  EXPECT_FLOAT_EQ(slice(3, 1), 31.0f);
}

TEST(IndependentCache3D, DomainTracking) {
  InterpolationCache<float, double, double, double> cache(2, 1);

  // Set coordinates
  for (size_t i = 0; i < 4; ++i) {
    cache.set_coord<0>(i, static_cast<double>(i) * 10.0);  // X: [0,10,20,30]
  }
  for (size_t j = 0; j < 2; ++j) {
    cache.set_coord<1>(j, static_cast<double>(j) * 100.0);  // Y: [0,100]
  }
  for (size_t k = 0; k < 4; ++k) {
    cache.set_coord<2>(k, static_cast<double>(k) * 5.0);  // Z: [0,5,10,15]
  }

  cache.finalize();

  EXPECT_TRUE(cache.has_domain());

  // X domain: [10, 20], Y domain: [0, 100], Z domain: [5, 10]
  EXPECT_TRUE(cache.contains(15.0, 50.0, 7.5));
  EXPECT_FALSE(cache.contains(5.0, 50.0, 7.5));    // X out
  EXPECT_FALSE(cache.contains(15.0, 150.0, 7.5));  // Y out
  EXPECT_FALSE(cache.contains(15.0, 50.0, 12.0));  // Z out
}

// ==================== 4D Cache Tests ====================

TEST(IndependentCache4D, AsymmetricXY) {
  // X cubic, Y linear, Z and U always 4
  InterpolationCache<float, double, double, double, double> cache(2, 1);

  EXPECT_EQ(cache.x_points(), 4);
  EXPECT_EQ(cache.y_points(), 2);
  EXPECT_EQ(cache.points_per_dim(2), 4);       // Z always 4
  EXPECT_EQ(cache.points_per_dim(3), 4);       // U always 4
  EXPECT_EQ(cache.values_flat().size(), 128);  // 4×2×4×4
}

TEST(IndependentCache4D, MatrixSliceAsymmetric) {
  InterpolationCache<float, double, double, double, double> cache(2, 1);

  // Fill (z=1, u=2) slice
  for (size_t i = 0; i < 4; ++i) {
    for (size_t j = 0; j < 2; ++j) {
      cache[i, j, 1, 2] = static_cast<float>(i * 10 + j);
    }
  }

  auto slice = cache.matrix(1, 2);

  EXPECT_EQ(slice.rows(), 4);  // X
  EXPECT_EQ(slice.cols(), 2);  // Y
  EXPECT_FLOAT_EQ(slice(2, 1), 21.0f);
}

// ==================== Different Window Combinations ====================

TEST(WindowCombinations, LinearLinear) {
  // Linear in both X and Y
  InterpolationCache<float, double, double> cache(1, 1);

  EXPECT_EQ(cache.x_points(), 2);
  EXPECT_EQ(cache.y_points(), 2);
  EXPECT_EQ(cache.values_flat().size(), 4);  // 2×2

  cache[0, 0] = 1.0f;
  cache[0, 1] = 2.0f;
  cache[1, 0] = 3.0f;
  cache[1, 1] = 4.0f;

  auto mat = cache.matrix();
  EXPECT_FLOAT_EQ(mat.sum(), 10.0f);
}

TEST(WindowCombinations, CubicLinear) {
  // Cubic X, Linear Y
  InterpolationCache<float, double, double> cache(2, 1);

  EXPECT_EQ(cache.x_points(), 4);
  EXPECT_EQ(cache.y_points(), 2);
  EXPECT_EQ(cache.values_flat().size(), 8);  // 4×2
}

TEST(WindowCombinations, LinearCubic) {
  // Linear X, Cubic Y
  InterpolationCache<float, double, double> cache(1, 2);

  EXPECT_EQ(cache.x_points(), 2);
  EXPECT_EQ(cache.y_points(), 4);
  EXPECT_EQ(cache.values_flat().size(), 8);  // 2×4
}

TEST(WindowCombinations, CubicCubic) {
  // Cubic in both
  InterpolationCache<float, double, double> cache(2, 2);

  EXPECT_EQ(cache.x_points(), 4);
  EXPECT_EQ(cache.y_points(), 4);
  EXPECT_EQ(cache.values_flat().size(), 16);  // 4×4
}

TEST(WindowCombinations, QuinticLinear) {
  // Quintic X, Linear Y
  InterpolationCache<float, double, double> cache(3, 1);

  EXPECT_EQ(cache.x_points(), 6);
  EXPECT_EQ(cache.y_points(), 2);
  EXPECT_EQ(cache.values_flat().size(), 12);  // 6×2
}

TEST(WindowCombinations, LinearQuintic) {
  // Linear X, Quintic Y
  InterpolationCache<float, double, double> cache(1, 3);

  EXPECT_EQ(cache.x_points(), 2);
  EXPECT_EQ(cache.y_points(), 6);
  EXPECT_EQ(cache.values_flat().size(), 12);  // 2×6
}

TEST(WindowCombinations, QuinticCubic) {
  // Quintic X, Cubic Y
  InterpolationCache<float, double, double> cache(3, 2);

  EXPECT_EQ(cache.x_points(), 6);
  EXPECT_EQ(cache.y_points(), 4);
  EXPECT_EQ(cache.values_flat().size(), 24);  // 6×4
}

// ==================== Stride Tests ====================

TEST(Strides, AsymmetricLayout) {
  // 4×2 cache (cubic X, linear Y)
  InterpolationCache<float, double, double> cache(2, 1);

  // Fill with sequential values
  auto& flat = cache.values_flat();
  for (size_t i = 0; i < flat.size(); ++i) {
    flat[i] = static_cast<float>(i);
  }

  // Strides for 4×2: [2, 1]
  // index = i*2 + j*1

  expect_float_eq(cache[0, 0], 0.0f);  // 0*2 + 0*1 = 0
  expect_float_eq(cache[0, 1], 1.0f);  // 0*2 + 1*1 = 1
  expect_float_eq(cache[1, 0], 2.0f);  // 1*2 + 0*1 = 2
  expect_float_eq(cache[1, 1], 3.0f);  // 1*2 + 1*1 = 3
  expect_float_eq(cache[3, 0], 6.0f);  // 3*2 + 0*1 = 6
  expect_float_eq(cache[3, 1], 7.0f);  // 3*2 + 1*1 = 7
}

TEST(Strides, 3DAsymmetric) {
  // 4×2×4 cache
  InterpolationCache<float, double, double, double> cache(2, 1);

  auto& flat = cache.values_flat();
  for (size_t i = 0; i < flat.size(); ++i) {
    flat[i] = static_cast<float>(i);
  }

  // Strides for 4×2×4: [8, 4, 1]
  // index = i*8 + j*4 + k*1

  expect_float_eq(cache[0, 0, 0], 0.0f);   // 0*8 + 0*4 + 0*1 = 0
  expect_float_eq(cache[0, 0, 1], 1.0f);   // 0*8 + 0*4 + 1*1 = 1
  expect_float_eq(cache[0, 1, 0], 4.0f);   // 0*8 + 1*4 + 0*1 = 4
  expect_float_eq(cache[1, 0, 0], 8.0f);   // 1*8 + 0*4 + 0*1 = 8
  expect_float_eq(cache[2, 1, 3], 23.0f);  // 2*8 + 1*4 + 3*1 = 23
}

// ==================== Matrix Operations ====================

TEST(MatrixOps, AsymmetricOperations) {
  // 4×2 cache
  InterpolationCache<float, double, double> cache(2, 1);

  for (size_t i = 0; i < 4; ++i) {
    for (size_t j = 0; j < 2; ++j) {
      cache[i, j] = static_cast<float>((i + 1) * (j + 1));
    }
  }

  auto mat = cache.matrix();

  // Matrix:
  // [1, 2]
  // [2, 4]
  // [3, 6]
  // [4, 8]

  EXPECT_FLOAT_EQ(mat.sum(), 30.0f);   // 1+2+2+4+3+6+4+8
  EXPECT_FLOAT_EQ(mat.mean(), 3.75f);  // 30/8

  auto row2 = mat.row(2);
  EXPECT_FLOAT_EQ(row2.sum(), 9.0f);  // 3+6

  auto col1 = mat.col(1);
  EXPECT_FLOAT_EQ(col1.sum(), 20.0f);  // 2+4+6+8
}

// ==================== Performance Tests ====================

TEST(Performance, AsymmetricCache) {
  InterpolationCache<float, double, double> cache(2, 1);

  constexpr int iterations = 100000;
  auto start = std::chrono::high_resolution_clock::now();

  for (int iter = 0; iter < iterations; ++iter) {
    for (size_t i = 0; i < 4; ++i) {
      for (size_t j = 0; j < 2; ++j) {
        cache[i, j] = static_cast<float>(i * j);
      }
    }
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

  auto time_ns = duration / static_cast<double>(iterations);
  RecordProperty("AsymmetricCacheTimeNS", time_ns);
}

}  // namespace pyinterp::math::interpolate

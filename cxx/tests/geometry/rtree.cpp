#include "pyinterp/geometry/rtree.hpp"

#include <gtest/gtest.h>

#include <algorithm>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <cmath>
#include <limits>
#include <utility>
#include <vector>

#include "pyinterp/math/interpolate/kriging.hpp"
#include "pyinterp/math/interpolate/rbf.hpp"
#include "pyinterp/math/interpolate/window_function.hpp"

using Point2d =
    boost::geometry::model::point<double, 2, boost::geometry::cs::cartesian>;
using RTree2d = pyinterp::geometry::RTree<Point2d, double>;

using Point3d =
    boost::geometry::model::point<double, 3, boost::geometry::cs::cartesian>;
using RTree3d = pyinterp::geometry::RTree<Point3d, double>;

namespace interpolate = pyinterp::math::interpolate;

TEST(RTree2D, BasicInsertAndQuery) {
  RTree2d tree;
  std::vector<RTree2d::value_t> data = {{Point2d(0.0, 0.0), 1.0},
                                        {Point2d(1.0, 1.0), 2.0},
                                        {Point2d(2.0, 2.0), 3.0},
                                        {Point2d(3.0, 3.0), 4.0}};
  for (const auto& v : data) tree.insert(v);
  EXPECT_EQ(tree.size(), data.size());
  EXPECT_FALSE(tree.empty());
  auto bounds = tree.bounds();
  ASSERT_TRUE(bounds.has_value());
  tree.clear();
  EXPECT_EQ(tree.size(), 0);
  EXPECT_TRUE(tree.empty());
}

TEST(RTree2D, Packing) {
  RTree2d tree;
  std::vector<RTree2d::value_t> data = {{Point2d(0.0, 0.0), 1.0},
                                        {Point2d(1.0, 1.0), 2.0},
                                        {Point2d(2.0, 2.0), 3.0}};
  tree.packing(data);
  EXPECT_EQ(tree.size(), data.size());
}

TEST(RTree2D, KNNQuery) {
  RTree2d tree;
  for (int i = 0; i < 10; ++i) {
    tree.insert({Point2d(i, i), static_cast<double>(i)});
  }
  auto result = tree.query(Point2d(5.1, 5.1), 3);
  ASSERT_EQ(result.size(), 3);
  EXPECT_NEAR(result[0].second, 5.0, 1e-12);
}

TEST(RTree2D, QueryBall) {
  RTree2d tree;
  for (int i = 0; i < 10; ++i) {
    tree.insert({Point2d(i, 0), static_cast<double>(i)});
  }
  auto result = tree.query_ball(Point2d(5, 0), 2.0);
  std::vector<double> found;
  found.reserve(result.size());
  for (const auto& [dist, val] : result) {
    found.push_back(val);
  }
  EXPECT_TRUE(std::ranges::find(found, 4.0) != found.end());
  EXPECT_TRUE(std::ranges::find(found, 5.0) != found.end());
  EXPECT_TRUE(std::ranges::find(found, 6.0) != found.end());
}

TEST(RTree2D, QueryWithin) {
  RTree2d tree;
  for (int i = 0; i < 10; ++i) {
    tree.insert({Point2d(i, 0), static_cast<double>(i)});
  }
  auto result = tree.query_within(Point2d(5, 0), 3);
  // May be empty if not surrounded, but should not crash
  EXPECT_LE(result.size(), 3);
}

TEST(RTree2D, ValueQuery) {
  RTree2d tree;
  for (int i = 0; i < 10; ++i) {
    tree.insert({Point2d(i, 0), static_cast<double>(i)});
  }
  auto result = tree.value(Point2d(5, 0), 2.0, 3, false);
  EXPECT_LE(result.size(), 3);
}

TEST(RTree2D, InverseDistanceWeighting) {
  RTree2d tree;
  for (int i = 0; i < 10; ++i) {
    tree.insert({Point2d(i, 0), static_cast<double>(i)});
  }
  auto [val, n] =
      tree.inverse_distance_weighting(Point2d(5, 0), 2.0, 3, 2, false);
  EXPECT_TRUE(std::isfinite(val) || std::isnan(val));
  EXPECT_LE(n, 3);
}

TEST(RTree2D, KrigingInstantiation) {
  using promotion_t = decltype(std::declval<double>() + std::declval<double>());
  interpolate::Kriging<promotion_t> model(
      1.0, 1.0, 1.0, interpolate::CovarianceFunction::kGaussian);
  RTree2d tree;
  for (int i = 0; i < 3; ++i) {
    tree.insert({Point2d(i, 0), static_cast<double>(i)});
  }
  auto [val, n] = tree.kriging(Point2d(1, 0), 2.0, 3, false, model);
  EXPECT_TRUE(std::isfinite(val) || std::isnan(val));
  EXPECT_LE(n, 3);
}

TEST(RTree2D, RBFInstantiation) {
  using promotion_t = decltype(std::declval<double>() + std::declval<double>());
  interpolate::RBF<promotion_t> rbf(
      std::numeric_limits<double>::quiet_NaN(), 0,
      interpolate::RadialBasisFunction::kMultiquadric);
  RTree2d tree;
  for (int i = 0; i < 3; ++i) {
    tree.insert({Point2d(i, 0), static_cast<double>(i)});
  }
  auto [val, n] = tree.radial_basis_function(Point2d(1, 0), rbf, 2.0, 3, false);
  EXPECT_TRUE(std::isfinite(val) || std::isnan(val));
  EXPECT_LE(n, 3);
}

TEST(RTree2D, WindowFunctionInstantiation) {
  RTree2d tree;
  for (int i = 0; i < 3; ++i) {
    tree.insert({Point2d(i, 0), static_cast<double>(i)});
  }
  interpolate::WindowFunction<double> wf(
      interpolate::window::Function::kHamming);
  auto [val, n] = tree.window_function(Point2d(1, 0), wf, 0.5, 2.0, 3, false);
  EXPECT_TRUE(std::isfinite(val) || std::isnan(val));
  EXPECT_LE(n, 3);
}

// 3D Test Cases
TEST(RTree3D, BasicInsertAndQuery) {
  RTree3d tree;
  std::vector<RTree3d::value_t> data = {{Point3d(0.0, 0.0, 0.0), 1.0},
                                        {Point3d(1.0, 1.0, 1.0), 2.0},
                                        {Point3d(2.0, 2.0, 2.0), 3.0},
                                        {Point3d(3.0, 3.0, 3.0), 4.0}};
  for (const auto& v : data) tree.insert(v);
  EXPECT_EQ(tree.size(), data.size());
  EXPECT_FALSE(tree.empty());
  auto bounds = tree.bounds();
  ASSERT_TRUE(bounds.has_value());
  tree.clear();
  EXPECT_EQ(tree.size(), 0);
  EXPECT_TRUE(tree.empty());
}

TEST(RTree3D, Packing) {
  RTree3d tree;
  std::vector<RTree3d::value_t> data = {{Point3d(0.0, 0.0, 0.0), 1.0},
                                        {Point3d(1.0, 1.0, 1.0), 2.0},
                                        {Point3d(2.0, 2.0, 2.0), 3.0}};
  tree.packing(data);
  EXPECT_EQ(tree.size(), data.size());
}

TEST(RTree3D, KNNQuery) {
  RTree3d tree;
  for (int i = 0; i < 10; ++i) {
    tree.insert({Point3d(i, i, i), static_cast<double>(i)});
  }
  auto result = tree.query(Point3d(5.1, 5.1, 5.1), 3);
  ASSERT_EQ(result.size(), 3);
  EXPECT_NEAR(result[0].second, 5.0, 1e-12);
}

TEST(RTree3D, QueryBall) {
  RTree3d tree;
  for (int i = 0; i < 10; ++i) {
    tree.insert({Point3d(i, 0, 0), static_cast<double>(i)});
  }
  auto result = tree.query_ball(Point3d(5, 0, 0), 2.0);
  std::vector<double> found;
  found.reserve(result.size());
  for (const auto& [dist, val] : result) {
    found.push_back(val);
  }
  EXPECT_TRUE(std::ranges::find(found, 4.0) != found.end());
  EXPECT_TRUE(std::ranges::find(found, 5.0) != found.end());
  EXPECT_TRUE(std::ranges::find(found, 6.0) != found.end());
}

TEST(RTree3D, QueryWithin) {
  RTree3d tree;
  for (int i = 0; i < 10; ++i) {
    tree.insert({Point3d(i, 0, 0), static_cast<double>(i)});
  }
  auto result = tree.query_within(Point3d(5, 0, 0), 3);
  // May be empty if not surrounded, but should not crash
  EXPECT_LE(result.size(), 3);
}

TEST(RTree3D, ValueQuery) {
  RTree3d tree;
  for (int i = 0; i < 10; ++i) {
    tree.insert({Point3d(i, 0, 0), static_cast<double>(i)});
  }
  auto result = tree.value(Point3d(5, 0, 0), 2.0, 3, false);
  EXPECT_LE(result.size(), 3);
}

TEST(RTree3D, InverseDistanceWeighting) {
  RTree3d tree;
  for (int i = 0; i < 10; ++i) {
    tree.insert({Point3d(i, 0, 0), static_cast<double>(i)});
  }
  auto [val, n] =
      tree.inverse_distance_weighting(Point3d(5, 0, 0), 2.0, 3, 2, false);
  EXPECT_TRUE(std::isfinite(val) || std::isnan(val));
  EXPECT_LE(n, 3);
}

TEST(RTree3D, KrigingInstantiation) {
  using promotion_t = decltype(std::declval<double>() + std::declval<double>());
  interpolate::Kriging<promotion_t> model(
      1.0, 1.0, 1.0, interpolate::CovarianceFunction::kGaussian);
  RTree3d tree;
  for (int i = 0; i < 3; ++i) {
    tree.insert({Point3d(i, 0, 0), static_cast<double>(i)});
  }
  auto [val, n] = tree.kriging(Point3d(1, 0, 0), 2.0, 3, false, model);
  EXPECT_TRUE(std::isfinite(val) || std::isnan(val));
  EXPECT_LE(n, 3);
}

TEST(RTree3D, RBFInstantiation) {
  using promotion_t = decltype(std::declval<double>() + std::declval<double>());
  interpolate::RBF<promotion_t> rbf(
      std::numeric_limits<double>::quiet_NaN(), 0,
      interpolate::RadialBasisFunction::kMultiquadric);
  RTree3d tree;
  for (int i = 0; i < 3; ++i) {
    tree.insert({Point3d(i, 0, 0), static_cast<double>(i)});
  }
  auto [val, n] =
      tree.radial_basis_function(Point3d(1, 0, 0), rbf, 2.0, 3, false);
  EXPECT_TRUE(std::isfinite(val) || std::isnan(val));
  EXPECT_LE(n, 3);
}

TEST(RTree3D, WindowFunctionInstantiation) {
  RTree3d tree;
  for (int i = 0; i < 3; ++i) {
    tree.insert({Point3d(i, 0, 0), static_cast<double>(i)});
  }
  interpolate::WindowFunction<double> wf(
      interpolate::window::Function::kHamming);
  auto [val, n] =
      tree.window_function(Point3d(1, 0, 0), wf, 0.5, 2.0, 3, false);
  EXPECT_TRUE(std::isfinite(val) || std::isnan(val));
  EXPECT_LE(n, 3);
}

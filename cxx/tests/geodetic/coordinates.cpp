#include "pyinterp/geodetic/coordinates.hpp"

#include <gtest/gtest.h>

#include <random>

#include "boost/geometry/core/access.hpp"
#include "pyinterp/geodetic/spheroid.hpp"
#include "pyinterp/geometry/point.hpp"

namespace geodetic = pyinterp::geodetic;
namespace geometry = pyinterp::geometry;

TEST(Coordinates, LLAtoECEF) {
  // conversion from ECEF to LLA
  auto tls_lla = geometry::LLA<double>(1.367331864, 43.634330796, 146);
  // Computed by pyproj
  auto tls_ecef = geometry::ECEF<double>(4622395.2942195125, 110331.83487903349,
                                         4378876.426388506);
  auto ecef = geodetic::Coordinates(geodetic::Spheroid()).lla_to_ecef(tls_lla);
  EXPECT_NEAR(boost::geometry::get<0>(tls_ecef), boost::geometry::get<0>(ecef),
              1e-12);
  EXPECT_NEAR(boost::geometry::get<1>(tls_ecef), boost::geometry::get<1>(ecef),
              1e-12);
  EXPECT_NEAR(boost::geometry::get<2>(tls_ecef), boost::geometry::get<2>(ecef),
              1e-12);
}

TEST(Coordinates, ECEFtoLLA) {
  // conversion from ECEF to LLA
  auto tls_ecef = geometry::ECEF<double>(4622395.2942195125, 110331.83487903349,
                                         4378876.426388506);
  // Computed by pyproj
  auto tls_lla = geometry::LLA<double>(1.3673318639999998, 43.63433079599999,
                                       146.00000000093132);
  auto lla = geodetic::Coordinates(geodetic::Spheroid()).ecef_to_lla(tls_ecef);
  EXPECT_NEAR(boost::geometry::get<0>(tls_lla), boost::geometry::get<0>(lla),
              1e-12);
  EXPECT_NEAR(boost::geometry::get<1>(tls_lla), boost::geometry::get<1>(lla),
              1e-12);
  EXPECT_NEAR(boost::geometry::get<2>(tls_lla), boost::geometry::get<2>(lla),
              1e-9);
}

TEST(Coordinates, LLAtoECEFtoLLA) {
  // statistical validation of the accuracy of the method used
  // (10-8 m) when converting from ECEF to LLA coordinates

  std::uniform_real_distribution<double> lat(-90, 90);
  std::uniform_real_distribution<double> lon(-180, 180);
  std::uniform_real_distribution<double> alt(-10'000, 100'000);
  std::default_random_engine re;

  auto coordinates = geodetic::Coordinates(geodetic::Spheroid());

  for (int ix = 0; ix < 1'000'000; ix++) {
    auto ref = geometry::LLA<double>(lon(re), lat(re), alt(re));
    auto ecef = coordinates.lla_to_ecef(ref);
    auto lla = coordinates.ecef_to_lla(ecef);

    EXPECT_NEAR(boost::geometry::get<0>(ref), boost::geometry::get<0>(lla),
                1e-12);
    EXPECT_NEAR(boost::geometry::get<1>(ref), boost::geometry::get<1>(lla),
                1e-12);
    EXPECT_NEAR(boost::geometry::get<2>(ref), boost::geometry::get<2>(lla),
                1e-8);
  }
}

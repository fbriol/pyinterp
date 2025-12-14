#include "pyinterp/geodetic/multi_polygon.hpp"

namespace pyinterp::geodetic {

auto MultiPolygon::unpack(serialization::Reader& state) -> MultiPolygon {
  if (state.size() == 0) {
    throw std::invalid_argument(
        "Cannot restore multipolygon from empty state.");
  }
  auto magic_number = state.read<uint32_t>();
  if (magic_number != kMagicNumber) {
    throw std::invalid_argument(
        "Invalid multipolygon state (bad magic number).");
  }
  auto num_polygons = state.read<size_t>();
  std::vector<Polygon> polygons;
  polygons.reserve(num_polygons);
  for (size_t i = 0; i < num_polygons; ++i) {
    auto polygon_state = state.read_vector<std::byte>();
    auto reader = serialization::Reader(std::move(polygon_state));
    polygons.emplace_back(Polygon::unpack(reader));
  }
  return MultiPolygon(std::move(polygons));
}

}  // namespace pyinterp::geodetic

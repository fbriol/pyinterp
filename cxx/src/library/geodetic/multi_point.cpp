#include "pyinterp/geodetic/multi_point.hpp"

namespace pyinterp::geodetic {

auto MultiPoint::unpack(serialization::Reader& state) -> MultiPoint {
  if (state.size() == 0) {
    throw std::invalid_argument("Cannot restore multipoint from empty state.");
  }
  auto magic_number = state.read<uint32_t>();
  if (magic_number != kMagicNumber) {
    throw std::invalid_argument("Invalid multipoint state (bad magic number).");
  }
  auto points = state.read_vector<Point>();
  return MultiPoint(std::move(points));
}

}  // namespace pyinterp::geodetic

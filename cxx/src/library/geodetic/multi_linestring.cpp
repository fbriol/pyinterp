#include "pyinterp/geometry/geographic/multi_linestring.hpp"

namespace pyinterp::geometry::geographic {

auto MultiLineString::unpack(serialization::Reader& state) -> MultiLineString {
  if (state.size() == 0) {
    throw std::invalid_argument(
        "Cannot restore multilinestring from empty state.");
  }
  auto magic_number = state.read<uint32_t>();
  if (magic_number != kMagicNumber) {
    throw std::invalid_argument(
        "Invalid multilinestring state (bad magic number).");
  }
  auto num_lines = state.read<size_t>();
  std::vector<LineString> lines;
  lines.reserve(num_lines);
  for (size_t i = 0; i < num_lines; ++i) {
    auto ls_state = state.read_vector<std::byte>();
    auto reader = serialization::Reader(std::move(ls_state));
    lines.emplace_back(LineString::unpack(reader));
  }
  return MultiLineString(std::move(lines));
}

}  // namespace pyinterp::geometry::geographic

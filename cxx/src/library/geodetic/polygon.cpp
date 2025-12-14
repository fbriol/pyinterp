#include "pyinterp/geodetic/polygon.hpp"

namespace pyinterp::geodetic {

auto Polygon::unpack(serialization::Reader& state) -> Polygon {
  if (state.size() == 0) {
    throw std::invalid_argument("Cannot restore polygon from empty state.");
  }
  auto magic_number = state.read<uint32_t>();
  if (magic_number != kMagicNumber) {
    throw std::invalid_argument("Invalid polygon state (bad magic number).");
  }
  auto ring_state = state.read_vector<std::byte>();
  auto reader = serialization::Reader(std::move(ring_state));
  auto exterior = Ring::unpack(reader);
  auto num_interiors = state.read<size_t>();
  std::vector<Ring> interiors;
  interiors.reserve(num_interiors);
  for (size_t i = 0; i < num_interiors; ++i) {
    ring_state = state.read_vector<std::byte>();
    reader = serialization::Reader(std::move(ring_state));
    interiors.push_back(Ring::unpack(reader));
  }
  return {std::move(exterior), std::move(interiors)};
}

}  // namespace pyinterp::geodetic

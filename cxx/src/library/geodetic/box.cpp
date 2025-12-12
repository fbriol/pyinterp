#include "pyinterp/geodetic/box.hpp"

#include <optional>

#include "pyinterp/geodetic/algorithm.hpp"

namespace pyinterp::geodetic {

auto Box::area(const std::optional<Spheroid>& wgs,
               const StrategyMethod strategy) const -> double {
  return geodetic::area<Box>(*this, wgs, strategy);
}

auto Box::area(const std::optional<Spheroid>& wgs) const -> double {
  return geodetic::area<Box, StrategyMethod::kVincenty>(*this, wgs);
}

}  // namespace pyinterp::geodetic

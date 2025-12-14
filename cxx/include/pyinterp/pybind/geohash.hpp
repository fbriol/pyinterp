// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#pragma once

#include <nanobind/nanobind.h>

namespace pyinterp {
namespace geohash::pybind {

/// @brief Initialize geohash string bindings
/// @param[in,out] m Python module
auto init_geohash_string(nanobind::module_& m) -> void;

/// @brief Initialize GeoHash class bindings
/// @param[in,out] m Python module
auto init_geohash_class(nanobind::module_& m) -> void;

}  // namespace geohash::pybind

namespace pybind {

/// @brief Initialize geohash bindings
/// @param[in,out] m Python module
void init_geohash(nanobind::module_& m);

}  // namespace pybind
}  // namespace pyinterp

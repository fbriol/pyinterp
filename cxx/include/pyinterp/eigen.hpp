// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#pragma once

#include <Eigen/Core>

namespace pyinterp {

/// @brief Dynamic vector of type T
/// @tparam T The data type of the vector elements.
template <typename T>
using Vector = Eigen::Matrix<T, Eigen::Dynamic, 1>;

/// @brief Dynamic matrix of type T
/// @tparam T The data type of the matrix elements.
template <typename T>
using Matrix = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;

/// @brief Row major dynamic matrix of type T
/// @tparam T The data type of the matrix elements.
template <typename T>
using RowMajorMatrix =
    Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

}  // namespace pyinterp

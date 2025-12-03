// Copyright (c) 2025 CNES
//
// All rights reserved. Use of this source code is governed by a
// BSD-style license that can be found in the LICENSE file.
#pragma once

#include <nanobind/nanobind.h>
#include <nanobind/ndarray.h>
#include <nanobind/stl/string.h>
#include <sys/types.h>

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <format>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <utility>

#include "pyinterp/pybind/axis.hpp"
#include "pyinterp/pybind/temporal_axis.hpp"

namespace pyinterp::pybind {
namespace detail {

/// Format a number of bytes into a human-readable string.
/// @param nbytes Number of bytes.
/// @return Formatted string.
[[nodiscard]] inline auto format_bytes(size_t nbytes) -> std::string {
  constexpr auto units =
      std::array<std::string_view, 5>{"B", "KB", "MB", "GB", "TB"};
  constexpr size_t kBytesPerKilobyte = 1024;
  for (const auto& unit : units) {
    if (nbytes < kBytesPerKilobyte) {
      return std::format("{} {}", nbytes, unit);
    }
    nbytes /= kBytesPerKilobyte;
  }
  return std::format("{} PB", nbytes);
}

/// Dtype name traits.
/// @tparam T Data type.
template <typename T>
struct dtype_name_traits;

/// @brief Specializations of dtype_name_traits for int8_t.
template <>
struct dtype_name_traits<int8_t> {
  static constexpr const char* value = "int8";
};

/// @brief Specializations of dtype_name_traits for uint8_t.
template <>
struct dtype_name_traits<uint8_t> {
  static constexpr const char* value = "uint8";
};

/// @brief Specializations of dtype_name_traits for int16_t.
template <>
struct dtype_name_traits<int16_t> {
  static constexpr const char* value = "int16";
};

/// @brief Specializations of dtype_name_traits for uint16_t.
template <>
struct dtype_name_traits<uint16_t> {
  static constexpr const char* value = "uint16";
};

/// @brief Specializations of dtype_name_traits for int32_t.
template <>
struct dtype_name_traits<int32_t> {
  static constexpr const char* value = "int32";
};

/// @brief Specializations of dtype_name_traits for uint32_t.
template <>
struct dtype_name_traits<uint32_t> {
  static constexpr const char* value = "uint32";
};

/// @brief Specializations of dtype_name_traits for int64_t.
template <>
struct dtype_name_traits<int64_t> {
  static constexpr const char* value = "int64";
};

/// @brief Specializations of dtype_name_traits for uint64_t.
template <>
struct dtype_name_traits<uint64_t> {
  static constexpr const char* value = "uint64";
};

/// @brief Specializations of dtype_name_traits for float.
template <>
struct dtype_name_traits<float> {
  static constexpr const char* value = "float32";
};

/// @brief Specializations of dtype_name_traits for double.
template <>
struct dtype_name_traits<double> {
  static constexpr const char* value = "float64";
};

/// Get the dtype name for a given data type.
/// @tparam T Data type.
/// @return Dtype name as a C string.
template <typename T>
[[nodiscard]] constexpr auto dtype_name() -> const char* {
  if constexpr (requires { dtype_name_traits<T>::value; }) {
    return dtype_name_traits<T>::value;
  } else {
    std::unreachable();
  }
}

/// Get a string representation of the shape of a NumPy array.
/// @tparam NDIMS Number of dimensions.
/// @tparam NDArray NumPy array type.
/// @param array NumPy array.
/// @return String representation of the shape.
template <size_t NDIMS, typename NDArray>
[[nodiscard]] inline auto array_shape_str(const NDArray& array) -> std::string {
  std::string shape_str = "(";
  for (size_t i = 0; i < NDIMS; ++i) {
    if (i > 0) {
      shape_str += ", ";
    }
    shape_str += std::to_string(array.shape(i));
  }
  shape_str += ")";
  return shape_str;
}

/// Trait to map math axis types to their nanobind wrappers.
template <typename MathAxisT>
struct axis_pybind_wrapper;

/// Specialization for math::Axis
/// @tparam T Value type of the axis
template <typename T>
struct axis_pybind_wrapper<math::Axis<T>> {
  using type = Axis<T>;
};

/// Specialization for math::TemporalAxis
template <>
struct axis_pybind_wrapper<math::TemporalAxis> {
  using type = TemporalAxis;
};

/// Helper alias to get the nanobind wrapper type for a math axis type.
/// @tparam MathAxisT Math axis type.
template <typename MathAxisT>
using axis_pybind_wrapper_t = typename axis_pybind_wrapper<MathAxisT>::type;

}  // namespace detail

/// Concept for a valid axis.
template <typename T>
concept AxisTypeConcept = requires(const T& ax, typename T::value_type val) {
  typename T::value_type;
  { ax.size() } -> std::convertible_to<int64_t>;
  { ax.min_value() } -> std::same_as<typename T::value_type>;
  { ax.max_value() } -> std::same_as<typename T::value_type>;
  {
    ax.find_indexes(val)
  } -> std::same_as<std::optional<std::pair<int64_t, int64_t>>>;
  { ax.is_periodic() } -> std::same_as<bool>;
  { ax.coordinate_repr(val) } -> std::convertible_to<std::string>;
};

/// Generic N-dimensional Cartesian grid.
/// @tparam DataType Type of data stored in the grid.
/// @tparam MathAxes Axis types (math::Axis<double>, math::TemporalAxis, etc.).
template <typename DataType, AxisTypeConcept... MathAxes>
  requires(sizeof...(MathAxes) >= 1 && sizeof...(MathAxes) <= 4)
class Grid {
 public:
  /// Number of dimensions.
  static constexpr size_t kNDim = sizeof...(MathAxes);

  /// Tuple of axes.
  using math_axes_tuple_t = std::tuple<MathAxes...>;

  /// Math axis type at index I.
  /// @tparam I Index of the axis.
  template <size_t I>
  using math_axis_t = std::tuple_element_t<I, math_axes_tuple_t>;

  /// Nanobind axis type at index I (for Python-facing methods).
  /// @tparam I Index of the axis.
  template <size_t I>
  using pybind_axis_t = detail::axis_pybind_wrapper_t<math_axis_t<I>>;

  /// Extract the value type of the math axis at index I.
  /// @tparam I Index of the axis.
  template <size_t I>
  using math_axis_value_t = typename math_axis_t<I>::value_type;

  /// N-dimensional array type.
  using array_t = nanobind::ndarray<nanobind::numpy, DataType,
                                    nanobind::ndim<kNDim>, nanobind::c_contig>;

  /// N-dimensional array view type.
  using view_t = nanobind::ndarray_view<DataType, kNDim, 'C'>;

  /// Constructor.
  /// @param[in] axes Axes of the grid.
  /// @param[in] array N-dimensional data array.
  explicit Grid(detail::axis_pybind_wrapper_t<MathAxes>... axes, array_t array)
      : axes_{static_cast<MathAxes>(std::move(axes))...},
        array_{std::move(array)},
        ptr_{array_.template view<DataType>()} {
    validate_construction();
  }

  /// Constructor taking math axis types.
  explicit Grid(MathAxes... axes, array_t array)
      : axes_{std::move(axes)...},
        array_{std::move(array)},
        ptr_{array_.template view<DataType>()} {
    validate_construction();
  }

  /// Default constructor.
  Grid() = default;

  /// Destructor.
  virtual ~Grid() = default;

  /// Copy/move semantics.
  Grid(const Grid&) = default;
  Grid(Grid&&) noexcept = default;
  auto operator=(const Grid&) -> Grid& = default;
  auto operator=(Grid&&) noexcept -> Grid& = default;

  /// Get axis at index I at compile time.
  /// @tparam I Index of the axis.
  /// @return Reference to the axis.
  template <size_t I>
    requires(I < kNDim)
  [[nodiscard]] constexpr auto axis() const noexcept -> const math_axis_t<I>& {
    return std::get<I>(axes_);
  }

  /// Get nanobind axis at index I at compile time.
  /// @tparam I Index of the axis.
  /// @return New nanobind axis object.
  template <size_t I>
    requires(I < kNDim)
  [[nodiscard]] constexpr auto pybind_axis() const noexcept
      -> detail::axis_pybind_wrapper_t<math_axis_t<I>> {
    return detail::axis_pybind_wrapper_t<math_axis_t<I>>(
        this->template axis<I>());
  }

  /// Get the underlying data array.
  /// @return Reference to the data array.
  [[nodiscard]] constexpr auto array() const noexcept -> const array_t& {
    return array_;
  }

  /// Get the grid value at specified indices.
  /// @tparam Index Types of the indices.
  /// @param[in] indices Indices along each axis.
  /// @return Reference to the data value.
  template <typename... Index>
    requires(sizeof...(Index) == kNDim)
  [[nodiscard]] auto value(Index&&... indices) const noexcept
      -> const DataType& {
    return ptr_(std::forward<Index>(indices)...);
  }

  /// Check if a value is within bounds for axis I.
  /// @tparam I Index of the axis.
  /// @param[in] coordinate Value to check.
  /// @return True if the value is within bounds, false otherwise.
  template <size_t I>
  [[nodiscard]] auto is_within_bounds(
      const math_axis_value_t<I>& coordinate) const -> bool {
    const auto& ax = std::get<I>(axes_);
    return coordinate >= ax.min_value() && coordinate <= ax.max_value();
  }

  /// Construct an error description for out-of-bounds access on axis I.
  /// @tparam I Index of the axis.
  /// @param[in] coordinate Value that is out of bounds.
  /// @return Error description string.
  template <size_t I>
  [[nodiscard]] auto construct_bounds_error_description(
      const math_axis_value_t<I>& coordinate) const -> std::string {
    const auto& ax = std::get<I>(axes_);
    constexpr std::array<std::string_view, 4> labels{"x", "y", "z", "u"};
    return std::format("{} is out of bounds for axis {} [{}, ..., {}]",
                       ax.coordinate_repr(coordinate), labels[I],
                       ax.coordinate_repr(ax.min_value()),
                       ax.coordinate_repr(ax.max_value()));
  }

  /// Throw an out-of-bounds error for axis I.
  /// @tparam I Index of the axis.
  /// @param[in] coordinate Value that is out of bounds.
  template <size_t I>
  [[noreturn]] auto throw_bounds_error(
      const math_axis_value_t<I>& coordinate) const -> void {
    throw std::invalid_argument(
        construct_bounds_error_description<I>(coordinate));
  }

  /// Find the indexes that surround a given coordinate along axis I.
  /// @tparam I Index of the axis.
  /// @param[in] coordinate Coordinate value.
  /// @param[in] bounds_error Whether to raise an error if out of bounds.
  /// @return Pair of surrounding indexes, or `std::nullopt` if out of bounds.
  template <size_t I>
  [[nodiscard]] auto find_indexes(const math_axis_value_t<I>& coordinate,
                                  const bool bounds_error) const
      -> std::optional<std::pair<size_t, size_t>> {
    const auto& ax = std::get<I>(axes_);
    auto indexes = ax.find_indexes(coordinate);
    if (!indexes.has_value() && bounds_error) {
      this->template throw_bounds_error<I>(coordinate);
    }
    return indexes;
  }

  /// Get the state for pickling.
  /// @return Tuple representing the state.
  [[nodiscard]] virtual auto getstate() const -> nanobind::tuple {
    return getstate_impl(std::index_sequence_for<MathAxes...>{});
  }

  /// Set the state from unpickling.
  /// @param[in] state Tuple representing the state.
  /// @return Reconstructed `Grid` object.
  [[nodiscard]] static auto setstate(const nanobind::tuple& state) -> Grid {
    if (state.size() != kNDim + 1) {
      throw std::runtime_error(
          std::format("invalid state: expected {} elements, got {}", kNDim + 1,
                      state.size()));
    }
    return setstate_impl(state, std::index_sequence_for<MathAxes...>{});
  }

  /// Convert the grid to a string representation.
  /// @return String representation of the grid showing dimensions, shape,
  /// dtype, and memory size.
  [[nodiscard]] explicit operator std::string() const {
    constexpr std::array<std::string_view, 4> dim_names{"1D", "2D", "3D", "4D"};
    std::string_view prefix = has_temporal_axis() ? "Temporal" : "";

    return std::format(
        "{}Grid{}(shape={}, dtype={}, nbytes={})", prefix, dim_names[kNDim - 1],
        detail::array_shape_str<kNDim>(array_), detail::dtype_name<DataType>(),
        detail::format_bytes(array_.nbytes()));
  }

 protected:
  math_axes_tuple_t axes_;
  array_t array_;
  view_t ptr_;

 private:
  /// Validate grid construction.
  auto validate_construction() -> void {
    validate_axes(std::index_sequence_for<MathAxes...>{});
    validate_shapes(std::index_sequence_for<MathAxes...>{});
  }

  /// Validate that circular axes are only allowed on the first axis.
  template <size_t... Is>
  auto validate_axes(std::index_sequence<Is...>) -> void {
    (
        [&] {
          if constexpr (Is > 0) {
            const auto& ax = std::get<Is>(axes_);
            if (ax.is_periodic()) {
              constexpr std::array<std::string_view, 4> labels{"x", "y", "z",
                                                               "u"};
              throw std::invalid_argument(
                  std::format("{}-axis cannot be a circle", labels[Is]));
            }
          }
        }(),
        ...);
  }

  /// Validate that axis sizes match array shapes.
  template <size_t... Is>
  auto validate_shapes(std::index_sequence<Is...>) -> void {
    (
        [&] {
          const auto& ax = std::get<Is>(axes_);
          if (ax.size() != static_cast<size_t>(array_.shape(Is))) {
            constexpr std::array<std::string_view, 4> labels{"x", "y", "z",
                                                             "u"};
            throw std::invalid_argument(std::format(
                "{} axis size ({}) doesn't match array shape[{}] ({})",
                labels[Is], ax.size(), Is, array_.shape(Is)));
          }
        }(),
        ...);
  }

  /// Check if this grid has a temporal axis.
  template <size_t... Is>
  [[nodiscard]] constexpr auto has_temporal_axis_impl(
      std::index_sequence<Is...>) const -> bool {
    return ((std::is_same_v<math_axis_t<Is>, math::TemporalAxis>) || ...);
  }

  [[nodiscard]] constexpr auto has_temporal_axis() const -> bool {
    return has_temporal_axis_impl(std::index_sequence_for<MathAxes...>{});
  }

  /// Serialize the grid state for pickling.
  template <size_t... Is>
  [[nodiscard]] auto getstate_impl(std::index_sequence<Is...>) const
      -> nanobind::tuple {
    return nanobind::make_tuple(
        pybind_axis_t<Is>(std::get<Is>(axes_)).getstate()..., array_);
  }

  /// Deserialize the grid state from unpickling.
  template <size_t... Is>
  [[nodiscard]] static auto setstate_impl(const nanobind::tuple& state,
                                          std::index_sequence<Is...>) -> Grid {
    return Grid(static_cast<math_axis_t<Is>>(pybind_axis_t<Is>::setstate(
                    nanobind::cast<nanobind::tuple>(state[Is])))...,
                nanobind::cast<array_t>(state[kNDim]));
  }
};

/// Spatial axis alias for clarity.
template <typename T = double>
using MathSpatialAxis = math::Axis<T>;

/// Temporal axis alias for clarity.
using MathTemporalAxis = math::TemporalAxis;

/// One-dimensional grid alias.
/// @tparam DataType Type of data stored in the grid.
template <typename DataType>
using Grid1D = Grid<DataType, MathSpatialAxis<>>;

/// Two-dimensional grid alias.
/// @tparam DataType Type of data stored in the grid.
template <typename DataType>
using Grid2D = Grid<DataType, MathSpatialAxis<>, MathSpatialAxis<>>;

/// Three-dimensional grid alias.
/// @tparam DataType Type of data stored in the grid.
template <typename DataType>
using Grid3D =
    Grid<DataType, MathSpatialAxis<>, MathSpatialAxis<>, MathSpatialAxis<>>;

/// Four-dimensional grid alias.
/// @tparam DataType Type of data stored in the grid.
template <typename DataType>
using Grid4D = Grid<DataType, MathSpatialAxis<>, MathSpatialAxis<>,
                    MathSpatialAxis<>, MathSpatialAxis<>>;

/// Temporal three-dimensional grid alias.
/// @tparam DataType Type of data stored in the grid.
template <typename DataType>
using TemporalGrid3D =
    Grid<DataType, MathSpatialAxis<>, MathSpatialAxis<>, MathTemporalAxis>;

/// Temporal four-dimensional grid alias.
/// @tparam DataType Type of data stored in the grid.
template <typename DataType>
using TemporalGrid4D = Grid<DataType, MathSpatialAxis<>, MathSpatialAxis<>,
                            MathTemporalAxis, MathSpatialAxis<>>;

/// Helper to bind a grid class to Python.
/// @tparam GridType The grid type to bind.
/// @param m Nanobind module.
/// @param name Python class name.
/// @param docstring Class documentation.
/// @return The `nanobind::class_` object for the bound class.
template <typename GridType>
auto bind_grid(nanobind::module_& m, std::string_view name,
               std::string_view docstring) -> nanobind::class_<GridType> {
  constexpr size_t N = GridType::kNDim;

  auto cls = nanobind::class_<GridType>(m, name.data(), docstring.data());
  // Bind common properties
  cls.def_prop_ro("array", &GridType::array, "Gets the data array")
      .def(
          "__repr__",
          [](const GridType& self) -> std::string { return std::string(self); },
          "Return the string representation of this Grid.")
      .def("__getstate__", &GridType::getstate, "Get the state for pickling.")
      .def(
          "__setstate__",
          [](GridType& self, nanobind::tuple& state) {
            return new (&self) GridType(std::move(GridType::setstate(state)));
          },
          nanobind::arg("state"), "Set the state for unpickling.");

  // Bind axis accessors based on dimension
  if constexpr (N >= 1) {
    cls.def_prop_ro(
        "x",
        [](const GridType& self) { return self.template pybind_axis<0>(); },
        "Gets the X-axis");
  }
  if constexpr (N >= 2) {
    cls.def_prop_ro(
        "y",
        [](const GridType& self) { return self.template pybind_axis<1>(); },
        "Gets the Y-axis");
  }
  if constexpr (N >= 3) {
    cls.def_prop_ro(
        "z",
        [](const GridType& self) { return self.template pybind_axis<2>(); },
        "Gets the Z-axis");
  }
  if constexpr (N >= 4) {
    cls.def_prop_ro(
        "u",
        [](const GridType& self) { return self.template pybind_axis<3>(); },
        "Gets the U-axis");
  }

  return cls;
}

/// Bind specific constructor for 1D grid
template <typename DataType>
auto bind_grid_1d(nanobind::module_& m, std::string_view name,
                  std::string_view docstring) -> void {
  using GridType = Grid1D<DataType>;

  bind_grid<GridType>(m, name, docstring)
      .def(nanobind::init<Axis<double>, typename GridType::array_t>(),
           nanobind::arg("x"), nanobind::arg("array"));
}

/// Bind specific constructor for 2D grid
template <typename DataType>
auto bind_grid_2d(nanobind::module_& m, std::string_view name,
                  std::string_view docstring) -> void {
  using GridType = Grid2D<DataType>;

  bind_grid<GridType>(m, name, docstring)
      .def(nanobind::init<Axis<double>, Axis<double>,
                          typename GridType::array_t>(),
           nanobind::arg("x"), nanobind::arg("y"), nanobind::arg("array"));
}

/// Bind specific constructor for 3D grid
template <typename DataType>
auto bind_grid_3d(nanobind::module_& m, std::string_view name,
                  std::string_view docstring) -> void {
  using GridType = Grid3D<DataType>;

  bind_grid<GridType>(m, name, docstring)
      .def(nanobind::init<Axis<double>, Axis<double>, Axis<double>,
                          typename GridType::array_t>(),
           nanobind::arg("x"), nanobind::arg("y"), nanobind::arg("z"),
           nanobind::arg("array"));
}

/// Bind specific constructor for 4D grid
template <typename DataType>
auto bind_grid_4d(nanobind::module_& m, std::string_view name,
                  std::string_view docstring) -> void {
  using GridType = Grid4D<DataType>;

  bind_grid<GridType>(m, name, docstring)
      .def(nanobind::init<Axis<double>, Axis<double>, Axis<double>,
                          Axis<double>, typename GridType::array_t>(),
           nanobind::arg("x"), nanobind::arg("y"), nanobind::arg("z"),
           nanobind::arg("u"), nanobind::arg("array"));
}

/// Bind specific constructor for 3D grid
template <typename DataType>
auto bind_temporal_grid_3d(nanobind::module_& m, std::string_view name,
                           std::string_view docstring) -> void {
  using GridType = TemporalGrid3D<DataType>;

  bind_grid<GridType>(m, name, docstring)
      .def(nanobind::init<Axis<double>, Axis<double>, TemporalAxis,
                          typename GridType::array_t>(),
           nanobind::arg("x"), nanobind::arg("y"), nanobind::arg("z"),
           nanobind::arg("array"));
}

/// Bind specific constructor for 4D grid
template <typename DataType>
auto bind_temporal_grid_4d(nanobind::module_& m, std::string_view name,
                           std::string_view docstring) -> void {
  using GridType = TemporalGrid4D<DataType>;

  bind_grid<GridType>(m, name, docstring)
      .def(nanobind::init<Axis<double>, Axis<double>, TemporalAxis,
                          Axis<double>, typename GridType::array_t>(),
           nanobind::arg("x"), nanobind::arg("y"), nanobind::arg("z"),
           nanobind::arg("u"), nanobind::arg("array"));
}

/// Bind all standard grid types for Python.
/// @tparam DataType The data type for the grids.
/// @param m Nanobind module.
/// @param suffix Suffix for class names (e.g., "Float64").
template <typename DataType>
auto bind_grids(nanobind::module_& m, std::string_view suffix) -> void {
  bind_grid_1d<DataType>(m, std::format("Grid1D{}", suffix),
                         "Cartesian Grid 1D");
  bind_grid_2d<DataType>(m, std::format("Grid2D{}", suffix),
                         "Cartesian Grid 2D");
  bind_grid_3d<DataType>(m, std::format("Grid3D{}", suffix),
                         "Cartesian Grid 3D");
  bind_grid_4d<DataType>(m, std::format("Grid4D{}", suffix),
                         "Cartesian Grid 4D");
  bind_temporal_grid_3d<DataType>(m, std::format("TemporalGrid3D{}", suffix),
                                  "Temporal Cartesian Grid 3D");
  bind_temporal_grid_4d<DataType>(m, std::format("TemporalGrid4D{}", suffix),
                                  "Temporal Cartesian Grid 4D");
}

/// Bind Grid classes to Python.
auto init_grids(nanobind::module_& m) -> void;

}  // namespace pyinterp::pybind

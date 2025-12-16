#pragma once

#include <iterator>
#include <vector>

#include "pyinterp/geometry/geographic/point.hpp"
#include "pyinterp/serialization_buffer.hpp"

namespace pyinterp::geometry::geographic {

/// @brief Ring: closed linestring (for polygon boundaries).
///
/// The `Ring` class represents a closed sequence of `Point` objects used
/// as polygon boundaries. It provides basic container-like operations and
/// iterator support.
class Ring {
 public:
  /// @brief Underlying container type for point storage.
  using container_type = std::vector<Point>;

  /// @brief Value type for the container
  using value_type = Point;

  /// @brief Iterator over points.
  using iterator = container_type::iterator;

  /// @brief Const iterator over points.
  using const_iterator = container_type::const_iterator;

  /// @brief Default constructor. Creates an empty ring.
  constexpr Ring() = default;

  /// @brief Construct a ring from a vector of points.
  /// @param[in] points Vector of points (copied/moved into the ring).
  constexpr explicit Ring(std::vector<Point> points)
      : points_{std::move(points)} {}

  /// @brief Append a point to the ring.
  /// @param[in] pt Point to append.
  constexpr void push_back(const Point& pt) { points_.push_back(pt); }

  /// @brief Remove all points from the ring.
  constexpr void clear() noexcept { points_.clear(); }

  /// @brief Resize the ring to contain `n` points.
  /// @param[in] n New size.
  constexpr void resize(std::size_t n) { points_.resize(n); }

  /// @brief Return the number of points in the ring.
  /// @returns Number of points.
  [[nodiscard]] constexpr auto size() const noexcept -> std::size_t {
    return points_.size();
  }

  /// @brief Check whether the ring is empty.
  /// @returns `true` if empty.
  [[nodiscard]] constexpr auto empty() const noexcept -> bool {
    return points_.empty();
  }

  /// @brief Access the i-th point (mutable).
  /// @param[in] ix Index of the point.
  /// @returns Reference to the point at index `ix`.
  [[nodiscard]] constexpr auto operator[](std::size_t ix) -> Point& {
    return points_[ix];
  }

  /// @brief Access the i-th point (const).
  /// @param[in] ix Index of the point.
  /// @returns Const reference to the point at index `ix`.
  [[nodiscard]] constexpr auto operator[](std::size_t ix) const
      -> const Point& {
    return points_[ix];
  }

  /// @brief Return a mutable iterator to the first point.
  [[nodiscard]] constexpr auto begin() noexcept -> iterator {
    return points_.begin();
  }

  /// @brief Return a mutable iterator to past-the-end.
  [[nodiscard]] constexpr auto end() noexcept -> iterator {
    return points_.end();
  }

  /// @brief Return a const iterator to the first point.
  [[nodiscard]] constexpr auto begin() const noexcept -> const_iterator {
    return points_.begin();
  }

  /// @brief Return a const iterator to past-the-end.
  [[nodiscard]] constexpr auto end() const noexcept -> const_iterator {
    return points_.end();
  }

  /// @brief Serialize the ring state for storage or transmission.
  /// @return Serialized state as a vector of points.
  [[nodiscard]] constexpr auto pack() const -> serialization::Writer {
    serialization::Writer writer;
    writer.write(kMagicNumber);
    writer.write(points_);
    return writer;
  }

  /// @brief Deserialize a ring from serialized state.
  /// @param[in] state Reference to serialization Reader containing encoded ring
  /// data.
  /// @return New Ring instance with restored points.
  /// @throw std::invalid_argument If the state is invalid or empty.
  [[nodiscard]] static auto unpack(serialization::Reader& state) -> Ring {
    if (state.size() == 0) {
      throw std::invalid_argument("Cannot restore ring from empty state.");
    }
    auto magic_number = state.read<uint32_t>();
    if (magic_number != kMagicNumber) {
      throw std::invalid_argument("Invalid ring state (bad magic number).");
    }
    auto points = state.read_vector<Point>();
    return Ring(std::move(points));
  }

 private:
  /// @brief Magic number for validation
  static constexpr uint32_t kMagicNumber = 0x52494E47;  // "RING"
  /// @brief Container holding the points.
  container_type points_;
};

}  // namespace pyinterp::geometry::geographic

namespace boost::geometry::traits {

template <>
struct tag<pyinterp::geometry::geographic::Ring> {
  using type = ring_tag;
};

template <>
struct point_type<pyinterp::geometry::geographic::Ring> {
  using type = pyinterp::geometry::geographic::Point;
};

}  // namespace boost::geometry::traits

// Make Ring compatible with boost::geometry range utilities
namespace std {
template <>
class back_insert_iterator<pyinterp::geometry::geographic::Ring> {
 public:
  using iterator_category = std::output_iterator_tag;
  using value_type = void;
  using difference_type = void;
  using pointer = void;
  using reference = void;
  using container_type = pyinterp::geometry::geographic::Ring;

  explicit back_insert_iterator(pyinterp::geometry::geographic::Ring& ring)
      : ring_(&ring) {}

  constexpr auto operator=(const pyinterp::geometry::geographic::Point& point)
      -> back_insert_iterator& {
    ring_->push_back(point);
    return *this;
  }

  constexpr auto operator=(pyinterp::geometry::geographic::Point&& point)
      -> back_insert_iterator& {
    ring_->push_back(point);
    return *this;
  }

  constexpr auto operator*() -> back_insert_iterator& { return *this; }
  constexpr auto operator++() -> back_insert_iterator& { return *this; }
  constexpr auto operator++(int) -> back_insert_iterator { return *this; }

 private:
  pyinterp::geometry::geographic::Ring* ring_;
};

}  // namespace std

#pragma once

#include <cstdint>
#include <vector>

#include "pyinterp/geodetic/ring.hpp"
#include "pyinterp/serialization_buffer.hpp"

namespace pyinterp::geodetic {

/// @brief Polygon: exterior ring + optional interior rings (holes).
///
/// The `Polygon` class represents a polygon defined by an exterior ring and
/// optional interior rings (holes). It provides accessors for the exterior
/// ring and the interior rings.
class Polygon {
 public:
  /// @brief Alias for ring type.
  using ring_type = Ring;

  /// @brief Alias for container of interior rings.
  using inner_container_type = std::vector<Ring>;

  /// @brief Default constructor.
  constexpr Polygon() = default;

  /// @brief Construct a polygon with an exterior ring only.
  /// @param[in] exterior Exterior ring for the polygon.
  constexpr explicit Polygon(Ring exterior) : exterior_{std::move(exterior)} {}

  /// @brief Construct a polygon with exterior and interior rings.
  /// @param[in] exterior Exterior ring for the polygon.
  /// @param[in] interiors Interior rings (holes).
  constexpr Polygon(Ring exterior, std::vector<Ring> interiors)
      : exterior_{std::move(exterior)}, interiors_{std::move(interiors)} {}

  /// @brief Get the exterior ring (const).
  /// @returns Const reference to the exterior ring.
  [[nodiscard]] constexpr auto outer() const noexcept -> const Ring& {
    return exterior_;
  }

  /// @brief Get the exterior ring (mutable).
  /// @returns Reference to the exterior ring.
  [[nodiscard]] constexpr auto outer() noexcept -> Ring& { return exterior_; }

  /// @brief Get the interior rings (const).
  /// @returns Const reference to the container of interior rings.
  [[nodiscard]] constexpr auto inners() const noexcept
      -> const inner_container_type& {
    return interiors_;
  }

  /// @brief Get the interior rings (mutable).
  /// @returns Reference to the container of interior rings.
  [[nodiscard]] constexpr auto inners() noexcept -> inner_container_type& {
    return interiors_;
  }

  /// @brief Serialize the polygon state for storage or transmission.
  /// @return Serialized state as a vector of points.
  [[nodiscard]] constexpr auto pack() const -> serialization::Writer {
    serialization::Writer writer;
    writer.write(kMagicNumber);
    writer.write(exterior_.pack());
    writer.write(interiors_.size());
    for (const auto& ring : interiors_) {
      writer.write(ring.pack());
    }
    return writer;
  }

  /// @brief Deserialize a polygon from serialized state.
  /// @param[in] state Reference to serialization Reader containing encoded
  /// polygon data.
  /// @return New Polygon instance with restored rings.
  /// @throw std::invalid_argument If the state is invalid or empty.
  [[nodiscard]] static auto unpack(serialization::Reader& state) -> Polygon;

 private:
  /// @brief Magic number for validation
  static constexpr uint32_t kMagicNumber = 0x504f4c59;  // "POLY"
  /// @brief Exterior ring.
  Ring exterior_{};
  /// @brief Interior rings (holes).
  inner_container_type interiors_{};
};

}  // namespace pyinterp::geodetic

namespace boost::geometry::traits {

template <>
struct tag<pyinterp::geodetic::Polygon> {
  using type = polygon_tag;
};

template <>
struct point_type<pyinterp::geodetic::Polygon> {
  using type = pyinterp::geodetic::Point;
};

template <>
struct ring_const_type<pyinterp::geodetic::Polygon> {
  using type = const pyinterp::geodetic::Ring&;
};

template <>
struct ring_mutable_type<pyinterp::geodetic::Polygon> {
  using type = pyinterp::geodetic::Ring&;
};

template <>
struct interior_const_type<pyinterp::geodetic::Polygon> {
  using type = const std::vector<pyinterp::geodetic::Ring>&;
};

template <>
struct interior_mutable_type<pyinterp::geodetic::Polygon> {
  using type = std::vector<pyinterp::geodetic::Ring>&;
};

template <>
struct exterior_ring<pyinterp::geodetic::Polygon> {
  static auto get(pyinterp::geodetic::Polygon& p) -> pyinterp::geodetic::Ring& {
    return p.outer();
  }
  static auto get(const pyinterp::geodetic::Polygon& p)
      -> const pyinterp::geodetic::Ring& {
    return p.outer();
  }
};

template <>
struct interior_rings<pyinterp::geodetic::Polygon> {
  static auto get(pyinterp::geodetic::Polygon& p)
      -> std::vector<pyinterp::geodetic::Ring>& {
    return p.inners();
  }
  static auto get(const pyinterp::geodetic::Polygon& p)
      -> const std::vector<pyinterp::geodetic::Ring>& {
    return p.inners();
  }
};

}  // namespace boost::geometry::traits

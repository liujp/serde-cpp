// This file is part of CAF, the C++ Actor Framework. See the file LICENSE in
// the main distribution directory for license terms and copyright or visit
// https://github.com/actor-framework/actor-framework/blob/master/LICENSE.

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "load_inspector_base.hpp"
#include "my_error.hpp"
#include "span.hpp"
#include "squashed_int.hpp"
#include "type_def.h"
#include "type_id.hpp"

/// Deserializes C++ objects from sequence of bytes. Does not perform
/// run-time type checks.
class binary_deserializer : public load_inspector_base<binary_deserializer> {
public:
  binary_deserializer() : current_(nullptr), end_(nullptr) {}
  virtual ~binary_deserializer() {}

  using super = load_inspector_base<binary_deserializer>;

  template <class Container>
  binary_deserializer(const Container &input) noexcept {
    reset(as_bytes(make_span(input)));
  }

  binary_deserializer(const void *buf, size_t size) noexcept
      : binary_deserializer(
            make_span(reinterpret_cast<const std::byte *>(buf), size)) {}

  /// Returns how many bytes are still available to read.
  size_t remaining() const noexcept {
    return static_cast<size_t>(end_ - current_);
  }

  /// Returns the remaining bytes.
  span<const std::byte> remainder() const noexcept {
    return make_span(current_, end_);
  }

  /// Jumps `num_bytes` forward.
  /// @pre `num_bytes <= remaining()`
  void skip(size_t num_bytes);

  /// Assigns a new input.
  void reset(span<const std::byte> bytes) noexcept;

  /// Returns the current read position.
  const std::byte *current() const noexcept { return current_; }

  /// Returns the end of the assigned memory block.
  const std::byte *end() const noexcept { return end_; }

  static constexpr bool has_human_readable_format() noexcept { return false; }

  bool fetch_next_object_type(type_id_t &type) noexcept;

  constexpr bool begin_object(type_id_t, std::string_view) noexcept {
    return true;
  }

  constexpr bool end_object() noexcept { return true; }

  constexpr bool begin_field(std::string_view) noexcept { return true; }

  bool begin_field(std::string_view name, bool &is_present) noexcept;

  bool begin_field(std::string_view name, span<const type_id_t> types,
                   size_t &index) noexcept;

  bool begin_field(std::string_view name, bool &is_present,
                   span<const type_id_t> types, size_t &index) noexcept;

  constexpr bool end_field() { return true; }

  constexpr bool begin_tuple(size_t) noexcept { return true; }

  constexpr bool end_tuple() noexcept { return true; }

  constexpr bool begin_key_value_pair() noexcept { return true; }

  constexpr bool end_key_value_pair() noexcept { return true; }

  bool begin_sequence(size_t &list_size) noexcept;

  constexpr bool end_sequence() noexcept { return true; }

  bool begin_associative_array(size_t &size) noexcept {
    return begin_sequence(size);
  }

  bool end_associative_array() noexcept { return end_sequence(); }

  bool value(bool &x) noexcept;

  bool value(std::byte &x) noexcept;

  bool value(uint8_t &x) noexcept;

  bool value(int8_t &x) noexcept;

  bool value(int16_t &x) noexcept;

  bool value(uint16_t &x) noexcept;

  bool value(int32_t &x) noexcept;

  bool value(uint32_t &x) noexcept;

  bool value(int64_t &x) noexcept;

  bool value(uint64_t &x) noexcept;

  template <class T>
  std::enable_if_t<std::is_integral<T>::value, bool> value(T &x) noexcept {
    auto tmp = squashed_int_t<T>{0};
    if (value(tmp)) {
      x = static_cast<T>(tmp);
      return true;
    } else {
      return false;
    }
  }

  bool value(float &x) noexcept;

  bool value(double &x) noexcept;

  bool value(long double &x);

  bool value(std::string &x);

  bool value(std::u16string &x);

  bool value(std::u32string &x);

  bool value(span<std::byte> x) noexcept;

  bool value(std::vector<bool> &x);

private:
  bool range_check(size_t read_size) const noexcept {
    return current_ + read_size <= end_;
  }
  const std::byte *current_;
  const std::byte *end_;
};

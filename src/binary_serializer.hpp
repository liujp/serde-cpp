#pragma once

#include <cstddef>
#include <string>
#include <type_traits>
#include <vector>

#include "save_inspector_base.hpp"
#include "span.hpp"
#include "squashed_int.hpp"
#include "type_def.h"
#include "type_id.hpp"

using byte_buffer = std::vector<std::byte>;

class binary_serializer : public save_inspector_base<binary_serializer> {
public:
  // using super = save_inspector_base<binary_serializer>;
  using container_type = byte_buffer;
  using value_type = std::byte;
  binary_serializer(byte_buffer &buf) noexcept
      : buf_(buf), write_pos_(buf.size()) {}
  virtual ~binary_serializer() {}
  DISABLE_COPY(binary_serializer)
  DISABLE_MOVE(binary_serializer)
  byte_buffer &buf() noexcept { return buf_; }
  const byte_buffer &buf() const noexcept { return buf_; }
  size_t write_pos() const noexcept { return write_pos_; }
  static constexpr bool has_human_readable_format() noexcept { return false; }
  void seek(size_t offset) noexcept { write_pos_ = offset; }
  void skip(size_t num_bytes);
  constexpr bool begin_object(type_id_t, std::string_view) noexcept {return true; }
  constexpr bool end_object() { return true; }
  constexpr bool begin_field(std::string_view) noexcept { return true; }
  bool begin_field(std::string_view, bool is_present);
  bool begin_field(std::string_view, span<const type_id_t> types, size_t index);
  bool begin_field(std::string_view, bool is_present,
                   span<const type_id_t> types, size_t index);
  constexpr bool end_field() { return true; }
  constexpr bool begin_tuple(size_t) { return true; }
  constexpr bool end_tuple() { return true; }
  constexpr bool begin_key_value_pair() { return true; }
  constexpr bool end_key_value_pair() { return true; }
  bool begin_sequence(size_t list_size);
  constexpr bool end_sequence() { return true; }
  bool begin_associative_array(size_t size) { return begin_sequence(size); }
  bool end_associative_array() { return end_sequence(); }
  bool value(std::byte x);
  bool value(bool x);
  bool value(int8_t x);
  bool value(uint8_t x);
  bool value(int16_t x);
  bool value(uint16_t x);
  bool value(int32_t x);
  bool value(uint32_t x);
  bool value(int64_t x);
  bool value(uint64_t x);
  template <class T>
  std::enable_if_t<std::is_integral<T>::value, bool> value(T x) {
    return value(static_cast<squashed_int_t<T>>(x));
  }
  bool value(float x);
  bool value(double x);
  bool value(long double x);
  bool value(std::string_view x);
  bool value(const std::u16string &x);
  bool value(const std::u32string &x);
  bool value(span<const std::byte> x);
  bool value(const std::vector<bool> &x);

private:
  byte_buffer &buf_;
  size_t write_pos_;
};

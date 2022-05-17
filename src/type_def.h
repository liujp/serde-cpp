#pragma once
#include <set>

#define DISABLE_COPY(class_name)                                               \
  class_name(const class_name &) = delete;                                     \
  class_name &operator=(const class_name &) = delete;

#define DISABLE_MOVE(class_name)                                               \
  class_name(class_name &&) = delete;                                          \
  class_name &operator=(class_name &&) = delete;

/// Internal representation of a type ID.
using type_id_t = uint16_t;
using l_double = long double;
using stl_sting = std::string;
using stl_u16string = std::u16string;
using stl_u32string = std::u32string;
using stl_set_string = std::set<std::string>;
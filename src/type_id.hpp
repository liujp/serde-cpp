// This file is part of CAF, the C++ Actor Framework. See the file LICENSE in
// the main distribution directory for license terms and copyright or visit
// https://github.com/actor-framework/actor-framework/blob/master/LICENSE.

#pragma once

#include <cstdint>
#include <set>
#include <string>
#include <string_view>
#include <utility>

#include "def_traits.hpp"
#include "squashed_int.hpp"
#include "type_def.h"

/// Special value equal to the greatest possible value for `type_id_t`.
/// Generally indicates that no type ID for a given type exists.
constexpr type_id_t invalid_type_id = 65535;

/// Maps the type `T` to a globally unique ID.
template <class T> struct type_id;

/// Convenience alias for `type_id<T>::value`.
/// especially for u/int8/16/32/64
/// @relates type_id
template <class T>
constexpr type_id_t type_id_v = type_id<squash_if_int_t<T>>::value;

/// Maps the globally unique ID `V` to a type (inverse to ::type_id).
/// @relates type_id
template <type_id_t V> struct type_by_id;

/// Convenience alias for `type_by_id<I>::type`.
/// @relates type_by_id
template <type_id_t I> using type_by_id_t = typename type_by_id<I>::type;

/// Maps the globally unique ID `V` to a type name.
template <type_id_t V> struct type_name_by_id;

/// Convenience alias for `type_name_by_id<I>::value`.
/// @relates type_name_by_id
template <type_id_t I>
constexpr std::string_view type_name_by_id_v = type_name_by_id<I>::value;

/// Convenience type that resolves to `type_name_by_id<type_id_v<T>>`.
template <class T> struct type_name;

/// Convenience specialization that enables generic code to not treat `void`
/// manually.
template <> struct type_name<void> {
  static constexpr std::string_view value = "void";
};

/// Convenience alias for `type_name<T>::value`.
/// @relates type_name
template <class T> constexpr std::string_view type_name_v = type_name<T>::value;

/// The first type ID not reserved by CAF and its modules.
constexpr type_id_t first_custom_type_id = 200;

/// Checks whether `type_id` is specialized for `T`.
template <class T> constexpr bool has_type_id_v = is_complete<type_id<T>>;

/// Returns `type_name_v<T>` if available, "anonymous" otherwise.
template <class T> std::string_view type_name_or_anonymous() {
  if constexpr (is_complete<type_name<T>>)
    return type_name<T>::value;
  else
    return "anonymous";
}

/// Returns `type_id_v<T>` if available, `invalid_type_id` otherwise.
template <class T> type_id_t type_id_or_invalid() {
  if constexpr (is_complete<type_id<T>>)
    return type_id<T>::value;
  else
    return invalid_type_id;
}

// type udf types define
#define UName(name) udf_##name,
#define EName(name) udf_##name
#define PName(name) "udf_" #name

#define TypeId(name)                                                           \
  template <> struct type_id<name> {                                           \
    static constexpr type_id_t value = UType::EName(name);                     \
  };

#define TypeName(name)                                                         \
  template <> struct type_name<name> {                                         \
    static constexpr std::string_view value = PName(name);                     \
  };

#define InitT(_)                                                               \
  enum UType : uint16_t { T_START = 0, _(UName) T_END };                       \
  _(TypeId);                                                                   \
  _(TypeName);

#define Lists(_)                                                               \
  _(bool)                                                                      \
  _(double)                                                                    \
  _(float)                                                                     \
  _(int16_t)                                                                   \
  _(int32_t)                                                                   \
  _(int64_t)                                                                   \
  _(int8_t)                                                                    \
  _(l_double)                                                                  \
  _(uint16_t)                                                                  \
  _(uint32_t)                                                                  \
  _(uint64_t)                                                                  \
  _(uint8_t)                                                                   \
  _(stl_sting)                                                                 \
  _(stl_u16string)                                                             \
  _(stl_u32string)                                                             \
  _(stl_set_string)

InitT(Lists);
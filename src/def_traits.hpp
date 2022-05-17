#pragma once
#include <stddef.h>
#include <string>
#include <type_traits>

#include "span.hpp"

template <class> constexpr bool assertion_failed_v = false;

template <class T, std::size_t = sizeof(T)>
std::true_type is_complete_impl(T *);

std::false_type is_complete_impl(...);

template <class... Ts> struct type_list {
  constexpr type_list() {
    // nop
  }
};

template <class T> T &as_mutable_ref(T &x) { return x; }

template <class T> T &as_mutable_ref(const T &x) { return const_cast<T &>(x); }

/// Checks whether T is a complete type. For example, passing a forward
/// declaration or undefined template specialization evaluates to `false`.
template <class T>
constexpr bool is_complete =
    decltype(is_complete_impl(std::declval<T *>()))::value;

template <class Trait, class T = void>
using enable_if_tt = typename std::enable_if<Trait::value, T>::type;

template <class T> using decay_t = typename std::decay<T>::type;

template <class T> struct is_primitive {
  static constexpr bool value = std::is_convertible<T, std::string>::value ||
                                std::is_convertible<T, std::u16string>::value ||
                                std::is_convertible<T, std::u32string>::value ||
                                std::is_arithmetic<T>::value;
};
template <class T> constexpr T *null_v = nullptr;

// use unsafe message type
template <class T> struct allowed_unsafe_message_type : std::false_type {};

template <class T>
struct is_allowed_unsafe_message_type : allowed_unsafe_message_type<T> {};

template <class T>
struct is_allowed_unsafe_message_type<T &> : allowed_unsafe_message_type<T> {};

template <class T>
struct is_allowed_unsafe_message_type<T &&> : allowed_unsafe_message_type<T> {};

template <class T>
struct is_allowed_unsafe_message_type<const T &>
    : allowed_unsafe_message_type<T> {};

template <class T>
constexpr bool is_allowed_unsafe_message_type_v =
    allowed_unsafe_message_type<T>::value;

#define CAF_ALLOW_UNSAFE_MESSAGE_TYPE(type_name)                               \
  template <> struct allowed_unsafe_message_type<type_name> : std::true_type {};

// judge if a member function(member) exist in class
#define CAF_HAS_MEMBER_TRAIT(name)                                             \
  template <class T> class has_##name##_member {                               \
  private:                                                                     \
    template <class U>                                                         \
    static auto sfinae(U *x) -> decltype(x->name(), std::true_type());         \
                                                                               \
    template <class U> static auto sfinae(...) -> std::false_type;             \
                                                                               \
    using sfinae_type = decltype(sfinae<T>(nullptr));                          \
                                                                               \
  public:                                                                      \
    static constexpr bool value = sfinae_type::value;                          \
  }

#define CAF_HAS_ALIAS_TRAIT(name)                                              \
  template <class T> class has_##name##_alias {                                \
  private:                                                                     \
    template <class C> static std::true_type sfinae(typename C::name *);       \
                                                                               \
    template <class> static std::false_type sfinae(...);                       \
                                                                               \
    using sfinae_type = decltype(sfinae<T>(nullptr));                          \
                                                                               \
  public:                                                                      \
    static constexpr bool value = sfinae_type::value;                          \
  }

/// Checks whether `T` is primitive, i.e., either an arithmetic type or
/// cv-qualified bool, char, char8_t (since C++20), char16_t, char32_t, wchar_t, 
/// short, int, long, long long,float, double, long double; 
template <class T, bool IsLoading> struct is_builtin_inspector_type {
  static constexpr bool value = std::is_arithmetic<T>::value;
};

template <bool IsLoading>
struct is_builtin_inspector_type<std::byte, IsLoading> {
  static constexpr bool value = true;
};

template <bool IsLoading>
struct is_builtin_inspector_type<span<std::byte>, IsLoading> {
  static constexpr bool value = true;
};

template <bool IsLoading>
struct is_builtin_inspector_type<std::string, IsLoading> {
  static constexpr bool value = true;
};

template <bool IsLoading>
struct is_builtin_inspector_type<std::u16string, IsLoading> {
  static constexpr bool value = true;
};

template <bool IsLoading>
struct is_builtin_inspector_type<std::u32string, IsLoading> {
  static constexpr bool value = true;
};

template <> struct is_builtin_inspector_type<std::string_view, false> {
  static constexpr bool value = true;
};

template <> struct is_builtin_inspector_type<span<const std::byte>, false> {
  static constexpr bool value = true;
};

/// Checks whether the inspector has a `builtin_inspect` overload for `T`.
template <class Inspector, class T> class has_builtin_inspect {
private:
  template <class I, class U>
  static auto sfinae(I &f, U &x)
      -> decltype(f.builtin_inspect(x), std::true_type{});

  template <class I> static std::false_type sfinae(I &, ...);

  using sfinae_result =
      decltype(sfinae(std::declval<Inspector &>(), std::declval<T &>()));

public:
  static constexpr bool value = sfinae_result::value;
};

/// Checks whether `T` provides an `inspect` overload for `Inspector`.
template <class Inspector, class T> class has_inspect_overload {
private:
  template <class U>
  static auto sfinae(Inspector &x, U &y)
      -> decltype(inspect(x, y), std::true_type{});

  static std::false_type sfinae(Inspector &, ...);

  using result_type =
      decltype(sfinae(std::declval<Inspector &>(), std::declval<T &>()));

public:
  static constexpr bool value = result_type::value;
};

/// Evaluates to `true` for all types that specialize `std::tuple_size`, i.e.,
/// `std::tuple`, `std::pair`, and `std::array`.
template <class T> struct is_stl_tuple_type {
  template <class U>
  static auto sfinae(U *) -> decltype(
      std::integral_constant<bool, std::tuple_size<U>::value >= 0>{});

  template <class U> static auto sfinae(...) -> std::false_type;

  using type = decltype(sfinae<T>(nullptr));

  static constexpr bool value = type::value;
};

template <class T>
constexpr bool is_stl_tuple_type_v = is_stl_tuple_type<T>::value;

template <class T> class is_forward_iterator {
  template <class C>
  static bool sfinae(C &x, C &y,
                     // check for operator*
                     decay_t<decltype(*x)> * = nullptr,
                     // check for operator++ returning an iterator
                     decay_t<decltype(x = ++y)> * = nullptr,
                     // check for operator==
                     decay_t<decltype(x == y)> * = nullptr,
                     // check for operator!=
                     decay_t<decltype(x != y)> * = nullptr);

  static void sfinae(...);

  using result_type =
      decltype(sfinae(std::declval<T &>(), std::declval<T &>()));

public:
  static constexpr bool value = std::is_same<bool, result_type>::value;
};

template <class T> class is_iterable {
  // this horrible code would just disappear if we had concepts
  template <class C>
  static bool
  sfinae(C *cc,
         // check if 'C::begin()' returns a forward iterator
         enable_if_tt<is_forward_iterator<decltype(cc->begin())>> * = nullptr,
         // check if begin() and end() both exist and are comparable
         decltype(cc->begin() != cc->end()) * = nullptr);

  // SFNINAE default
  static void sfinae(void *);

  using result_type = decltype(sfinae(static_cast<decay_t<T> *>(nullptr)));

public:
  static constexpr bool value =
      is_primitive<T>::value == false && std::is_same<bool, result_type>::value;
};

CAF_HAS_ALIAS_TRAIT(key_type);

CAF_HAS_ALIAS_TRAIT(mapped_type);

/// Checks whether T behaves like `std::map`.
template <class T> struct is_map_like {
  static constexpr bool value = is_iterable<T>::value &&
                                has_key_type_alias<T>::value &&
                                has_mapped_type_alias<T>::value;
};

template <class T> constexpr bool is_map_like_v = is_map_like<T>::value;

// list like type

CAF_HAS_ALIAS_TRAIT(value_type);

template <class T> struct has_insert {
private:
  template <class List>
  static auto sfinae(List *l, typename List::value_type *x = nullptr)
      -> decltype(l->insert(l->end(), *x), std::true_type());

  template <class U> static auto sfinae(...) -> std::false_type;

  using sfinae_type = decltype(sfinae<T>(nullptr));

public:
  static constexpr bool value = sfinae_type::value;
};

template <class T> struct has_size {
private:
  template <class List>
  static auto sfinae(List *l) -> decltype(l->size(), std::true_type());

  template <class U> static auto sfinae(...) -> std::false_type;

  using sfinae_type = decltype(sfinae<T>(nullptr));

public:
  static constexpr bool value = sfinae_type::value;
};

/// Checks whether T behaves like `std::vector`, `std::list`, or `std::set`.
template <class T> struct is_list_like {
  static constexpr bool value = is_iterable<T>::value &&
                                has_value_type_alias<T>::value &&
                                !has_mapped_type_alias<T>::value &&
                                has_insert<T>::value && has_size<T>::value;
};

template <class T> constexpr bool is_list_like_v = is_list_like<T>::value;

/// Checks whether the inspector has an `opaque_value` overload for `T`.
template <class Inspector, class T> class accepts_opaque_value {
private:
  template <class F, class U>
  static auto sfinae(F *f, U *x)
      -> decltype(f->opaque_value(*x), std::true_type{});

  static std::false_type sfinae(...);

  using sfinae_result = decltype(sfinae(null_v<Inspector>, null_v<T>));

public:
  static constexpr bool value = sfinae_result::value;
};
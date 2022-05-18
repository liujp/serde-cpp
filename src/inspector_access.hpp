// This file is part of CAF, the C++ Actor Framework. See the file LICENSE in
// the main distribution directory for license terms and copyright or visit
// https://github.com/actor-framework/actor-framework/blob/master/LICENSE.

#pragma once

#include <chrono>
#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
#include <cassert>

#include "inspector_access_type.hpp"
#include "my_error.hpp"
#include "span.hpp"
#include "type_def.h"
#include "inspector_access_base.hpp"
#include "type_id.hpp"

template <class> constexpr bool assertion_failed_v = false;

// Converts a setter that returns void, error or bool to a sync function object
// taking no arguments that always returns a bool.
// inspector has a function called set error, and set a is function
template <class Inspector, class Set, class ValueType>
auto bind_setter(Inspector &f, Set &set, ValueType &tmp) {
  using set_result_type = decltype(set(std::move(tmp)));
  if constexpr (std::is_same<set_result_type, bool>::value) {
    return [&] { return set(std::move(tmp)); };
  } else if constexpr (std::is_same<set_result_type, error>::value) {
    return [&] {
      if (auto err = set(std::move(tmp)); !err) {
        return true;
      } else {
        f.set_error(std::move(err));
        return false;
      }
    };
  } else {
    static_assert(std::is_same<set_result_type, void>::value,
                  "a setter must return caf::error, bool or void");
    return [&] {
      set(std::move(tmp));
      return true;
    };
  }
}

template <class Inspector, class T>
bool load(Inspector &f, T &x, inspector_access_type::specialization) {
  return inspector_access<T>::apply(f, x);
}


template <class Inspector, class T>
bool load(Inspector &f, T &x, inspector_access_type::inspect) {
  // user should to complete this function
  return inspect(f, x);
}

template <class Inspector, class T>
bool load(Inspector &f, T &x, inspector_access_type::builtin) {
  return f.value(x);
}

template <class Inspector, class T>
bool load(Inspector &f, T &x, inspector_access_type::builtin_inspect) {
  return f.builtin_inspect(x);
}

template <class Inspector, class T>
bool load(Inspector &f, T &x, inspector_access_type::empty) {
  return f.object(x).fields();
}

template <class Inspector, class T>
bool load(Inspector &f, T &, inspector_access_type::unsafe) {
  f.emplace_error(error_code::unsafe_type);
  return false;
}

template <class Inspector, class T, size_t N>
bool load(Inspector &f, T (&xs)[N], inspector_access_type::tuple) {
  return f.tuple(xs);
}

template <class Inspector, class T>
bool load(Inspector &f, T &xs, inspector_access_type::tuple) {
  return f.tuple(xs);
}

template <class Inspector, class T>
bool load(Inspector &f, T &x, inspector_access_type::map) {
  return f.map(x);
}

template <class Inspector, class T>
bool load(Inspector &f, T &x, inspector_access_type::list) {
  return f.list(x);
}

template <class Inspector, class T>
std::enable_if_t<accepts_opaque_value<Inspector, T>::value, bool>
load(Inspector &f, T &x, inspector_access_type::none) {
  return f.opaque_value(x);
}

template <class Inspector, class T>
std::enable_if_t<!accepts_opaque_value<Inspector, T>::value, bool>
load(Inspector &, T &, inspector_access_type::none) {
  static_assert(assertion_failed_v<T>, "please provide an inspect "
                                       "overload for T or specialize "
                                       "inspector_access");
  return false;
}

template <class Inspector, class T> bool load(Inspector &f, T &x) {
  return load(f, x, inspect_access_type<Inspector, T>());
}

template <class Inspector, class T, class IsValid, class SyncValue>
bool load_field(Inspector &f, std::string_view field_name, T &x,
                IsValid &is_valid, SyncValue &sync_value) {
  using impl = std::conditional_t<is_complete<inspector_access<T>>, // if
                                  inspector_access<T>,              // then
                                  inspector_access_base<T>>;        // else
  return impl::load_field(f, field_name, x, is_valid, sync_value);
}

template <class Inspector, class T, class IsValid, class SyncValue,
          class SetFallback>
bool load_field(Inspector &f, std::string_view field_name, T &x,
                IsValid &is_valid, SyncValue &sync_value,
                SetFallback &set_fallback) {
  using impl = std::conditional_t<is_complete<inspector_access<T>>, // if
                                  inspector_access<T>,              // then
                                  inspector_access_base<T>>;        // else
  return impl::load_field(f, field_name, x, is_valid, sync_value, set_fallback);
}

// -- saving -------------------------------------------------------------------
template <class Inspector, class T>
bool save(Inspector &f, T &x, inspector_access_type::specialization) {
  return inspector_access<T>::apply(f, x);
}

template <class Inspector, class T>
bool save(Inspector &f, T &x, inspector_access_type::inspect) {
  return inspect(f, x);
}

// built types, use binary_serialize value function to serialize the value
template <class Inspector, class T>
bool save(Inspector &f, T &x, inspector_access_type::builtin) {
  return f.value(x);
}

// if inspect has a builtin_inspect function
template <class Inspector, class T>
bool save(Inspector &f, T &x, inspector_access_type::builtin_inspect) {
  return f.builtin_inspect(x);
}

template <class Inspector, class T>
bool save(Inspector &f, T &x, inspector_access_type::empty) {
  return f.object(x).fields();
}

template <class Inspector, class T>
bool save(Inspector &f, T &, inspector_access_type::unsafe) {
  f.emplace_error(unsafe_type);
  return false;
}

template <class Inspector, class T, size_t N>
bool save(Inspector &f, T (&xs)[N], inspector_access_type::tuple) {
  return f.tuple(xs);
}

template <class Inspector, class T>
bool save(Inspector &f, const T &xs, inspector_access_type::tuple) {
  return f.tuple(xs);
}

template <class Inspector, class T>
bool save(Inspector &f, T &x, inspector_access_type::map) {
  return f.map(x);
}

template <class Inspector, class T>
bool save(Inspector &f, T &x, inspector_access_type::list) {
  return f.list(x);
}

template <class Inspector, class T>
std::enable_if_t<accepts_opaque_value<Inspector, T>::value, bool>
save(Inspector &f, T &x, inspector_access_type::none) {
  return f.opaque_value(x);
}

template <class Inspector, class T>
std::enable_if_t<!accepts_opaque_value<Inspector, T>::value, bool>
save(Inspector &, T &, inspector_access_type::none) {
  static_assert(assertion_failed_v<T>, "please provide an inspect "
                                       "overload for T or specialize "
                                       "inspector_access");
  return false;
}

template <class Inspector, class T> bool save(Inspector &f, T &x) {
  return save(f, x, inspect_access_type<Inspector, T>());
}

template <class Inspector, class T> bool save(Inspector &f, const T &x) {
  if constexpr (!std::is_function<T>::value) {
    return save(f, as_mutable_ref(x), inspect_access_type<Inspector, T>());
  } else {
    // Only inspector such as the string specification_inspector are going to accept
    // function pointers. Most other inspectors are going to trigger a static
    // assertion when passing `inspector_access_type::none`.
    auto fptr = std::add_pointer_t<T>{x};
    return save(f, fptr, inspector_access_type::none{});
  }
}

template <class Inspector, class T>
bool save_field(Inspector &f, std::string_view field_name, T &x) {
  using impl = std::conditional_t<is_complete<inspector_access<T>>, // if
                                  inspector_access<T>,              // then
                                  inspector_access_base<T>>;        // else
  return impl::save_field(f, field_name, x);
}

template <class Inspector, class IsPresent, class Get>
bool save_field(Inspector &f, std::string_view field_name,
                IsPresent &is_present, Get &get) {
  using T = std::decay_t<decltype(get())>;
  using impl = std::conditional_t<is_complete<inspector_access<T>>, // if
                                  inspector_access<T>,              // then
                                  inspector_access_base<T>>;        // else
  return impl::save_field(f, field_name, is_present, get);
}

// -- customization points -----------------------------------------------------

/// Customization point for adding support for a custom type.
template <class T> struct inspector_access;


// -- inspection support for optional values -----------------------------------

struct optional_inspector_traits_base {
  template <class T> static auto &deref_load(T &x) { return *x; }

  template <class T> static auto &deref_save(T &x) { return *x; }
};

template <class T> struct optional_inspector_traits;

/// Provides inspector access for types that represent optional values.
template <class T> struct optional_inspector_access {
  using traits = optional_inspector_traits<T>;

  using container_type = typename traits::container_type;

  using value_type = typename traits::value_type;

  template <class Inspector>
  [[nodiscard]] static bool apply(Inspector &f, container_type &x) {
    return f.object(x).fields(f.field("value", x));
  }

  template <class Inspector>
  static bool save_field(Inspector &f, std::string_view field_name,
                         container_type &x) {
    auto is_present = [&x] { return static_cast<bool>(x); };
    auto get = [&x]() -> decltype(auto) { return traits::deref_save(x); };
    return save_field(f, field_name, is_present, get);
  }

  template <class Inspector, class IsPresent, class Get>
  static bool save_field(Inspector &f, std::string_view field_name,
                         IsPresent &is_present, Get &get) {
    return save_field(f, field_name, is_present, get);
  }

  template <class Inspector, class IsValid, class SyncValue>
  static bool load_field(Inspector &f, std::string_view field_name,
                         container_type &x, IsValid &is_valid,
                         SyncValue &sync_value) {
    traits::emplace(x);
    auto reset = [&x] { x.reset(); };
    return load_field(f, field_name, traits::deref_load(x), is_valid,
                      sync_value, reset);
  }

  template <class Inspector, class IsValid, class SyncValue, class SetFallback>
  static bool load_field(Inspector &f, std::string_view field_name,
                         container_type &x, IsValid &is_valid,
                         SyncValue &sync_value, SetFallback &set_fallback) {
    traits::emplace(x);
    return load_field(f, field_name, traits::deref_load(x), is_valid,
                      sync_value, set_fallback);
  }
};

// -- inspection support for optional<T> ---------------------------------------
template <class T>
struct inspector_access<std::optional<T>>
    : optional_inspector_access<std::optional<T>> {};


template <class T>
struct optional_inspector_traits<std::optional<T>>
    : optional_inspector_traits_base {
  using container_type = std::optional<T>;

  using value_type = T;

  template <class... Ts>
  static void emplace(container_type &container, Ts &&... xs) {
    container.emplace(std::forward<Ts>(xs)...);
  }
};

// -- inspection support for std::byte -----------------------------------------

template <>
struct inspector_access<std::byte> : inspector_access_base<std::byte> {
  template <class Inspector>
  [[nodiscard]] static bool apply(Inspector &f, std::byte &x) {
    using integer_type = std::underlying_type_t<std::byte>;
    auto get = [&x] { return static_cast<integer_type>(x); };
    auto set = [&x](integer_type val) { x = static_cast<std::byte>(val); };
    return f.apply(get, set);
  }
};

// -- inspection support for variant<Ts...> ------------------------------------

template <class T> struct variant_inspector_traits;

template <class T> struct variant_inspector_access {
  using value_type = T;

  using traits = variant_inspector_traits<T>;

  template <class Inspector>
  [[nodiscard]] static bool apply(Inspector &f, value_type &x) {
    return f.object(x).fields(f.field("value", x));
  }

  template <class Inspector>
  static bool save_field(Inspector &f, std::string_view field_name,
                         value_type &x) {
    auto g = [&f](auto &y) { return save(f, y); };
    return f.begin_field(field_name, make_span(traits::allowed_types),
                         traits::type_index(x)) //
           && traits::visit(g, x)               //
           && f.end_field();
  }

  template <class Inspector, class IsPresent, class Get>
  static bool save_field(Inspector &f, std::string_view field_name,
                         IsPresent &is_present, Get &get) {
    auto allowed_types = make_span(traits::allowed_types);
    if (is_present()) {
      auto &&x = get();
      auto g = [&f](auto &y) { return save(f, y); };
      return f.begin_field(field_name, true, allowed_types,
                           traits::type_index(x)) //
             && traits::visit(g, x)               //
             && f.end_field();
    }
    return f.begin_field(field_name, false, allowed_types, 0) //
           && f.end_field();
  }

  template <class Inspector>
  static bool load_variant_value(Inspector &f, std::string_view field_name,
                                 value_type &x, type_id_t runtime_type) {
    auto res = false;
    auto type_found = traits::load(runtime_type, [&](auto &tmp) {
      if (!load(f, tmp))
        return;
      traits::assign(x, std::move(tmp));
      res = true;
      return;
    });
    if (!type_found)
      f.emplace_error(error_code::invalid_field_type, std::string{field_name});
    return res;
  }

  template <class Inspector, class IsValid, class SyncValue>
  static bool load_field(Inspector &f, std::string_view field_name,
                         value_type &x, IsValid &is_valid,
                         SyncValue &sync_value) {
    size_t type_index = std::numeric_limits<size_t>::max();
    auto allowed_types = make_span(traits::allowed_types);
    if (!f.begin_field(field_name, allowed_types, type_index))
      return false;
    if (type_index >= allowed_types.size()) {
      f.emplace_error(error_code::invalid_field_type, std::string{field_name});
      return false;
    }
    auto runtime_type = allowed_types[type_index];
    if (!load_variant_value(f, field_name, x, runtime_type))
      return false;
    if (!is_valid(x)) {
      f.emplace_error(error_code::field_invariant_check_failed,
                      std::string{field_name});
      return false;
    }
    if (!sync_value()) {
      if (!f.get_error())
        f.emplace_error(error_code::field_value_synchronization_failed,
                        std::string{field_name});
      return false;
    }
    return f.end_field();
  }

  template <class Inspector, class IsValid, class SyncValue, class SetFallback>
  static bool load_field(Inspector &f, std::string_view field_name,
                         value_type &x, IsValid &is_valid,
                         SyncValue &sync_value, SetFallback &set_fallback) {
    bool is_present = false;
    auto allowed_types = make_span(traits::allowed_types);
    size_t type_index = std::numeric_limits<size_t>::max();
    if (!f.begin_field(field_name, is_present, allowed_types, type_index))
      return false;
    if (is_present) {
      if (type_index >= allowed_types.size()) {
        f.emplace_error(error_code::invalid_field_type,
                        std::string{field_name});
        return false;
      }
      auto runtime_type = allowed_types[type_index];
      if (!load_variant_value(f, field_name, x, runtime_type))
        return false;
      if (!is_valid(x)) {
        f.emplace_error(error_code::field_invariant_check_failed,
                        std::string{field_name});
        return false;
      }
      if (!sync_value()) {
        if (!f.get_error())
          f.emplace_error(error_code::field_value_synchronization_failed,
                          std::string{field_name});
        return false;
      }
    } else {
      set_fallback();
    }
    return f.end_field();
  }
};

template <class... Ts> struct variant_inspector_traits<std::variant<Ts...>> {
  static_assert(
      (has_type_id_v<Ts> && ...),
      "inspectors requires that each type in a variant has a type_id");

  using value_type = std::variant<Ts...>;

  static constexpr type_id_t allowed_types[] = {type_id_v<Ts>...};

  static auto type_index(const value_type &x) { return x.index(); }

  template <class F, class Value> static auto visit(F &&f, Value &&x) {
    return std::visit(std::forward<F>(f), std::forward<Value>(x));
  }

  template <class U> static auto assign(value_type &x, U &&value) {
    x = std::forward<U>(value);
  }

  template <class F> static bool load(type_id_t, F &, type_list<>) {
    return false;
  }

  template <class F, class U, class... Us>
  static bool load(type_id_t type, F &continuation, type_list<U, Us...>) {
    if (type_id_v<U> == type) {
      auto tmp = U{};
      continuation(tmp);
      return true;
    }
    return load(type, continuation, type_list<Us...>{});
  }

  template <class F> static bool load(type_id_t type, F continuation) {
    return load(type, continuation, type_list<Ts...>{});
  }
};

template <class... Ts>
struct inspector_access<std::variant<Ts...>>
    : variant_inspector_access<std::variant<Ts...>> {
  // nop
};

// -- inspection support for std::chrono types ---------------------------------

template <class Rep, class Period>
struct inspector_access<std::chrono::duration<Rep, Period>>
    : inspector_access_base<std::chrono::duration<Rep, Period>> {
  using value_type = std::chrono::duration<Rep, Period>;

  template <class Inspector> static bool apply(Inspector &f, value_type &x) {
    if (f.has_human_readable_format()) {
      auto get = [&x] {
        std::string str;
        print(str, x);
        return str;
      };
      auto set = [&x](std::string str) {
        return false;
      };
      return f.apply(get, set);
    } else {
      auto get = [&x] { return x.count(); };
      auto set = [&x](Rep value) {
        x = std::chrono::duration<Rep, Period>{value};
        return true;
      };
      return f.apply(get, set);
    }
  }
};

template <class Duration>
struct inspector_access<
    std::chrono::time_point<std::chrono::system_clock, Duration>>
    : inspector_access_base<
          std::chrono::time_point<std::chrono::system_clock, Duration>> {
  using value_type =
      std::chrono::time_point<std::chrono::system_clock, Duration>;

  template <class Inspector> static bool apply(Inspector &f, value_type &x) {
    if (f.has_human_readable_format()) {
      auto get = [&x] {
        std::string str;
        print(str, x);
        return str;
      };
      auto set = [&x](std::string str) { return true; };
      return f.apply(get, set);
    } else {
      using rep_type = typename Duration::rep;
      auto get = [&x] { return x.time_since_epoch().count(); };
      auto set = [&x](rep_type value) {
        x = value_type{Duration{value}};
        return true;
      };
      return f.apply(get, set);
    }
  }
};


// print functions

template <class Buffer>
void print(Buffer& buf, bool x) {
  using namespace std::literals;
  auto str = x ? "true"sv : "false"sv;
  buf.insert(buf.end(), str.begin(), str.end());
}

template <class Buffer, class T>
std::enable_if_t<std::is_integral<T>::value> print(Buffer& buf, T x) {
  // An integer can at most have 20 digits (UINT64_MAX).
  char stack_buffer[24];
  char* p = stack_buffer;
  // Convert negative values into positives as necessary.
  if constexpr (std::is_signed<T>::value) {
    if (x == std::numeric_limits<T>::min()) {
      using namespace std::literals;
      // The code below would fail for the smallest value, because this value
      // has no positive counterpart. For example, an int8_t ranges from -128 to
      // 127. Hence, an int8_t cannot represent `abs(-128)`.
      std::string_view result;
      if constexpr (sizeof(T) == 1) {
        result = "-128"sv;
      } else if constexpr (sizeof(T) == 2) {
        result = "-32768"sv;
      } else if constexpr (sizeof(T) == 4) {
        result = "-2147483648"sv;
      } else {
        static_assert(sizeof(T) == 8);
        result = "-9223372036854775808"sv;
      }
      buf.insert(buf.end(), result.begin(), result.end());
      return;
    }
    if (x < 0) {
      buf.push_back('-');
      x = -x;
    }
  }
  // Fill the buffer.
  *p++ = static_cast<char>((x % 10) + '0');
  x /= 10;
  while (x != 0) {
    *p++ = static_cast<char>((x % 10) + '0');
    x /= 10;
  }
  // Now, the buffer holds the string representation in reverse order.
  do {
    buf.push_back(*--p);
  } while (p != stack_buffer);
}

template <class Buffer, class T>
std::enable_if_t<std::is_floating_point<T>::value> print(Buffer& buf, T x) {
  // TODO: Check whether to_chars is available on supported compilers and
  //       re-implement using the new API as soon as possible.
  auto str = std::to_string(x);
  if (str.find('.') != std::string::npos) {
    // Drop trailing zeros.
    while (str.back() == '0')
      str.pop_back();
    // Drop trailing dot as well if we've removed all decimal places.
    if (str.back() == '.')
      str.pop_back();
  }
  buf.insert(buf.end(), str.begin(), str.end());
}

template <class Buffer, class Rep, class Period>
void print(Buffer& buf, std::chrono::duration<Rep, Period> x) {
  using namespace std::literals;
  if (x.count() == 0) {
    auto str = "0s"sv;
    buf.insert(buf.end(), str.begin(), str.end());
    return;
  }
  auto try_print = [&buf](auto converted, std::string_view suffix) {
    if (converted.count() < 1)
      return false;
    print(buf, converted.count());
    buf.insert(buf.end(), suffix.begin(), suffix.end());
    return true;
  };

  using hours = std::chrono::duration<double, std::ratio<3600>>;
  using minutes = std::chrono::duration<double, std::ratio<60>>;
  using seconds = std::chrono::duration<double>;
  using milliseconds = std::chrono::duration<double, std::milli>;
  using microseconds = std::chrono::duration<double, std::micro>;
  if (try_print(std::chrono::duration_cast<hours>(x), "h")
      || try_print(std::chrono::duration_cast<minutes>(x), "min")
      || try_print(std::chrono::duration_cast<seconds>(x), "s")
      || try_print(std::chrono::duration_cast<milliseconds>(x), "ms")
      || try_print(std::chrono::duration_cast<microseconds>(x), "us"))
    return;
  auto converted = std::chrono::duration_cast<std::chrono::nanoseconds>(x);
  print(buf, converted.count());
  auto suffix = "ns"sv;
  buf.insert(buf.end(), suffix.begin(), suffix.end());
}

// size_t print_timestamp_v1(char* buf, size_t buf_size, time_t ts, size_t ms) {
//   tm time_buf;
// #ifdef CAF_MSVC
//   localtime_s(&time_buf, &ts);
// #else
//   localtime_r(&ts, &time_buf);
// #endif
//   auto pos = strftime(buf, buf_size, "%FT%T", &time_buf);
//   buf[pos++] = '.';
//   if (ms > 0) {
//     assert(ms < 1000);
//     buf[pos++] = static_cast<char>((ms / 100) + '0');
//     buf[pos++] = static_cast<char>(((ms % 100) / 10) + '0');
//     buf[pos++] = static_cast<char>((ms % 10) + '0');
//   } else {
//     for (int i = 0; i < 3; ++i)
//       buf[pos++] = '0';
//   }
//   buf[pos] = '\0';
//   return pos;
// }

template <class Buffer, class Duration>
void print(Buffer& buf,
           std::chrono::time_point<std::chrono::system_clock, Duration> x) {
  namespace sc = std::chrono;
  using clock_type = sc::system_clock;
  using clock_timestamp = typename clock_type::time_point;
  using clock_duration = typename clock_type::duration;
  auto tse = x.time_since_epoch();
  clock_timestamp as_sys_time{sc::duration_cast<clock_duration>(tse)};
  auto secs = clock_type::to_time_t(as_sys_time);
  auto msecs = sc::duration_cast<sc::milliseconds>(tse).count() % 1000;
  // We print in ISO 8601 format, e.g., "2020-09-01T15:58:42.372". 32-Bytes are
  // more than enough space.
  char stack_buffer[32];
  buf.insert(buf.end(), stack_buffer, stack_buffer);
}
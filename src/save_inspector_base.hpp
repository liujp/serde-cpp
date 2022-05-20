
// This file is part of CAF, the C++ Actor Framework. See the file LICENSE in
#pragma once

#include <string_view>
#include <tuple>

#include "inspector_access.hpp"
#include "save_inspector.hpp"
#include "type_id.hpp"

// subtype inherit the class, here is binary_serialize class
template <class Subtype> class save_inspector_base : public save_inspector {
public:
  using super = save_inspector;
  save_inspector_base() = default;
  virtual ~save_inspector_base() {}

  template <class T> constexpr auto object(T &) noexcept {
    return super::object_t<Subtype>{type_id_or_invalid<T>(),
                                    type_name_or_anonymous<T>(), dptr()};
  }

  constexpr auto virtual_object(std::string_view type_name) noexcept {
    return super::object_t<Subtype>{invalid_type_id, type_name, dptr()};
  }

  template <class T> bool begin_object_t() {
    return dref().begin_object(type_id_v<T>, type_name_v<T>); /*return true*/
  }

  // where T = (member type) value_type + size(): list + vector
  template <class T> bool list(const T &xs) {
    using value_type = typename T::value_type;
    auto size = xs.size();
    if (!dref().begin_sequence(size)) /*add size to the first*/
      return false;
    for (auto &&val : xs) {
      using found_type = std::decay_t<decltype(val)>;
      if constexpr (std::is_same<found_type, value_type>::value) {
        if (!save(dref(), val))
          return false;
      } else {
        // Deals with atrocities like std::vector<bool>.
        auto tmp = static_cast<value_type>(val);
        if (!save(dref(), tmp))
          return false;
      }
    }
    return dref().end_sequence(); /*return true*/
  }

  // where T = map; serialize a map
  template <class T> bool map(const T &xs) {
    if (!dref().begin_associative_array(xs.size())) /*begin_sequence(size)*/
      return false;
    for (auto &&kvp : xs) {
      if (!(dref().begin_key_value_pair()    /*return true*/
            && save(dref(), kvp.first)       //
            && save(dref(), kvp.second)      //
            && dref().end_key_value_pair())) /*return true*/
        return false;
    }
    return dref().end_associative_array();
  }

  // where T = tuple
  template <class T, size_t... Is>
  bool tuple(const T &xs, std::index_sequence<Is...>) {
    using std::get;
    return dref().begin_tuple(sizeof...(Is))     /*return true*/
           && (save(dref(), get<Is>(xs)) && ...) //
           && dref().end_tuple();                /*return true*/
  }

  template <class T> bool tuple(const T &xs) {
    return tuple(xs, std::make_index_sequence<std::tuple_size<T>::value>{});
  }

  template <class T, size_t N> bool tuple(T (&xs)[N]) {
    if (!dref().begin_tuple(N))
      return false;
    for (size_t index = 0; index < N; ++index)
      if (!save(dref(), xs[index]))
        return false;
    return dref().end_tuple();
  }

  // -- dispatching to load/save functions -------------------------------------

  template <class T>[[nodiscard]] bool apply(const T &x) {
    return save(dref(), x);
  }

  template <class Get, class Set>[[nodiscard]] bool apply(Get &&get, Set &&) {
    return save(dref(), get());
  }

private:
  Subtype *dptr() { return static_cast<Subtype *>(this); }
  Subtype &dref() { return *static_cast<Subtype *>(this); }
};

#pragma once

#include <string>
#include <string_view>
#include <utility>

#include "inspector_access.hpp"
#include "my_error.hpp"

class save_inspector {
public:
  static constexpr bool is_loading = false;
  save_inspector() = default;
  virtual ~save_inspector() {}

  void set_error(error &stop_reason) { err_ = std::move(stop_reason); }

  template <class... Ts> void emplace_error(Ts &&... xs) {
    err_ = set_error(100);
  }

  const error &get_error() const noexcept { return err_; }

  error &&move_error() noexcept { return std::move(err_); }

  template <class... Ts> void field_invariant_check_failed(error msg) {
    emplace_error(std::move(msg));
  }

  template <class... Ts> void field_value_synchronization_failed(error msg) {
    emplace_error(std::move(msg));
  }

  template <class... Ts> void invalid_field_type(error msg) {
    emplace_error(std::move(msg));
  }

  template <class T, class U> struct field_with_fallback_t {
    std::string_view field_name;
    T *val;
    U fallback;

    // Inspector: binary_serialize
    template <class Inspector> bool operator()(Inspector &f) {
      auto is_present = [this] { return *val != fallback; };
      auto get = [this] { return *val; };
      return save_field(f, field_name, is_present, get);
    }

    template <class Predicate>
    field_with_fallback_t &&invariant(Predicate &&) && {
      return std::move(*this);
    }
  };

  template <class T> struct field_t {
    std::string_view field_name;
    T *val;

    template <class Inspector> bool operator()(Inspector &f) {
      return save_field(f, field_name, *val);
    }

    template <class U> auto fallback(U value) && {
      return field_with_fallback_t<T, U>{field_name, val, std::move(value)};
    }

    template <class Predicate> field_t &&invariant(Predicate &&) && {
      return std::move(*this);
    }
  };

  template <class T, class Get, class U> struct virt_field_with_fallback_t {
    std::string_view field_name;
    Get get;
    U fallback;

    template <class Inspector> bool operator()(Inspector &f) {
      auto is_present = [this] { return get() != fallback; };
      return save_field(f, field_name, is_present, get);
    }

    template <class Predicate>
    virt_field_with_fallback_t &&invariant(Predicate &&) && {
      return std::move(*this);
    }
  };

  template <class T, class Get> struct virt_field_t {
    std::string_view field_name;
    Get get;

    template <class Inspector> bool operator()(Inspector &f) {
      auto &&x = get();
      return save_field(f, field_name, as_mutable_ref(x));
    }

    template <class U> auto fallback(U value) && {
      return virt_field_with_fallback_t<T, Get, U>{
          field_name,
          std::move(get),
          std::move(value),
      };
    }

    template <class Predicate> virt_field_t &&invariant(Predicate &&) && {
      return std::move(*this);
    }
  };

  template <class T, class IsPresent, class Get> struct optional_virt_field_t {
    std::string_view field_name;
    IsPresent is_present;
    Get get;

    template <class Inspector> bool operator()(Inspector &f) {
      return save_field(f, field_name, is_present, get);
    }
  };

  template <class Inspector, class SaveCallback>
  struct object_with_save_callback_t {
    type_id_t object_type;
    std::string_view object_name;
    Inspector *f;
    SaveCallback save_callback;

    template <class... Fields> bool fields(Fields &&... fs) {
      using save_callback_result = decltype(save_callback());
      if (!(f->begin_object(object_type, object_name) && (fs(*f) && ...)))
        return false;
      if constexpr (std::is_same<save_callback_result, bool>::value) {
        if (!save_callback()) {
          f->set_error(sec::save_callback_failed);
          return false;
        }
      } else {
        if (auto err = save_callback()) {
          f->set_error(std::move(err));
          return false;
        }
      }
      return f->end_object();
    }

    auto pretty_name(std::string_view name) && { return object_t{name, f}; }

    template <class F> object_with_save_callback_t &&on_load(F &&) && {
      return std::move(*this);
    }
  };

  template <class Inspector> struct object_t {
    type_id_t object_type;
    std::string_view object_name;
    Inspector *f;

    template <class... Fields> bool fields(Fields &&... fs) {
      return f->begin_object(object_type, object_name) //
             && (fs(*f) && ...)                        //
             && f->end_object();
    }

    auto pretty_name(std::string_view name) && {
      return object_t{object_type, name, f};
    }

    template <class F> object_t &&on_load(F &&) && { return std::move(*this); }

    template <class F> auto on_save(F fun) && {
      return object_with_save_callback_t<Inspector, F>{
          object_type,
          object_name,
          f,
          std::move(fun),
      };
    }
  };

  template <class T> static auto field(std::string_view name, T &x) {
    static_assert(!std::is_const<T>::value);
    return field_t<T>{name, std::addressof(x)};
  }

  template <class Get, class Set>
  static auto field(std::string_view name, Get get, Set &&) {
    using field_type = std::decay_t<decltype(get())>;
    return virt_field_t<field_type, Get>{name, get};
  }

  template <class IsPresent, class Get, class... Ts>
  static auto field(std::string_view name, IsPresent is_present, Get get,
                    Ts &&...) {
    using field_type = std::decay_t<decltype(get())>;
    return optional_virt_field_t<field_type, IsPresent, Get>{
        name,
        is_present,
        get,
    };
  }

protected:
  error err_;
};

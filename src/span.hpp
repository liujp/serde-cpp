#pragma once

#include <array>
#include <cstddef>

template <class T, class To> class has_convertible_data_member {
private:
  template <class U>
  static auto sfinae(U *x) -> std::integral_constant<
      bool, std::is_convertible<decltype(x->data()), To *>::value>;

  template <class U> static auto sfinae(...) -> std::false_type;

  using sfinae_type = decltype(sfinae<T>(nullptr));

public:
  static constexpr bool value = sfinae_type::value;
};

template <class T>
using remove_reference_t = typename std::remove_reference<T>::type;

template <class T> class span {
public:
  using element_type = T;
  using value_type = typename std::remove_cv<T>::type;
  using index_type = size_t;
  using difference_type = ptrdiff_t;
  using pointer = T *;
  using const_pointer = const T *;
  using reference = T &;
  using const_reference = T &;
  using iterator = pointer;
  using const_iterator = const_pointer;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;
  constexpr span() noexcept : begin_(nullptr), size_(0) {}
  constexpr span(pointer ptr, size_t size) : begin_(ptr), size_(size) {}
  constexpr span(pointer first, pointer last)
      : begin_(first), size_(static_cast<size_t>(last - first)) {}

  template <size_t Size>
  constexpr span(element_type (&arr)[Size]) noexcept
      : begin_(arr), size_(Size) {}

  template <class C, class = std::enable_if_t<
                         has_convertible_data_member<C, value_type>::value>>
  span(C &xs) noexcept : begin_(xs.data()), size_(xs.size()) {}

  template <class C, class = std::enable_if_t<
                         has_convertible_data_member<C, value_type>::value>>
  span(const C &xs) noexcept : begin_(xs.data()), size_(xs.size()) {
    // nop
  }

  constexpr span(const span &) noexcept = default;

  span &operator=(const span &) noexcept = default;

  constexpr iterator begin() const noexcept { return begin_; }

  constexpr const_iterator cbegin() const noexcept { return begin_; }

  constexpr iterator end() const noexcept { return begin() + size_; }

  constexpr const_iterator cend() const noexcept { return cbegin() + size_; }

  constexpr reverse_iterator rbegin() const noexcept {
    return reverse_iterator{end()};
  }

  constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator{end()};
  }

  constexpr reverse_iterator rend() const noexcept {
    return reverse_iterator{begin()};
  }

  constexpr const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator{begin()};
  }

  constexpr reference operator[](size_t index) const noexcept {
    return begin_[index];
  }

  constexpr reference front() const noexcept { return *begin_; }

  constexpr reference back() const noexcept { return (*this)[size_ - 1]; }

  constexpr size_t size() const noexcept { return size_; }

  constexpr size_t size_bytes() const noexcept {
    return size_ * sizeof(element_type);
  }

  constexpr bool empty() const noexcept { return size_ == 0; }

  constexpr pointer data() const noexcept { return begin_; }

  constexpr span subspan(size_t offset, size_t num_bytes) const {
    return {begin_ + offset, num_bytes};
  }

  constexpr span subspan(size_t offset) const {
    return {begin_ + offset, size_ - offset};
  }

  constexpr span first(size_t num_bytes) const { return {begin_, num_bytes}; }

  constexpr span last(size_t num_bytes) const {
    return subspan(size_ - num_bytes, num_bytes);
  }

private:
  pointer begin_;
  size_t size_;
};

// global operations for span
template <class T> auto begin(const span<T> &xs) -> decltype(xs.begin()) {
  return xs.begin();
}

template <class T> auto cbegin(const span<T> &xs) -> decltype(xs.cbegin()) {
  return xs.cbegin();
}

template <class T> auto end(const span<T> &xs) -> decltype(xs.end()) {
  return xs.end();
}

template <class T> auto cend(const span<T> &xs) -> decltype(xs.cend()) {
  return xs.cend();
}

template <class T> span<const std::byte> as_bytes(span<T> xs) {
  return {reinterpret_cast<const std::byte *>(xs.data()), xs.size_bytes()};
}

template <class T> span<std::byte> as_writable_bytes(span<T> xs) {
  return {reinterpret_cast<std::byte *>(xs.data()), xs.size_bytes()};
}

template <class T>
auto make_span(T &xs) -> span<remove_reference_t<decltype(xs[0])>> {
  return {xs.data(), xs.size()};
}

template <class T, size_t N> span<T> make_span(T (&xs)[N]) { return {xs, N}; }

template <class T> span<T> make_span(T *first, size_t size) {
  return {first, size};
}

template <class T> span<T> make_span(T *first, T *last) {
  return {first, last};
}

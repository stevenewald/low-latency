#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <utility>

template <typename T, typename Compare, std::size_t N> class SortedSet {
public:
  static_assert(N > 0, "Capacity must be at least 1");

  SortedSet() noexcept : size_(0) {}

  // Copy-insert
  bool insert(T value) { return emplace(value); }
  // Move-insert
  // bool insert(const T& value) { return emplace(std::move(value)); }

  // Erase value if present.
  bool erase(const T &value) {
    T *begin = data_.data();
    T *end = begin + size_;
    T *it = std::lower_bound(begin, end, value, comp_);
    if (it == end || comp_(value, *it) || comp_(*it, value))
      return false;
    std::move(it + 1, end, it);
    --size_;
    return true;
  }

  void clear() noexcept { size_ = 0; }

  std::size_t size() const noexcept { return size_; }
  constexpr std::size_t capacity() const noexcept { return N; }
  bool empty() const noexcept { return size_ == 0; }
  bool full() const noexcept { return size_ == N; }

  // Iterators
  T *begin() noexcept { return data_.data(); }
  T *end() noexcept { return data_.data() + size_; }
  const T *begin() const noexcept { return data_.data(); }
  const T *end() const noexcept { return data_.data() + size_; }

private:
  // Generic emplace for copy/move
  template <typename U> bool emplace(U &&value) {
    if (size_ >= N)
      return false;
    T *begin = data_.data();
    T *end = begin + size_;
    T *it = std::lower_bound(begin, end, value, comp_);
    if (it != end && !comp_(value, *it) && !comp_(*it, value))
      return false;
    // shift elements right in one call
    std::move_backward(it, end, end + 1);
    *it = std::forward<U>(value);
    ++size_;
    return true;
  }

  std::array<T, N> data_{};
  std::size_t size_;
  Compare comp_;
};

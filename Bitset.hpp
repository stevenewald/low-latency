#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
template <std::size_t N, bool Reverse> class Bitset {
public:
  static constexpr std::size_t BITS = 64;
  static constexpr std::size_t WORDS = (N) / BITS;
  static constexpr std::size_t ignore = 2;

  static constexpr uint16_t invalid = (Reverse) ? 0 : uint16_t(~0);
  std::uint16_t cached = invalid;

  std::array<uint64_t, WORDS> data_{};

  __attribute__((always_inline, hot)) inline void insert(std::uint16_t i) {
    data_[i / BITS] |= 1ULL << (i % BITS);
    if constexpr (Reverse) {
      cached = std::max(i, cached);
    }
    if constexpr (!Reverse) {
      cached = std::min(i, cached);
    }
  }
  __attribute__((always_inline, hot)) inline void erase(std::uint16_t i) {
    data_[i / BITS] &= ~(1ULL << (i % BITS));
    cached = invalid;
  }

  __attribute__((always_inline, hot)) inline std::uint16_t begin() {
    if (cached != invalid)
      return cached;
    if constexpr (Reverse) {
      for (std::size_t w = WORDS - ignore; w-- > ignore;) {
        if (data_[w])
          return cached = w * BITS + (BITS - 1 - __builtin_clzll(data_[w]));
      }
    }
    if constexpr (!Reverse) {
      for (std::size_t w = ignore; w < WORDS - ignore; ++w) {
        if (data_[w])
          return cached = w * BITS + __builtin_ctzll(data_[w]);
      }
    }
    cached = invalid;
    return N;
  }
};

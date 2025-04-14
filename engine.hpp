#pragma once

#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>
#include <array>
#include <boost/container/flat_map.hpp>
#include <boost/container/stable_vector.hpp>
#include <cstdint>
#include <functional>
#include <immintrin.h>
#include <iostream>
#include <limits>
#include <queue>
#include <stack>
#include <stdexcept>

enum class Side : uint8_t { BUY = 0, SELL = 1 };

using IdType = uint32_t;
using PriceType = uint16_t;
using QuantityType = uint16_t;

// You CANNOT change this
struct Order {
  IdType id; // Unique
  PriceType price;
  QuantityType quantity;
  Side side;
};

#pragma once

#include <array>
#include <cstddef>
#include <optional>

static constexpr std::size_t cap2 = 16;

// Simple and performant fixed-size ring buffer
// Assumes single producer / single consumer for simplicity and performance.

template <typename T, std::size_t Capacity> class ringbuf {
public:
  static_assert(Capacity > 0, "Capacity must be greater than 0");

  __attribute__((always_inline)) inline void push(const T &item) {
    auto next_head = (head_ + 1) % Capacity;

    buffer_[head_] = item;
    head_ = next_head;
  }

  __attribute__((always_inline)) inline void pop() {
    tail_ = (tail_ + 1) % Capacity;
  }

  __attribute__((always_inline)) inline T front() const {
    return buffer_[tail_];
  }
  __attribute__((always_inline)) inline T next() const {
    return buffer_[(tail_ + 1) % Capacity];
  }

  __attribute__((always_inline)) inline bool empty() const {
    return head_ == tail_;
  }

  __attribute__((always_inline)) inline std::size_t size() const {
    return (head_ + Capacity - tail_) % Capacity;
  }

  void erase(T item) {
    std::size_t index = tail_;

    for (std::size_t count = 0; count < size(); ++count) {
      if (buffer_[index] == item) {
        break;
      }
      index = (index + 1) % Capacity;
    }

    while (index != head_) {
      std::size_t next_index = (index + 1) % Capacity;
      if (next_index != head_) {
        buffer_[index] = buffer_[next_index];
      }
      index = next_index;
    }

    head_ = (head_ + Capacity - 1) % Capacity;
  }

  std::array<T, cap2> buffer_{};
  uint64_t head_ = 0;
  uint64_t tail_ = 0;
};

using OrderList = ringbuf<IdType, cap2>;

template <std::size_t N> class Bitset {
public:
  static constexpr std::size_t BITS = 64;
  static constexpr std::size_t WORDS = (N) / BITS;

  std::array<uint64_t, WORDS> data_{};

  __attribute__((always_inline)) inline void set(std::size_t i) {
    data_[i / BITS] |= 1ULL << (i % BITS);
  }
  __attribute__((always_inline)) inline void clear(std::size_t i) {
    data_[i / BITS] &= ~(1ULL << (i % BITS));
  }

  __attribute__((always_inline)) inline std::size_t first_set() const {
    for (std::size_t w = 1; w < WORDS; ++w) {
      if (data_[w])
        return w * BITS + __builtin_ctzll(data_[w]);
    }
    return N;
  }

  __attribute__((always_inline)) inline std::size_t last_set() const {
    for (std::size_t w = WORDS; w-- > 0;) {
      if (data_[w])
        return w * BITS + (BITS - 1 - std::countl_zero(data_[w]));
    }
    return N;
  }
};

template <bool Reverse> class alignas(64) OrderBookSide {
public:
  static constexpr size_t k = 1024;
  static constexpr size_t offset = 3456;

  std::array<OrderList, k> levels{};
  Bitset<k> occupied{};

  __attribute__((always_inline)) inline void add(const Order &o) {
    occupied.set(o.price - offset);
    levels[o.price - offset].push(o.id);
  }
  __attribute__((always_inline)) inline OrderList &get(PriceType p) {
    return levels[p - offset];
  }

  __attribute__((always_inline)) inline void mark_mt(PriceType p) {
    occupied.clear(p - offset);
  }

  __attribute__((always_inline)) inline std::pair<OrderList *, PriceType> const
  get_best() {
    if constexpr (Reverse) {
      auto N = occupied.last_set();
      if (N < k) [[likely]] {
        return {&levels[N], N + offset};
      }
    }
    if constexpr (!Reverse) {
      auto N = occupied.first_set();
      if (N < k) [[likely]] {
        return {&levels[N], N + offset};
      }
    }
    return {nullptr, 0};
  }
};

using OrderIdMap = std::array<Order, 10000 > ;
using OrderValidMap = std::array<bool, 10000>;
using OrderSizeMap = std::array<bool, 10000>;

using PriceVolumeMap =
    std::array<uint32_t[2], std::numeric_limits<PriceType>::max()>;

// You CAN and SHOULD change this
struct Orderbook {
  alignas(64) OrderBookSide<true> buyOrders{};
  alignas(64) OrderBookSide<false> sellOrders{};
  alignas(64) OrderIdMap orders{};
  alignas(64) PriceVolumeMap volume{};
  alignas(64) OrderValidMap valid{};
};

extern "C" {

// Takes in an incoming order, matches it, and returns the number of matches
// Partial fills are valid

uint32_t match_order(Orderbook &orderbook, const Order &incoming);

// Sets the new quantity of an order. If new_quantity==0, removes the order
void modify_order_by_id(Orderbook &orderbook, IdType order_id,
                        QuantityType new_quantity);

// Returns total resting volume at a given price point
uint32_t get_volume_at_level(Orderbook &orderbook, Side side,
                             PriceType quantity);

// Performance of these do not matter. They are only used to check correctness
Order lookup_order_by_id(Orderbook &orderbook, IdType order_id);
bool order_exists(Orderbook &orderbook, IdType order_id);
Orderbook *create_orderbook();
}

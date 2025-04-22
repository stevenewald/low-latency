#pragma once

#include "Bitset.hpp"
#include "set.hpp"
#include "set2.hpp"
#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>
#include <array>
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

static constexpr std::size_t cap2 = 20;

template <typename T, std::size_t Capacity> class ringbuf {
public:
  static_assert(Capacity > 0, "Capacity must be greater than 0");

  __attribute__((always_inline, hot)) inline bool push(T item) {
    if (full()) {
      return false;
    }
    buffer_[ins++] = item;
    return true;
  }

  __attribute__((always_inline, hot)) inline void pop() { ++start; }

  __attribute__((always_inline, hot)) inline T front() const {
    return buffer_[start];
  }

  __attribute__((always_inline, hot)) inline bool empty() const {
    return start == ins;
  }
  __attribute__((always_inline, hot)) inline bool full() const {
    return ins == Capacity;
  }

  std::array<T, cap2> buffer_{};
  uint64_t ins = 0;
  uint64_t start = 0;
};

using OrderList = ringbuf<IdType, cap2>;

template <bool Reverse> struct Comparator {
  bool operator()(const PriceType &a, const PriceType &b) const {
    if constexpr (Reverse)
      return a > b; // Descending order
    else
      return a < b; // Ascending order (default)
  }
};

template <bool Reverse> class alignas(64) OrderBookSide {
public:
  static constexpr size_t k = 1024;

  std::array<OrderList, k> levels{};
  // std::set<PriceType, Comparator<Reverse>> occupied{};
  // FastSet<PriceType, Comparator<Reverse>> occupied{};
  SortedSet<PriceType, Comparator<Reverse>, k> occupied{};
  // Bitset<k, Reverse> occupied;

  __attribute__((always_inline, hot)) inline bool add(const Order &o) {
    if (levels[o.price].push(o.id)) {
      occupied.insert(o.price);
      return true;
    }
    return false;
  }
  __attribute__((always_inline, hot)) inline OrderList &get(PriceType p) {
    return levels[p];
  }

  __attribute__((always_inline, hot)) inline void mark_mt(PriceType p) {
    occupied.erase(p);
  }

  __attribute__((always_inline,
                 hot)) inline std::pair<OrderList *, PriceType> const
  get_best() {
    auto N = occupied.begin();
    if (N != occupied.end()) [[likely]] {
      return {&levels[*N], *N};
    }
    return {nullptr, 0};
  }
};

using OrderIdMap = std::array<Order, 10000>;
using OrderValidMap = std::array<bool, 10000>;
using PriceVolumeMap = std::array<uint32_t[2], 1024>;

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

#pragma once

#include "nonstd/ring_span.hpp"
#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>
#include <array>
#include <boost/container/stable_vector.hpp>
#include <cstdint>
#include <functional>
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

static constexpr std::size_t cap = 512;
static constexpr std::size_t cap2 = 16;

// Simple and performant fixed-size ring buffer
// Assumes single producer / single consumer for simplicity and performance.

template <typename T, std::size_t Capacity> class ringbuf {
public:
  static_assert(Capacity > 0, "Capacity must be greater than 0");

  explicit ringbuf(std::stack<IdType *> &ptr) {
    buffer_ = ptr.top();
    ptr.pop();
  }

  ~ringbuf() {}

  void push(const T &item) {
    auto next_head = (head_ + 1) % Capacity;

    buffer_[head_] = item;
    head_ = next_head;
  }

  void pop() { tail_ = (tail_ + 1) % Capacity; }

  T &front() { return buffer_[tail_]; }

  [[nodiscard]] bool empty() const { return head_ == tail_; }

  [[nodiscard]] std::size_t size() const {
    return (head_ + Capacity - tail_) % Capacity;
  }

  [[nodiscard]] constexpr std::size_t capacity() const {
    return Capacity - 1; // actual usable capacity
  }

  void erase(const T &item) {
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

  T *buffer_{};
  uint32_t head_ = 0;
  uint32_t tail_ = 0;
};

using OrderList = ringbuf<IdType, cap2>;
using OrderIdMap = std::array<Order, 10000>;
using OrderValidMap = std::array<bool, 10000>;
using OrderSizeMap = std::array<bool, 10000>;

using PriceVolumeMap =
    std::array<uint32_t[2], std::numeric_limits<PriceType>::max()>;

// You CAN and SHOULD change this
struct Orderbook {
  alignas(64) absl::btree_map<PriceType, OrderList,
                              std::greater<PriceType>> buyOrders{};
  alignas(64) absl::btree_map<PriceType, OrderList> sellOrders{};
  alignas(64) OrderIdMap orders{};
  alignas(64) OrderValidMap ovalid{};
  alignas(64) PriceVolumeMap volume{};
  alignas(64) std::array<std::array<IdType, cap2>, cap> frees2{};
  alignas(64) std::stack<IdType *> frees;

  Orderbook() {
    for (std::size_t i = 0; i < cap; ++i) {
      frees.push(frees2[i].data());
    }
  }
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

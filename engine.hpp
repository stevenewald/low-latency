#pragma once

#include <absl/container/btree_map.h>
#include <absl/container/flat_hash_map.h>
#include <array>
#include <boost/container/stable_vector.hpp>
#include <cstdint>
#include <functional>
#include <limits>

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

using OrderList = std::vector<IdType>;
using OrderIt = OrderList::iterator;
using OrderIdMap = std::array<Order, 10000>;
using OrderValidMap = std::array<bool, 10000>;

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

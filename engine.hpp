#pragma once

#include <boost/container/stable_vector.hpp>
#include <cstdint>
#include <functional>
#include <limits>
#include <list>
#include <map>
#include <unordered_map>

enum class Side : uint8_t { BUY, SELL };

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

using OrderList = boost::container::stable_vector<Order>;
using OrderIt = OrderList::iterator;
using OrderIdMap = std::unordered_map<IdType, OrderIt>;

using PriceVolumeMap =
    std::array<uint32_t, std::numeric_limits<PriceType>::max()>;

// You CAN and SHOULD change this
struct Orderbook {
  std::map<PriceType, OrderList, std::greater<PriceType>> buyOrders{};
  std::map<PriceType, OrderList> sellOrders{};
  OrderIdMap buyorders{};
  OrderIdMap sellorders{};
  PriceVolumeMap buyVolume{0};
  PriceVolumeMap sellVolume{0};
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

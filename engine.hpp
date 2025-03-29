#pragma once

#include <cstdint>
#include <list>
#include <map>
#include <optional>

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

// You CAN and SHOULD change this
struct Orderbook {
  std::map<PriceType, std::list<Order>, std::greater<PriceType>> buyOrders;
  std::map<PriceType, std::list<Order>> sellOrders;
};

extern "C" {

// These are the two functions you must optimize
//////////////////////////////////////////////////////////////

// Takes in an incoming order, matches it, and returns the number of matches
// Partial fills are valid
uint32_t match_order(Orderbook &orderbook, const Order &incoming);

// Sets the new quantity of an order. If new_quantity==0, removes the order
void modify_order_by_id(Orderbook &orderbook, IdType order_id,
                        QuantityType new_quantity);

//////////////////////////////////////////////////////////////

// Performance of this does not matter. It is only used to check correctness
Order lookup_order_by_id(Orderbook &orderbook, IdType order_id);
bool order_exists(Orderbook &orderbook, IdType order_id);
}

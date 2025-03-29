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

uint32_t match_order(Orderbook &orderbook, const Order &incoming);

void modify_order_by_id(Orderbook &orderbook, IdType order_id,
                        QuantityType new_quantity);

std::optional<Order> lookup_order_by_id(Orderbook &orderbook, IdType order_id);

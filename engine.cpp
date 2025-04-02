#include "engine.hpp"
#include <functional>
#include <optional>
#include <stdexcept>

// This is an example correct implementation
// It is INTENTIONALLY suboptimal
// You are encouraged to rewrite as much or as little as you'd like

// Templated helper to process matching orders.
// The Condition predicate takes the price level and the incoming order price
// and returns whether the level qualifies.
template <typename OrderMap, typename Condition>
uint32_t process_orders(Order &order, OrderMap &ordersMap, OrderIdMap &idMap,
                        PriceVolumeMap &pvm, Condition cond) {
  uint32_t matchCount = 0;
  auto it = ordersMap.begin();
  while (it != ordersMap.end() && order.quantity > 0 &&
         (it->first == order.price || cond(it->first, order.price))) {
    auto &ordersAtPrice = it->second;
    for (auto orderIt = ordersAtPrice.begin();
         orderIt != ordersAtPrice.end() && order.quantity > 0;) {
      QuantityType trade = std::min(order.quantity, orderIt->quantity);
      order.quantity -= trade;
      orderIt->quantity -= trade;
      pvm[orderIt->price] -= trade;
      ++matchCount;
      if (orderIt->quantity == 0) {
        idMap.erase(orderIt->id);
        orderIt = ordersAtPrice.erase(orderIt);
      } else
        ++orderIt;
    }
    if (ordersAtPrice.empty())
      it = ordersMap.erase(it);
    else
      ++it;
  }
  return matchCount;
}

uint32_t match_order(Orderbook &orderbook, const Order &incoming) {
  uint32_t matchCount = 0;
  Order order = incoming; // Create a copy to modify the quantity

  if (order.side == Side::BUY) {
    // For a BUY, match with sell orders priced at or below the order's price.
    matchCount =
        process_orders(order, orderbook.sellOrders, orderbook.buyorders,
                       orderbook.sellVolume, std::less<>());
    if (order.quantity > 0) {
      auto [it, ins] = orderbook.buyOrders.emplace(order.price, OrderList());
      it->second.push_back(order);
      orderbook.buyorders.emplace(order.id, std::prev(it->second.end()));
      orderbook.buyVolume[order.price] += order.quantity;
    }
  } else { // Side::SELL
    // For a SELL, match with buy orders priced at or above the order's price.
    matchCount =
        process_orders(order, orderbook.buyOrders, orderbook.sellorders,
                       orderbook.buyVolume, std::greater<>());
    if (order.quantity > 0) {
      auto [it, ins] = orderbook.sellOrders.emplace(order.price, OrderList());
      it->second.push_back(order);
      orderbook.sellorders.emplace(order.id, std::prev(it->second.end()));
      orderbook.sellVolume[order.price] += order.quantity;
    }
  }
  return matchCount;
}

// Templated helper to cancel an order within a given orders map.
template <typename OrderMap>
bool modify_order_in_map(OrderMap &ordersMap, IdType order_id,
                         OrderIdMap &ordersIdMap, PriceVolumeMap &volMap,
                         QuantityType new_quantity) {
  auto it = ordersIdMap.find(order_id);
  if (it == ordersIdMap.end()) {
    return false;
  }
  auto price = it->second->price;
  volMap[price] -= it->second->quantity;
  if (new_quantity != 0) {
    it->second->quantity = new_quantity;
    volMap[price] += new_quantity;
  } else {
    auto &priceList = ordersMap.at(price);
    priceList.erase(it->second);
    if (priceList.size() == 0) {
      ordersMap.erase(price);
    }
  }
  return true;
}

void modify_order_by_id(Orderbook &orderbook, IdType order_id,
                        QuantityType new_quantity) {
  if (modify_order_in_map(orderbook.buyOrders, order_id, orderbook.buyorders,
                          orderbook.buyVolume, new_quantity))
    return;
  if (modify_order_in_map(orderbook.sellOrders, order_id, orderbook.sellorders,
                          orderbook.sellVolume, new_quantity))
    return;
}

template <typename OrderMap>
std::optional<Order> lookup_order_in_map(OrderMap &ordersMap, IdType order_id) {
  for (const auto &[price, orderList] : ordersMap) {
    for (const auto &order : orderList) {
      if (order.id == order_id) {
        return order;
      }
    }
  }
  return std::nullopt;
}

uint32_t get_volume_at_level(Orderbook &orderbook, Side side, PriceType price) {
  uint32_t total = 0;
  if (side == Side::BUY) {
    return orderbook.buyVolume[price];
  } else if (side == Side::SELL) {
    return orderbook.sellVolume[price];
  }
  return total;
}

// Functions below here don't need to be performant. Just make sure they're
// correct
Order lookup_order_by_id(Orderbook &orderbook, IdType order_id) {
  auto order1 = lookup_order_in_map(orderbook.buyOrders, order_id);
  auto order2 = lookup_order_in_map(orderbook.sellOrders, order_id);
  if (order1.has_value())
    return *order1;
  if (order2.has_value())
    return *order2;
  throw std::runtime_error("Order not found");
}

bool order_exists(Orderbook &orderbook, IdType order_id) {
  auto order1 = lookup_order_in_map(orderbook.buyOrders, order_id);
  auto order2 = lookup_order_in_map(orderbook.sellOrders, order_id);
  return (order1.has_value() || order2.has_value());
}

Orderbook *create_orderbook() { return new Orderbook; }

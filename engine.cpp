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
template <uint8_t si, typename Condition, typename OrderMap>
uint32_t process_orders(Order &order, OrderMap &ordersMap, OrderIdMap &idMap,
                        PriceVolumeMap &pvm) {
  uint32_t matchCount = 0;
  auto it = ordersMap.begin();
  while (it != ordersMap.end() && order.quantity > 0 &&
         (it->first == order.price || Condition()(it->first, order.price))) {
    auto &ordersAtPrice = it->second;
    for (auto orderIt = ordersAtPrice.begin();
         order.quantity > 0 && orderIt != ordersAtPrice.end();) {
      auto &order2 = idMap[*orderIt].value();
      QuantityType trade = std::min(order.quantity, order2.quantity);
      order.quantity -= trade;
      order2.quantity -= trade;
      pvm[order2.price][si] -= trade;
      ++matchCount;
      if (order2.quantity == 0) {
        idMap[order2.id] = std::nullopt;
        orderIt = ordersAtPrice.erase(orderIt);
      } else {
        break;
      }
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
    matchCount = process_orders<1, std::less<PriceType>>(
        order, orderbook.sellOrders, orderbook.orders, orderbook.volume);
    if (order.quantity > 0) {
      orderbook.buyOrders[order.price].emplace_back(order.id);
      orderbook.orders[order.id] = order;
      orderbook.volume[order.price][0] += order.quantity;
    }
  } else { // Side::SELL
    // For a SELL, match with buy orders priced at or above the order's price.
    matchCount = process_orders<0, std::greater<PriceType>>(
        order, orderbook.buyOrders, orderbook.orders, orderbook.volume);
    if (order.quantity > 0) {
      orderbook.sellOrders[order.price].emplace_back(order.id);
      orderbook.orders[order.id] = order;
      orderbook.volume[order.price][1] += order.quantity;
    }
  }
  return matchCount;
}

// Templated helper to cancel an order within a given orders map.
template <typename OrderMap>
bool modify_order_in_map(Order &order, OrderIdMap &ordersMap, OrderMap &mp,
                         QuantityType new_quantity) {
  if (new_quantity != 0) {
    order.quantity = new_quantity;
  } else {
    mp[order.price].erase(
        std::find(mp[order.price].begin(), mp[order.price].end(), order.id));
    ordersMap[order.id] = std::nullopt;
  }
  return true;
}

void modify_order_by_id(Orderbook &orderbook, IdType order_id,
                        QuantityType new_quantity) {
  auto &order_opt = orderbook.orders[order_id];
  if (!order_opt)
    return;
  auto &order = order_opt.value();
  orderbook.volume[order.price][static_cast<uint8_t>(order.side)] +=
      (new_quantity - order.quantity);
  if (order.side == Side::BUY) {
    modify_order_in_map(order, orderbook.orders, orderbook.buyOrders,
                        new_quantity);
  } else {
    modify_order_in_map(order, orderbook.orders, orderbook.sellOrders,
                        new_quantity);
  }
}

uint32_t get_volume_at_level(Orderbook &orderbook, Side side, PriceType price) {
  uint32_t total = 0;
  return orderbook.volume[price][static_cast<uint8_t>(side)];
  return total;
}

// Functions below here don't need to be performant. Just make sure they're
// correct
Order lookup_order_by_id(Orderbook &orderbook, IdType order_id) {
  auto order1 = orderbook.orders[order_id];
  if (order1.has_value())
    return *order1;
  throw std::runtime_error("Order not found");
}

bool order_exists(Orderbook &orderbook, IdType order_id) {
  auto order1 = orderbook.orders[order_id];
  return order1.has_value();
}

Orderbook *create_orderbook() { return new Orderbook; }

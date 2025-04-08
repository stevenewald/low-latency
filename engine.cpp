#include "engine.hpp"
#include <functional>
#include <optional>
#include <stdexcept>
#include <sys/mman.h>

// This is an example correct implementation
// It is INTENTIONALLY suboptimal
// You are encouraged to rewrite as much or as little as you'd like

// Templated helper to process matching orders.
// The Condition predicate takes the price level and the incoming order price
// and returns whether the level qualifies.

template <uint8_t si, typename Condition, typename T>
__attribute__((always_inline)) inline uint32_t
process_orders(Order &order, T &ordersMap, OrderIdMap &idMap,
               PriceVolumeMap &pvm, OrderValidMap &ovm) {
  uint32_t matchCount = 0;
  while (true) {
    if (order.quantity == 0)
      break;
    auto [p, ordersAtPrice] = ordersMap.get_best();
    if (p == 0)
      break;
    if (!(Condition{}(p, order.price) || p == order.price))
      break;
    auto &pvm2 = pvm[p];
    do {
      auto &order2 = idMap[ordersAtPrice->front()];
      QuantityType trade = std::min(order.quantity, order2.quantity);
      order.quantity -= trade;
      order2.quantity -= trade;
      pvm2[si] -= trade;
      ++matchCount;
      if (order2.quantity == 0) {
        ovm[order2.id] = false;
        ordersAtPrice->pop();
        if (order.quantity == 0)
          break;
      } else {
        break;
      }
    } while (!ordersAtPrice->empty());
    if (!ordersAtPrice->empty()) {
      break;
    } else {
      ordersMap.mark_mt(p);
    }
  }
  return matchCount;
}

uint32_t match_order(Orderbook &orderbook, const Order &incoming) {
  uint32_t matchCount = 0;
  Order order = incoming; // Create a copy to modify the quantity
  if (order.side == Side::BUY) {
    // For a BUY, match with sell orders priced at or below the order's price.
    matchCount = process_orders<1, std::less<PriceType>>(
        order, orderbook.sellOrders, orderbook.orders, orderbook.volume,
        orderbook.ovalid);
    if (order.quantity > 0) {
      orderbook.ovalid[order.id] = true;
      orderbook.buyOrders.add(order);
      orderbook.orders[order.id] = order;
      orderbook.volume[order.price][0] += order.quantity;
    }
  } else { // Side::SELL
    // For a SELL, match with buy orders priced at or above the order's price.
    matchCount = process_orders<0, std::greater<PriceType>>(
        order, orderbook.buyOrders, orderbook.orders, orderbook.volume,
        orderbook.ovalid);
    if (order.quantity > 0) {
      orderbook.ovalid[order.id] = true;
      orderbook.sellOrders.add(order);
      orderbook.orders[order.id] = order;
      orderbook.volume[order.price][1] += order.quantity;
    }
  }
  return matchCount;
}

// Templated helper to cancel an order within a given orders map.

template <typename T>
bool modify_order_in_map(Order &order, OrderValidMap &ordersMap, T &mp,
                         QuantityType new_quantity) {
  if (new_quantity != 0) {
    order.quantity = new_quantity;
  } else {
    auto &t = mp.get(order.price);
    t.erase(order.id);
    if (t.empty())
      mp.mark_mt(order.price);
    ordersMap[order.id] = false;
  }
  return true;
}

void modify_order_by_id(Orderbook &orderbook, IdType order_id,
                        QuantityType new_quantity) {
  if (!orderbook.ovalid[order_id])
    return;
  auto &order = orderbook.orders[order_id];
  if (order.side == Side::BUY) {
    orderbook.volume[order.price][0] += (new_quantity - order.quantity);
    modify_order_in_map(order, orderbook.ovalid, orderbook.buyOrders,
                        new_quantity);
  } else {
    orderbook.volume[order.price][1] += (new_quantity - order.quantity);
    modify_order_in_map(order, orderbook.ovalid, orderbook.sellOrders,
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
  if (orderbook.ovalid[order_id])
    return orderbook.orders[order_id];
  throw std::runtime_error("Order not found");
}

bool order_exists(Orderbook &orderbook, IdType order_id) {
  return orderbook.ovalid[order_id];
}

// good: 6

Orderbook *create_orderbook() {
  Orderbook *ob = new (std::align_val_t{64}) Orderbook();
  madvise(ob, sizeof(Orderbook), MADV_WILLNEED | MADV_HUGEPAGE);
  return ob;
}

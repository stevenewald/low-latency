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

template <uint8_t si, typename Condition, typename T, typename T2>
__attribute__((always_inline, hot)) inline uint32_t
process_orders(Order &order, T &ordersMap, T2 &om2, OrderValidMap &valid,
               OrderIdMap &idMap, PriceVolumeMap &pvm) {
  uint32_t matchCount = 0;
  while (order.quantity > 0) {
    auto [ordersAtPrice, p] = ordersMap.get_best();
    if (p == 0 || !(Condition{}(p, order.price)))
      break;
    uint32_t &pvm2 = pvm[p][si];
    do {
      ++matchCount;
      if (!valid[ordersAtPrice->front()]) [[unlikely]] {
        ordersAtPrice->pop();
        if (ordersAtPrice->empty()) {
          ordersMap.mark_mt(p);
          break;
        }
        continue;
      }
      auto &order2 = idMap[ordersAtPrice->front()];
      QuantityType trade = std::min(order.quantity, order2.quantity);
      order.quantity -= trade;
      order2.quantity -= trade;
      pvm2 -= trade;
      if (order2.quantity == 0) {
        valid[order2.id] = false;
        ordersAtPrice->pop();
        if (ordersAtPrice->empty()) {
          ordersMap.mark_mt(p);
          break;
        }
      }
    } while (order.quantity > 0);
  }
  if (order.quantity > 0 && !om2.get(order.price).full()) {
    om2.add(order);
    idMap[order.id] = order;
    valid[order.id] = true;
    pvm[order.price][0] += order.quantity;
  }
  return matchCount;
}

uint32_t match_order(Orderbook &orderbook, const Order &incoming) {
  Order order = incoming; // Create a copy to modify the quantity
  order.price -= 3456;
  if (order.side == Side::BUY) {
    // For a BUY, match with sell orders priced at or below the order's price.
    return process_orders<1, std::less_equal<PriceType>>(
        order, orderbook.sellOrders, orderbook.buyOrders, orderbook.valid,
        orderbook.orders, orderbook.volume);
  } else { // Side::SELL
    // For a SELL, match with buy orders priced at or above the order's price.
    return process_orders<0, std::greater_equal<PriceType>>(
        order, orderbook.buyOrders, orderbook.sellOrders, orderbook.valid,
        orderbook.orders, orderbook.volume);
  }
}

// Templated helper to cancel an order within a given orders map.

void modify_order_by_id(Orderbook &orderbook, IdType order_id,
                        QuantityType new_quantity) {
  if (!orderbook.valid[order_id])
    return;
  auto &order = orderbook.orders[order_id];
  orderbook.volume[order.price][static_cast<size_t>(order.side)] +=
      (new_quantity - order.quantity);
  if (new_quantity != 0) [[likely]] {
    order.quantity = new_quantity;
  } else {
    orderbook.valid[order.id] = false;
  }
}

uint32_t get_volume_at_level(Orderbook &orderbook, Side side, PriceType price) {
  return orderbook.volume[price][static_cast<size_t>(side)];
}

// Functions below here don't need to be performant. Just make sure they're
// correct
Order lookup_order_by_id(Orderbook &orderbook, IdType order_id) {
  return orderbook.orders[order_id];
}

bool order_exists(Orderbook &orderbook, IdType order_id) {
  return orderbook.valid[order_id];
}

// good: 6

Orderbook *create_orderbook() {
  Orderbook *ob = new (std::align_val_t{256}) Orderbook();
  madvise(ob, sizeof(Orderbook), MADV_WILLNEED | MADV_HUGEPAGE);
  return ob;
}

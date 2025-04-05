#include "engine.hpp"
#include <iostream>
#include <stdexcept>

void myassert(bool t) {
	if(!t) throw std::runtime_error("uhoh");
}

// We may add to these later on, but will provide additional tests before the
// deadline
// These should catch the vast majority of bugs

// Test 0: Simple order lookup
void test_lookup_order() {
  std::cout << "Test 0: Simple order lookup" << std::endl;
  Orderbook ob;
  Order sellOrder{1, 100, 10, Side::SELL};
  // Initially, order with id 1 should not exist.
  myassert(!order_exists(ob, 1));

  uint32_t matches = match_order(ob, sellOrder);
  myassert(matches == 0);

  // Now, order with id 1 should exist.
  myassert(order_exists(ob, 1));
  Order order_lookup = lookup_order_by_id(ob, 1);
  myassert(order_lookup.id == 1);
  myassert(order_lookup.price == 100);
  myassert(order_lookup.quantity == 10);
  myassert(order_lookup.side == Side::SELL);

  std::cout << "Test 0 passed." << std::endl;
}

// Test 1: Simple match and modify
void test_simple_match_and_modify() {
  std::cout << "Test 1: Simple match and modify" << std::endl;
  Orderbook ob;
  // Insert a sell order.
  Order sellOrder{1, 100, 10, Side::SELL};
  uint32_t matches = match_order(ob, sellOrder);
  myassert(matches == 0);

  // A buy order that partially matches the sell order.
  Order buyOrder{2, 100, 5, Side::BUY};
  matches = match_order(ob, buyOrder);
  myassert(matches == 1); // one resting order matched

  // Remaining sell order should have quantity 5.
  myassert(order_exists(ob, 1));
  Order order_lookup = lookup_order_by_id(ob, 1);
  myassert(order_lookup.quantity == 5);

  // Modify the remaining sell order.
  modify_order_by_id(ob, 1, 0);
  myassert(!order_exists(ob, 1));

  std::cout << "Test 1 passed." << std::endl;
}

// Test 2: Multiple matches across price levels
void test_multiple_matches() {
  std::cout << "Test 2: Multiple matches across price levels" << std::endl;
  Orderbook ob;
  // Insert two sell orders at different prices.
  Order sellOrder1{3, 90, 5, Side::SELL};
  Order sellOrder2{4, 95, 5, Side::SELL};
  uint32_t matches = match_order(ob, sellOrder1);
  myassert(matches == 0);
  matches = match_order(ob, sellOrder2);
  myassert(matches == 0);

  // A buy order that can match both.
  Order buyOrder{5, 100, 8, Side::BUY};
  matches = match_order(ob, buyOrder);
  myassert(matches == 2);

  // sellOrder1 should be fully matched; sellOrder2 partially matched (remaining
  // quantity = 2).
  myassert(order_exists(ob, 4));
  Order order_lookup = lookup_order_by_id(ob, 4);
  myassert(order_lookup.quantity == 2);

  // Modify remaining order partially.
  modify_order_by_id(ob, 4, 1);
  myassert(order_exists(ob, 4));
  order_lookup = lookup_order_by_id(ob, 4);
  myassert(order_lookup.quantity == 1);

  // Fully modify the order.
  modify_order_by_id(ob, 4, 0);
  myassert(!order_exists(ob, 4));

  std::cout << "Test 2 passed." << std::endl;
}

// Test 3: Sell order matching buy orders
void test_sell_order_matching_buy() {
  std::cout << "Test 3: Sell order matching buy orders" << std::endl;
  Orderbook ob;
  // Insert a buy order.
  Order buyOrder{6, 100, 10, Side::BUY};
  uint32_t matches = match_order(ob, buyOrder);
  myassert(matches == 0);

  // A sell order that matches part of the buy order.
  Order sellOrder{7, 100, 4, Side::SELL};
  matches = match_order(ob, sellOrder);
  myassert(matches == 1); // one resting order matched

  // Remaining buy order should have 6 quantity.
  myassert(order_exists(ob, 6));
  Order order_lookup = lookup_order_by_id(ob, 6);
  myassert(order_lookup.quantity == 6);

  // Send another sell order to match the rest of the buy order.
  Order sellOrder2{8, 90, 7, Side::SELL};
  matches = match_order(ob, sellOrder2);
  myassert(matches == 1); // matched with the remaining buy order

  // The buy order should be removed, and sellOrder2 should have 1 unit left.
  myassert(!order_exists(ob, 6));
  myassert(order_exists(ob, 8));
  order_lookup = lookup_order_by_id(ob, 8);
  myassert(order_lookup.quantity == 1);

  std::cout << "Test 3 passed." << std::endl;
}

// Test 4: Full fill buy order exact match
void test_full_fill_buy_order_exact_match() {
  std::cout << "Test 4: Full fill buy order exact match" << std::endl;
  Orderbook ob;
  // Insert a sell order.
  Order sellOrder{20, 100, 10, Side::SELL};
  uint32_t matches = match_order(ob, sellOrder);
  myassert(matches == 0);

  // Insert a buy order that exactly matches the sell order.
  Order buyOrder{21, 100, 10, Side::BUY};
  matches = match_order(ob, buyOrder);
  myassert(matches == 1);

  // Verify that the sell order is fully filled and removed.
  myassert(!order_exists(ob, 20));

  std::cout << "Test 4 passed." << std::endl;
}

// Test 5: Partial fill buy order across multiple sell levels
void test_partial_fill_buy_order_across_multiple_sell_levels() {
  std::cout << "Test 5: Partial fill buy order across multiple sell levels"
            << std::endl;
  Orderbook ob;
  // Insert two sell orders at different prices.
  Order sellOrder1{22, 95, 4, Side::SELL};
  Order sellOrder2{23, 100, 6, Side::SELL};
  match_order(ob, sellOrder1);
  match_order(ob, sellOrder2);

  // Insert a buy order that can match both.
  Order buyOrder{24, 100, 8, Side::BUY};
  uint32_t matches = match_order(ob, buyOrder);
  myassert(matches == 2);

  // sellOrder1 fully matched; sellOrder2 partially filled (remaining = 2).
  myassert(order_exists(ob, 23));
  Order order_lookup = lookup_order_by_id(ob, 23);
  myassert(order_lookup.quantity == 2);

  std::cout << "Test 5 passed." << std::endl;
}

// Test 6: Modify nonexistent order
void test_modify_nonexistent_order() {
  std::cout << "Test 6: Modify nonexistent order" << std::endl;
  Orderbook ob;
  // Insert a buy order.
  Order buyOrder{25, 100, 10, Side::BUY};
  match_order(ob, buyOrder);

  // Attempt to modify an order with a non-existent ID.
  modify_order_by_id(ob, 999, 0);

  // Verify the existing buy order remains unchanged.
  myassert(order_exists(ob, 25));
  Order order_lookup = lookup_order_by_id(ob, 25);
  myassert(order_lookup.id == 25);

  std::cout << "Test 6 passed." << std::endl;
}

// Test 7: Partial modification of a resting order
void test_partial_modification() {
  std::cout << "Test 7: Partial modification of a resting order" << std::endl;
  Orderbook ob;
  // Insert a sell order.
  Order sellOrder{26, 100, 10, Side::SELL};
  match_order(ob, sellOrder);

  // Record initial quantity.
  myassert(order_exists(ob, 26));
  Order order_lookup = lookup_order_by_id(ob, 26);
  int initialQuantity = order_lookup.quantity;
  myassert(initialQuantity == 10);

  // Partially modify (reduces quantity by setting it to 1).
  modify_order_by_id(ob, 26, 1);
  myassert(order_exists(ob, 26));
  order_lookup = lookup_order_by_id(ob, 26);
  myassert(order_lookup.quantity == 1);

  // Fully modify the remaining quantity.
  modify_order_by_id(ob, 26, 0);
  myassert(!order_exists(ob, 26));

  std::cout << "Test 7 passed." << std::endl;
}

// Test 8: Partial fill sell order across multiple buy levels
void test_partial_fill_sell_order_across_multiple_buy_levels() {
  std::cout << "Test 8: Partial fill sell order across multiple buy levels"
            << std::endl;
  Orderbook ob;
  // Insert two buy orders.
  Order buyOrder1{27, 100, 5, Side::BUY};
  Order buyOrder2{28, 95, 5, Side::BUY};
  match_order(ob, buyOrder1);
  match_order(ob, buyOrder2);

  // Insert a sell order that should match both.
  Order sellOrder{29, 90, 7, Side::SELL};
  uint32_t matches = match_order(ob, sellOrder);
  myassert(matches == 2);

  // buyOrder1 fully matched; buyOrder2 partially filled (remaining = 3).
  myassert(order_exists(ob, 28));
  Order order_lookup = lookup_order_by_id(ob, 28);
  myassert(order_lookup.quantity == 3);
  // Verify that the buy order with id 27 is removed.
  myassert(!order_exists(ob, 27));

  std::cout << "Test 8 passed." << std::endl;
}

// Test 9: No match due to exact price mismatch
void test_exact_price_mismatch_no_fill() {
  std::cout << "Test 9: No match due to exact price mismatch" << std::endl;
  Orderbook ob;
  // Insert a sell order with a price that is too high.
  Order sellOrder{30, 105, 5, Side::SELL};
  match_order(ob, sellOrder);

  // Insert a buy order with a lower price.
  Order buyOrder{31, 100, 5, Side::BUY};
  match_order(ob, buyOrder);

  // Both orders should remain in their respective books.
  myassert(order_exists(ob, 30));
  myassert(order_exists(ob, 31));

  std::cout << "Test 9 passed." << std::endl;
}

// Test 10: Multiple partial fills on same price level
void test_multiple_partial_fills_same_level() {
  std::cout << "Test 10: Multiple partial fills on same price level"
            << std::endl;
  Orderbook ob;
  // Insert two sell orders at the same price.
  Order sellOrder1{32, 100, 4, Side::SELL};
  Order sellOrder2{33, 100, 6, Side::SELL};
  match_order(ob, sellOrder1);
  match_order(ob, sellOrder2);

  // Insert a buy order that partially fills both sell orders.
  Order buyOrder{34, 100, 8, Side::BUY};
  uint32_t matches = match_order(ob, buyOrder);
  myassert(matches == 2);

  // sellOrder1 fully matched; sellOrder2 partially filled (remaining = 2).
  myassert(!order_exists(ob, 32));
  myassert(order_exists(ob, 33));
  Order order_lookup = lookup_order_by_id(ob, 33);
  myassert(order_lookup.quantity == 2);

  std::cout << "Test 10 passed." << std::endl;
}

// Test 11: Order book integrity after multiple operations
void test_order_book_integrity_after_multiple_operations() {
  std::cout << "Test 11: Order book integrity after multiple operations"
            << std::endl;
  Orderbook ob;
  // Insert a buy order.
  Order buyOrder{35, 100, 10, Side::BUY};
  match_order(ob, buyOrder);

  // Insert a sell order that partially fills the buy order.
  Order sellOrder1{36, 100, 5, Side::SELL};
  match_order(ob, sellOrder1);
  myassert(order_exists(ob, 35));
  Order order_lookup = lookup_order_by_id(ob, 35);
  myassert(order_lookup.quantity == 5);

  // Insert another sell order at a better price.
  Order sellOrder2{37, 95, 3, Side::SELL};
  match_order(ob, sellOrder2);
  myassert(order_exists(ob, 35));
  order_lookup = lookup_order_by_id(ob, 35);
  myassert(order_lookup.quantity == 2);

  // Modify the remaining buy order.
  modify_order_by_id(ob, 35, 0);
  myassert(!order_exists(ob, 35));

  // Insert a new sell order that remains unmatched.
  Order sellOrder3{38, 100, 2, Side::SELL};
  match_order(ob, sellOrder3);
  myassert(order_exists(ob, 38));

  std::cout << "Test 11 passed." << std::endl;
}

// Test 12: Multiple orders on the same side maintain FIFO order
void test_multiple_orders_same_side_ordering() {
  std::cout << "Test 12: Multiple orders on the same side ordering"
            << std::endl;
  Orderbook ob;
  // Insert two buy orders at the same price.
  Order buyOrder1{39, 100, 5, Side::BUY};
  Order buyOrder2{40, 100, 5, Side::BUY};
  match_order(ob, buyOrder1);
  match_order(ob, buyOrder2);

  // Insert a sell order that should match with the earliest buy order.
  Order sellOrder{41, 95, 3, Side::SELL};
  uint32_t matches = match_order(ob, sellOrder);
  myassert(matches == 1);

  // The first buy order (id 39) should be partially filled (5-3 = 2).
  myassert(order_exists(ob, 39));
  Order order_lookup = lookup_order_by_id(ob, 39);
  myassert(order_lookup.quantity == 2);

  std::cout << "Test 12 passed." << std::endl;
}

// Test 13: Full match sell order exact match
void test_full_match_sell_order_exact_match() {
  std::cout << "Test 13: Full match sell order exact match" << std::endl;
  Orderbook ob;
  // Insert a buy order.
  Order buyOrder{42, 100, 10, Side::BUY};
  match_order(ob, buyOrder);

  // Insert a sell order that exactly matches the buy order.
  Order sellOrder{43, 100, 10, Side::SELL};
  uint32_t matches = match_order(ob, sellOrder);
  myassert(matches == 1);

  // Verify that the buy order is fully filled and removed.
  myassert(!order_exists(ob, 42));

  std::cout << "Test 13 passed." << std::endl;
}

// Test 14: Modify with no change
void test_modify_no_change() {
  std::cout << "Test 14: Modify with no change" << std::endl;
  Orderbook ob;
  Order sellOrder{50, 100, 10, Side::SELL};
  match_order(ob, sellOrder);
  // Modify with the same quantity; order should remain unchanged.
  modify_order_by_id(ob, 50, 10);
  myassert(order_exists(ob, 50));
  Order order_lookup = lookup_order_by_id(ob, 50);
  myassert(order_lookup.quantity == 10);
  std::cout << "Test 14 passed." << std::endl;
}

// Test 15: Modify order after partial fill
void test_modify_after_partial_fill() {
  std::cout << "Test 15: Modify order after partial fill" << std::endl;
  Orderbook ob;
  Order buyOrder{51, 100, 10, Side::BUY};
  match_order(ob, buyOrder);
  Order sellOrder{52, 100, 4, Side::SELL};
  match_order(ob, sellOrder);
  // Resting buy order should now have a quantity of 6.
  myassert(order_exists(ob, 51));
  Order order_lookup = lookup_order_by_id(ob, 51);
  myassert(order_lookup.quantity == 6);
  // Modify the resting buy order to a quantity of 3.
  modify_order_by_id(ob, 51, 3);
  myassert(order_exists(ob, 51));
  order_lookup = lookup_order_by_id(ob, 51);
  myassert(order_lookup.quantity == 3);
  // Insert a sell order that exactly matches the modified buy order.
  Order sellOrder2{53, 90, 3, Side::SELL};
  uint32_t matches = match_order(ob, sellOrder2);
  myassert(matches == 1);
  // The buy order should be fully matched and removed.
  myassert(!order_exists(ob, 51));

  std::cout << "Test 15 passed." << std::endl;
}

// Test 16: Modify preserves FIFO order
void test_modify_preserves_fifo() {
  std::cout << "Test 16: Modify preserves FIFO order" << std::endl;
  Orderbook ob;
  // Insert two sell orders at the same price.
  Order sellOrderA{54, 100, 5, Side::SELL};
  Order sellOrderB{55, 100, 5, Side::SELL};
  match_order(ob, sellOrderA);
  match_order(ob, sellOrderB);
  // Modify the first order's quantity to 3.
  modify_order_by_id(ob, 54, 3);
  // Insert a buy order that partially fills the resting sell orders.
  Order buyOrder{56, 100, 4, Side::BUY};
  uint32_t matches = match_order(ob, buyOrder);
  myassert(matches == 2);
  // sellOrderA (modified) should be fully filled (3), and sellOrderB should be
  // partially filled (remaining = 4).
  myassert(!order_exists(ob, 54));
  myassert(order_exists(ob, 55));
  Order order_lookup = lookup_order_by_id(ob, 55);
  myassert(order_lookup.quantity == 4);

  std::cout << "Test 16 passed." << std::endl;
}

// Test 17: Multiple modifications on the same order
void test_multiple_modifications() {
  std::cout << "Test 17: Multiple modifications on the same order" << std::endl;
  Orderbook ob;
  Order buyOrder{57, 100, 12, Side::BUY};
  match_order(ob, buyOrder);
  // First modification: reduce quantity to 8.
  modify_order_by_id(ob, 57, 8);
  myassert(order_exists(ob, 57));
  Order order_lookup = lookup_order_by_id(ob, 57);
  myassert(order_lookup.quantity == 8);
  // Second modification: reduce quantity to 5.
  modify_order_by_id(ob, 57, 5);
  myassert(order_exists(ob, 57));
  order_lookup = lookup_order_by_id(ob, 57);
  myassert(order_lookup.quantity == 5);
  // Insert a sell order that exactly matches the final modified quantity.
  Order sellOrder{58, 100, 5, Side::SELL};
  uint32_t matches = match_order(ob, sellOrder);
  myassert(matches == 1);
  // The buy order should now be fully matched and removed.
  myassert(!order_exists(ob, 57));

  std::cout << "Test 17 passed." << std::endl;
}

// Test 18: Modify with quantity=0 removes the order entirely
void test_modify_with_zero_removes_order() {
  std::cout << "Test 18: Modify with quantity=0 removes the order entirely"
            << std::endl;
  Orderbook ob;
  // Insert a buy order.
  Order buyOrder{60, 100, 10, Side::BUY};
  match_order(ob, buyOrder);

  // Modify the order to set its quantity to 0, expecting it to be removed.
  modify_order_by_id(ob, 60, 0);

  // Verify that the order is removed.
  myassert(!order_exists(ob, 60));

  std::cout << "Test 18 passed." << std::endl;
}

// Test 19: Get volume at level with no orders.
void test_get_volume_no_orders() {
  std::cout << "Test 19: Get volume at level with no orders" << std::endl;
  Orderbook ob;
  uint32_t volume_buy = get_volume_at_level(ob, Side::BUY, 100);
  uint32_t volume_sell = get_volume_at_level(ob, Side::SELL, 100);
  myassert(volume_buy == 0);
  myassert(volume_sell == 0);
  std::cout << "Test 19 passed." << std::endl;
}

// Test 20: Get volume at level with a single order.
void test_get_volume_single_order() {
  std::cout << "Test 20: Get volume at level with a single order" << std::endl;
  Orderbook ob;
  Order sellOrder{100, 100, 10, Side::SELL};
  match_order(ob, sellOrder);
  uint32_t volume_sell = get_volume_at_level(ob, Side::SELL, 100);
  myassert(volume_sell == 10);
  uint32_t volume_buy = get_volume_at_level(ob, Side::BUY, 100);
  myassert(volume_buy == 0);
  std::cout << "Test 20 passed." << std::endl;
}

// Test 21: Get volume at level with multiple orders at the same price.
void test_get_volume_multiple_orders_same_level() {
  std::cout
      << "Test 21: Get volume at level with multiple orders at the same price"
      << std::endl;
  Orderbook ob;
  Order sellOrder1{101, 100, 5, Side::SELL};
  Order sellOrder2{102, 100, 7, Side::SELL};
  match_order(ob, sellOrder1);
  match_order(ob, sellOrder2);
  uint32_t volume_sell = get_volume_at_level(ob, Side::SELL, 100);
  myassert(volume_sell == 12); // 5 + 7 = 12
  std::cout << "Test 21 passed." << std::endl;
}

// Test 22: Get volume at level with orders at different price levels.
void test_get_volume_orders_different_levels() {
  std::cout
      << "Test 22: Get volume at level with orders at different price levels"
      << std::endl;
  Orderbook ob;
  Order buyOrder1{103, 100, 10, Side::BUY};
  Order buyOrder2{104, 101, 5, Side::BUY};
  match_order(ob, buyOrder1);
  match_order(ob, buyOrder2);
  uint32_t volume_buy_100 = get_volume_at_level(ob, Side::BUY, 100);
  uint32_t volume_buy_101 = get_volume_at_level(ob, Side::BUY, 101);
  myassert(volume_buy_100 == 10);
  myassert(volume_buy_101 == 5);
  std::cout << "Test 22 passed." << std::endl;
}

// Test 23: Get volume at level after partial fill.
void test_get_volume_after_partial_fill() {
  std::cout << "Test 23: Get volume at level after partial fill" << std::endl;
  Orderbook ob;
  Order sellOrder{105, 100, 10, Side::SELL};
  match_order(ob, sellOrder);
  // Insert a buy order that partially fills the sell order.
  Order buyOrder{106, 100, 4, Side::BUY};
  match_order(ob, buyOrder);
  // The sell order should now have 6 remaining.
  uint32_t volume_sell = get_volume_at_level(ob, Side::SELL, 100);
  myassert(volume_sell == 6);
  std::cout << "Test 23 passed." << std::endl;
}

// Test 24: Get volume at level after order cancellation.
void test_get_volume_after_cancellation() {
  std::cout << "Test 24: Get volume at level after order cancellation"
            << std::endl;
  Orderbook ob;
  Order buyOrder{107, 100, 10, Side::BUY};
  match_order(ob, buyOrder);
  uint32_t volume_buy = get_volume_at_level(ob, Side::BUY, 100);
  myassert(volume_buy == 10);
  // Cancel the order by setting its quantity to 0.
  modify_order_by_id(ob, 107, 0);
  volume_buy = get_volume_at_level(ob, Side::BUY, 100);
  myassert(volume_buy == 0);
  std::cout << "Test 24 passed." << std::endl;
}

// Test 25: Complex scenario with multiple SELL orders and modifications.
void test_get_volume_complex1() {
  std::cout
      << "Test 25: Complex scenario with multiple SELL orders and modifications"
      << std::endl;
  Orderbook ob;
  // Insert SELL orders at price levels 100 and 101.
  Order sellOrder1{200, 100, 10, Side::SELL};
  Order sellOrder2{201, 100, 20, Side::SELL};
  Order sellOrder3{202, 101, 15, Side::SELL};
  match_order(ob, sellOrder1);
  match_order(ob, sellOrder2);
  match_order(ob, sellOrder3);
  // Modify sellOrder1: reduce quantity to 5.
  modify_order_by_id(ob, 200, 5);
  // Check volume at level 100 for SELL: 5 + 20 = 25.
  uint32_t volume_100 = get_volume_at_level(ob, Side::SELL, 100);
  myassert(volume_100 == 25);
  // Check volume at level 101 for SELL.
  uint32_t volume_101 = get_volume_at_level(ob, Side::SELL, 101);
  myassert(volume_101 == 15);
  std::cout << "Test 25 passed." << std::endl;
}

// Test 26: Complex scenario with interleaved BUY order insertions, matches, and
// modifications.
void test_get_volume_complex2() {
  std::cout << "Test 26: Complex scenario with interleaved BUY order "
               "insertions, matches, and modifications"
            << std::endl;
  Orderbook ob;
  // Insert BUY orders at price 100.
  Order buyOrder1{300, 100, 20, Side::BUY};
  Order buyOrder2{301, 100, 10, Side::BUY};
  match_order(ob, buyOrder1);
  match_order(ob, buyOrder2);
  // Total BUY volume at price 100 should be 20 + 10 = 30.
  uint32_t volume_buy = get_volume_at_level(ob, Side::BUY, 100);
  myassert(volume_buy == 30);
  // Insert a SELL order that partially fills the earliest BUY order.
  Order sellOrder{302, 100, 15, Side::SELL};
  match_order(ob, sellOrder);
  // After matching, buyOrder1 reduces from 20 to 5, buyOrder2 remains at 10,
  // total becomes 15.
  volume_buy = get_volume_at_level(ob, Side::BUY, 100);
  myassert(volume_buy == 15);
  // Modify buyOrder2 to reduce its quantity to 5.
  modify_order_by_id(ob, 301, 5);
  // Now total volume is 5 (buyOrder1) + 5 (buyOrder2) = 10.
  volume_buy = get_volume_at_level(ob, Side::BUY, 100);
  myassert(volume_buy == 10);
  std::cout << "Test 26 passed." << std::endl;
}

// Test 27: Complex scenario with cancellations and new insertions on SELL side.
void test_get_volume_complex3() {
  std::cout << "Test 27: Complex scenario with cancellations and new "
               "insertions on SELL side"
            << std::endl;
  Orderbook ob;
  // Insert SELL orders at price 100.
  Order sellOrder1{400, 100, 30, Side::SELL};
  Order sellOrder2{401, 100, 20, Side::SELL};
  match_order(ob, sellOrder1);
  match_order(ob, sellOrder2);
  // Total volume at price 100 for SELL should be 30 + 20 = 50.
  uint32_t volume_sell = get_volume_at_level(ob, Side::SELL, 100);
  myassert(volume_sell == 50);
  // Cancel sellOrder1.
  modify_order_by_id(ob, 400, 0);
  volume_sell = get_volume_at_level(ob, Side::SELL, 100);
  myassert(volume_sell == 20);
  // Insert a new SELL order at price 100.
  Order sellOrder3{402, 100, 15, Side::SELL};
  match_order(ob, sellOrder3);
  volume_sell = get_volume_at_level(ob, Side::SELL, 100);
  myassert(volume_sell == 35); // 20 + 15 = 35.
  // Insert SELL order at price 101.
  Order sellOrder4{403, 101, 10, Side::SELL};
  match_order(ob, sellOrder4);
  uint32_t volume_sell_101 = get_volume_at_level(ob, Side::SELL, 101);
  myassert(volume_sell_101 == 10);
  // Modify sellOrder2 to reduce its quantity to 10.
  modify_order_by_id(ob, 401, 10);
  volume_sell = get_volume_at_level(ob, Side::SELL, 100);
  myassert(volume_sell == 25); // 10 (modified) + 15 = 25.
  std::cout << "Test 27 passed." << std::endl;
}

// Test 28: Comprehensive scenario encompassing both BUY and SELL sides.
void test_get_volume_all_encompassing() {
  std::cout
      << "Test 28: Comprehensive scenario encompassing both BUY and SELL sides"
      << std::endl;
  Orderbook ob;
  // Insert BUY orders.
  Order buyOrder1{500, 100, 20, Side::BUY};
  Order buyOrder2{501, 100, 15, Side::BUY};
  Order buyOrder3{502, 99, 10, Side::BUY};
  match_order(ob, buyOrder1);
  match_order(ob, buyOrder2);
  match_order(ob, buyOrder3);

  // Insert SELL orders that will trigger matching on crossing prices.
  Order sellOrder1{503, 100, 25,
                   Side::SELL}; // This should match with buyOrder1 and
                                // partially with buyOrder2.
  match_order(ob, sellOrder1);
  // After matching:
  // - buyOrder1 is fully filled (20 out of 25 matched).
  // - buyOrder2 is partially filled: 15 - (25-20=5) = 10 remaining.
  // - buyOrder3 remains untouched.

  // Insert SELL orders that do not cross.
  Order sellOrder2{
      504, 102, 30,
      Side::SELL}; // No match because BUY orders are at 100 and 99.
  Order sellOrder3{505, 101, 10,
                   Side::SELL}; // No match because BUY orders are below 101.
  match_order(ob, sellOrder2);
  match_order(ob, sellOrder3);

  // Modify orders.
  // Reduce buyOrder3 from 10 to 5.
  modify_order_by_id(ob, 502, 5);
  // Modify sellOrder2 from 30 to 20.
  modify_order_by_id(ob, 504, 20);

  // Insert a BUY order that crosses with SELL orders.
  Order buyOrder4{506, 102, 15, Side::BUY};
  match_order(ob, buyOrder4);
  // This order at 102 will match with SELL orders at the best available price.
  // It should first match with sellOrder3 (price 101, 10 units) then with
  // sellOrder2 (price 102, 5 units). sellOrder3 gets fully filled, and
  // sellOrder2 reduces from 20 to 15.

  // Final expected resting volumes:
  // BUY side:
  //   Price 100: buyOrder2 with 10 units remaining.
  //   Price 99: buyOrder3 with 5 units.
  // SELL side:
  //   Price 102: sellOrder2 with 15 units remaining.
  //   Price 101: sellOrder3 should be gone.

  uint32_t volume_buy_100 = get_volume_at_level(ob, Side::BUY, 100);
  uint32_t volume_buy_99 = get_volume_at_level(ob, Side::BUY, 99);
  uint32_t volume_sell_102 = get_volume_at_level(ob, Side::SELL, 102);
  uint32_t volume_sell_101 = get_volume_at_level(ob, Side::SELL, 101);

  myassert(volume_buy_100 == 10);
  myassert(volume_buy_99 == 5);
  myassert(volume_sell_102 == 15);
  myassert(volume_sell_101 == 0);

  std::cout << "Test 28 passed." << std::endl;
}

int main() {
  test_lookup_order();
  test_simple_match_and_modify();
  test_multiple_matches();
  test_sell_order_matching_buy();
  test_full_fill_buy_order_exact_match();
  test_partial_fill_buy_order_across_multiple_sell_levels();
  test_modify_nonexistent_order();
  test_partial_modification();
  test_partial_fill_sell_order_across_multiple_buy_levels();
  test_exact_price_mismatch_no_fill();
  test_multiple_partial_fills_same_level();
  test_order_book_integrity_after_multiple_operations();
  test_multiple_orders_same_side_ordering();
  test_full_match_sell_order_exact_match();
  test_modify_no_change();
  test_modify_after_partial_fill();
  test_modify_preserves_fifo();
  test_multiple_modifications();
  test_modify_with_zero_removes_order();
  test_get_volume_no_orders();
  test_get_volume_single_order();
  test_get_volume_multiple_orders_same_level();
  test_get_volume_orders_different_levels();
  test_get_volume_after_partial_fill();
  test_get_volume_after_cancellation();
  test_get_volume_complex1();
  test_get_volume_complex2();
  test_get_volume_complex3();
  test_get_volume_all_encompassing();
  std::cout << "All tests passed." << std::endl;
  return 0;
}

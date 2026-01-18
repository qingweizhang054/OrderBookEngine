#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <cmath>

#include "../Headers/MatchingEngine.h"
#include "../Headers/OrderBook.h"


/**
 * Generates synthetic test orders with randomized prices and quantities.
 * Fixes the logic bug where hardcoded prices prevented realistic matching scenarios.
 */
std::vector<Order> generateOrders(int count) {
    std::vector<Order> orders;
    orders.reserve(count); // Prevent multiple reallocations for better performance

    // Initialize random number generator with current time seed
    unsigned seed = std::chrono::steady_clock::now().time_since_epoch().count();
    std::mt19937 rng(seed);

    // 1. Distribution for Order Side (50% BUY, 50% SELL)
    std::uniform_int_distribution<int> sideDist(0, 1);

    // 2. Distribution for Price (Normal distribution around 100.0 with 2.0 std deviation)
    // This creates price overlaps (crosses) to trigger the matching engine
    std::normal_distribution<double> priceDist(100.0, 2.0);

    // 3. Distribution for Quantity (Random integer between 10 and 50)
    std::uniform_int_distribution<int> qtyDist(10, 50);

    for (int i = 0; i < count; ++i) {
        OrderSide side = (sideDist(rng) == 0) ? OrderSide::BUY : OrderSide::SELL;
        
        // Generate price and round to 2 decimal places
        double rawPrice = priceDist(rng);
        double price = std::round(rawPrice * 100.0) / 100.0;

        int quantity = qtyDist(rng);

        // Construct the order object
        orders.emplace_back(
            "order_" + std::to_string(i), 
            side, 
            price, 
            quantity
        );
    }

    return orders;
}


int main() {
  const int num_orders = 20000;

  MatchingEngine engine;
  auto& order_book = engine.getOrderBook();
  auto orders = generateOrders(num_orders);

  std::cout << "[INFO] Adding orders...\n";
  for (const auto& order : orders) {
    order_book.addOrder(order);
  }

  std::cout << "[INFO] Starting match...\n";
  auto start = std::chrono::high_resolution_clock::now();
  auto trades = engine.match();
  std::cout << "[DEBUG] match() returned trades.size(): " << trades.size()
            << "\n";

  auto end = std::chrono::high_resolution_clock::now();
  auto total_us =
      std::chrono::duration_cast<std::chrono::microseconds>(end - start)
          .count();

  std::cout << "✅ Matched " << num_orders << " orders in " << total_us
            << " μs\n";
  std::cout << "⚡ Throughput: " << (num_orders * 1'000'000LL / total_us)
            << " orders/sec\n";
  std::cout << "📈 Trades executed: " << trades.size() << "\n";

  return 0;
}

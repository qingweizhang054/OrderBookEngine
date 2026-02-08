#include "MatchingEngine.h"

MatchingEngine::MatchingEngine(OrderBook &book)
: book_(book) {}

void MatchingEngine::execute(const Order &order) {
    book_.addOrder(order);
    book_.matchOrders();
}

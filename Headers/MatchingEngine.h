#pragma once

#include "OrderBook.h"

class MatchingEngine {
public:
    explicit MatchingEngine(OrderBook &book);

    void execute(const Order &order);

private:
    OrderBook &book_;
};

#pragma once

#include "OrderBook.h"
#include "MatchingEngine.h"
#include "concurrentqueue.h"
#include <thread>
#include <atomic>

class OrderDispatcher {
public:
    explicit OrderDispatcher(OrderBook &book);
    ~OrderDispatcher();

    void start();
    void stop();
    void pushOrder(const Order &order);

private:
    void run();

    moodycamel::ConcurrentQueue<Order> queue_;
    std::thread matchingThread_;
    std::atomic<bool> running_;
    MatchingEngine engine_;
};

#include "OrderDispatcher.h"
#include <chrono>

OrderDispatcher::OrderDispatcher(OrderBook &book)
: engine_(book), running_(false) {}

OrderDispatcher::~OrderDispatcher() {
    stop();
}

void OrderDispatcher::start() {
    running_.store(true, std::memory_order_release);
    matchingThread_ = std::thread(&OrderDispatcher::run, this);
}

void OrderDispatcher::stop() {
    running_.store(false, std::memory_order_release);
    if (matchingThread_.joinable()) {
        matchingThread_.join();
    }
}

void OrderDispatcher::pushOrder(const Order &order) {
    queue_.enqueue(order);
}

void OrderDispatcher::run() {
    Order order;
    while (running_.load(std::memory_order_acquire) || queue_.size_approx() > 0) {
        while (queue_.try_dequeue(order)) {
            engine_.execute(order);
        }
        std::this_thread::sleep_for(std::chrono::microseconds(5));
    }
}

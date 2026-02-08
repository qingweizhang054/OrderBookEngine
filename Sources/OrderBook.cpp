#include "OrderBook.h"

bool OrderBook::addOrder(const Order &order) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (buy_lookup_.count(order.id) || sell_lookup_.count(order.id))
        return false;

    if (order.side == OrderSide::BUY) {
        auto it = buy_orders_.insert(order).first;
        buy_lookup_[order.id] = it;
    } else {
        auto it = sell_orders_.insert(order).first;
        sell_lookup_[order.id] = it;
    }
    return true;
}

bool OrderBook::cancelOrder(const std::string &order_id) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto ib = buy_lookup_.find(order_id);
    if (ib != buy_lookup_.end()) {
        buy_orders_.erase(ib->second);
        buy_lookup_.erase(ib);
        return true;
    }
    auto is = sell_lookup_.find(order_id);
    if (is != sell_lookup_.end()) {
        sell_orders_.erase(is->second);
        sell_lookup_.erase(is);
        return true;
    }
    return false;
}

bool OrderBook::modifyOrder(const std::string &order_id, double new_price,
                            int new_quantity) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto ib = buy_lookup_.find(order_id);
    if (ib != buy_lookup_.end()) {
        Order old = *ib->second;
        if (old.type == OrderType::MARKET)
            return false;
        buy_orders_.erase(ib->second);
        buy_lookup_.erase(order_id);

        Order updated(order_id, old.side, new_price, new_quantity, old.type,
                      old.timestamp);
        auto it = buy_orders_.insert(updated).first;
        buy_lookup_[order_id] = it;
        return true;
    }

    auto is = sell_lookup_.find(order_id);
    if (is != sell_lookup_.end()) {
        Order old = *is->second;
        if (old.type == OrderType::MARKET)
            return false;
        sell_orders_.erase(is->second);
        sell_lookup_.erase(order_id);

        Order updated(order_id, old.side, new_price, new_quantity, old.type,
                      old.timestamp);
        auto it = sell_orders_.insert(updated).first;
        sell_lookup_[order_id] = it;
        return true;
    }

    return false;
}

void OrderBook::matchOrders() {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    while (!buy_orders_.empty() && !sell_orders_.empty()) {
        auto ibid = buy_orders_.begin();
        auto iask = sell_orders_.begin();

        Order bid = *ibid;
        Order ask = *iask;

        double exec_price = 0.0;
        bool can_match = false;

        if (bid.type == OrderType::MARKET) {
            exec_price = ask.price;
            can_match = true;
        } else if (ask.type == OrderType::MARKET) {
            exec_price = bid.price;
            can_match = true;
        } else if (bid.price >= ask.price) {
            exec_price = ask.price;
            can_match = true;
        }

        if (!can_match) break;

        int qty = std::min(bid.quantity, ask.quantity);
        Trade trade{bid.id, ask.id, qty, exec_price,
                    std::chrono::steady_clock::now()};
        reportTrade(trade);

        // Remove bid / adjust
        buy_orders_.erase(ibid);
        buy_lookup_.erase(bid.id);
        if (bid.quantity > qty) {
            Order leftover_bid{bid.id, bid.side, bid.price,
                               bid.quantity - qty, bid.type,
                               std::chrono::steady_clock::now()};
            auto new_bid_it = buy_orders_.insert(leftover_bid).first;
            buy_lookup_[leftover_bid.id] = new_bid_it;
        }

        // Remove ask / adjust
        sell_orders_.erase(iask);
        sell_lookup_.erase(ask.id);
        if (ask.quantity > qty) {
            Order leftover_ask{ask.id, ask.side, ask.price,
                               ask.quantity - qty, ask.type,
                               std::chrono::steady_clock::now()};
            auto new_ask_it =
                sell_orders_.insert(leftover_ask).first;
            sell_lookup_[leftover_ask.id] = new_ask_it;
        }
    }
}

std::optional<Order> OrderBook::getTopBid() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (buy_orders_.empty())
        return std::nullopt;
    return *buy_orders_.begin();
}

std::optional<Order> OrderBook::getTopAsk() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (sell_orders_.empty())
        return std::nullopt;
    return *sell_orders_.begin();
}

void OrderBook::setTradeCallback(TradeCallback cb) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    trade_callback_ = std::move(cb);
}

const std::vector<Trade> &OrderBook::getExecutedTrades() const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return executed_trades_;
}

void OrderBook::reportTrade(const Trade &t) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    executed_trades_.push_back(t);
    if (trade_callback_)
        trade_callback_(t);
}

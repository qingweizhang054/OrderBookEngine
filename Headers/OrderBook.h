#pragma once

#include <chrono>
#include <functional>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

enum class OrderSide { BUY, SELL };
enum class OrderType { LIMIT, MARKET };

struct Order {
    std::string id;
    OrderSide side;
    double price;
    OrderType type;
    int quantity;
    std::chrono::steady_clock::time_point timestamp;

    Order() = default;  


    Order(const std::string &id_, OrderSide side_, double price_, int quantity_,
          OrderType type_ = OrderType::LIMIT,
          std::chrono::steady_clock::time_point ts =
              std::chrono::steady_clock::now())
        : id(id_), side(side_), price(price_), type(type_), quantity(quantity_),
          timestamp(ts) {}
};

struct Trade {
    std::string buy_order_id;
    std::string sell_order_id;
    int quantity;
    double price;
    std::chrono::steady_clock::time_point timestamp;
};

struct BidComparator {
    bool operator()(const Order &a, const Order &b) const {
        if (a.type != b.type)
            return a.type == OrderType::MARKET;
        if (a.price != b.price)
            return a.price > b.price;
        return a.timestamp < b.timestamp;
    }
};

struct AskComparator {
    bool operator()(const Order &a, const Order &b) const {
        if (a.type != b.type)
            return a.type == OrderType::MARKET;
        if (a.price != b.price)
            return a.price < b.price;
        return a.timestamp < b.timestamp;
    }
};

class OrderBook {
public:
    using TradeCallback = std::function<void(const Trade &)>;

    bool addOrder(const Order &order);
    bool cancelOrder(const std::string &order_id);
    bool modifyOrder(const std::string &order_id, double new_price,
                     int new_quantity);

    void matchOrders();

    std::optional<Order> getTopBid() const;
    std::optional<Order> getTopAsk() const;

    void setTradeCallback(TradeCallback cb);

    const std::vector<Trade> &getExecutedTrades() const;

private:
    void reportTrade(const Trade &t);

    std::unordered_map<std::string,
                       std::set<Order, BidComparator>::iterator>
        buy_lookup_;
    std::unordered_map<std::string,
                       std::set<Order, AskComparator>::iterator>
        sell_lookup_;
    std::set<Order, BidComparator> buy_orders_;
    std::set<Order, AskComparator> sell_orders_;

    std::vector<Trade> executed_trades_;
    TradeCallback trade_callback_;

    mutable std::recursive_mutex mutex_;
};

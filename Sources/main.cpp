#include "OrderDispatcher.h"
#include "OrderBook.h"
#include <iostream>

int main() {
    OrderBook book;
    OrderDispatcher dispatcher(book);

    dispatcher.start();

    dispatcher.pushOrder({"o1", OrderSide::BUY, 100.0, 10});
    dispatcher.pushOrder({"o2", OrderSide::SELL, 99.0, 5});
    dispatcher.pushOrder({"o3", OrderSide::SELL, 101.0, 10});

    // let dispatcher finish
    dispatcher.stop();

    for (auto &trade : book.getExecutedTrades()) {
        std::cout << "Trade: " << trade.buy_order_id << " vs "
                  << trade.sell_order_id << " qty " << trade.quantity
                  << " price " << trade.price << "\n";
    }

    return 0;
}

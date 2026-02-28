#ifndef TYPES_H
#define TYPES_H

// MARKET: Sell or Buy at whatever the market price is, the price isn't guaranteed but execution is guaranteed
// STOP: Sell or Buy at market price whenever the stock has been traded through a certain price. price not guaranteed
// LIMIT: Sell or Buy at the stated price or better.

#include <cstdint>

enum class order_type { MARKET_ORDER, STOP_ORDER, LIMIT_ORDER };

enum class side : std::int8_t { BUY = 1, SELL = -1 };

namespace order {

// MARKET QUANTITIES
using price_t = std::int64_t;
using qty_t = std::uint32_t;

// IDENTIFIERS
using order_id = std::uint64_t;

// ORDER
struct order {
  order_id id{};
  side side{};
  order_type type{};
  price_t price{};
  qty_t qty{};
};

}  // namespace order

#endif  // !ORDERS_H

//#include <random>

#include "common/types.h"

extern std::atomic<bool> running;

uint64_t stalls = 0;

static inline uint64_t tsc_producer() {
  unsigned lo, hi;
  asm volatile("rdtsc" : "=a"(lo), "=d"(hi));
  return ((uint64_t)hi << 32) | lo;
}

void producer(order::OrderQueue& queue) {
  // std::mt19937 rng(std::random_device{}());
  //
  // std::uniform_int_distribution<int> side_dist(0, 1);
  // std::uniform_int_distribution<int> price_dist(9900, 10100);
  // std::uniform_int_distribution<int> qty_dist(1, 20);
  //
  order::order_id id = 0;

  while (running) {
    queue.blockPush(
        [&](order::Order* slot) {
          slot->id = id++;

          //slot->side = side_dist(rng) ? Side::BUY : Side::SELL;
          slot->side = Side::BUY;

          slot->type = order_type::LIMIT_ORDER;

          slot->price = 10000;

          slot->qty = 1;

          slot->enq_tsc = tsc_producer();
        },
        stalls);
  }
}

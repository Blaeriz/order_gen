#include <immintrin.h>
#include <atomic>
#include <chrono>
#include <iostream>

#include "common/types.h"

extern std::atomic<bool> running;

void consumer(order::OrderQueue& queue) {
  using clock = std::chrono::steady_clock;

  uint64_t processed = 0;
  uint64_t total = 0;

  uint64_t stalls = 0;
  uint64_t last_stalls = 0;

  auto last = clock::now();

  while (running) {
    bool did_work = false;

    // Drain queue
    while (queue.tryPop([](order::Order*) {})) {
      processed++;
      total++;
      did_work = true;
    }

    if (!did_work) {
      stalls++;
      _mm_pause();
    }

    auto now = clock::now();

    if (now - last >= std::chrono::seconds(1)) {
      std::cout << "Throughput: " << processed << " orders/sec | "
                << "Consumer stalls/sec: " << (stalls - last_stalls) << " | "
                << "Total: " << total << std::endl;

      processed = 0;
      last_stalls = stalls;
      last = now;
    }
  }

  std::cout << "Consumer exiting. Total processed: " << total
            << " | Total stalls: " << stalls << std::endl;
}

#include <immintrin.h>
#include <atomic>
#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>

#include "common/types.h"

extern std::atomic<bool> running;

bool measure_latency = false;

uint32_t sample_counter = 4096;

constexpr int HIST_SIZE = 64;
uint64_t histogram[HIST_SIZE] = {0};
uint64_t samples = 0;

uint64_t percentile(uint64_t hist[], uint64_t total, double p) {
  uint64_t target = total * p;
  uint64_t cumulative = 0;

  for (int i = 0; i < HIST_SIZE; ++i) {
    cumulative += hist[i];
    if (cumulative >= target)
      return (1ULL << i) + (1ULL << (i - 1));
  }

  return 0;
}

static inline uint64_t tsc_consumer() {
  unsigned lo, hi;
  asm volatile("rdtscp" : "=a"(lo), "=d"(hi)::"rcx");
  return ((uint64_t)hi << 32) | lo;
}

void consumer(order::OrderQueue& queue) {
  using clock = std::chrono::steady_clock;

  std::vector<uint64_t> latencies;
  latencies.reserve(1000000);

  uint64_t processed = 0;
  uint64_t total = 0;

  uint64_t stalls = 0;
  uint64_t last_stalls = 0;

  auto last = clock::now();

  while (running) {
    bool did_work = false;

    // Drain queue
    int drained = 0;
    while (drained < 1024 && queue.tryPop([&](order::Order* o) {
      if (measure_latency) {
        if (--sample_counter == 0) {
          sample_counter = 4096;

          uint64_t latency = tsc_consumer() - o->enq_tsc;

          int bucket = 63 - __builtin_clzll(latency | 1);
          if (bucket >= HIST_SIZE)
            bucket = HIST_SIZE - 1;

          histogram[bucket]++;
          samples++;
        }
      }
      processed++;
      total++;
      drained++;
      did_work = true;
    }))

      if (!did_work) {
        stalls++;
        _mm_pause();
      }

    auto now = clock::now();

    if (now - last >= std::chrono::seconds(1)) {
      if (samples > 0) {
        uint64_t p50 = percentile(histogram, samples, 0.50);
        uint64_t p99 = percentile(histogram, samples, 0.99);
        uint64_t p999 = percentile(histogram, samples, 0.999);

        std::cout << "Latency cycles | "
                  << "p50: " << p50 << " p99: " << p99 << " p999: " << p999
                  << std::endl;

        std::memset(histogram, 0, sizeof(histogram));
        samples = 0;
      }
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

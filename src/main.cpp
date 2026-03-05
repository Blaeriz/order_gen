#include <atomic>
#include <csignal>
#include <iostream>
#include <thread>

#include "common/types.h"

std::atomic<bool> running{true};
extern bool measure_latency;

void producer(order::OrderQueue&);
void consumer(order::OrderQueue&);

void signal_handler(int) {
  running = false;
}

int main(int argc, char** argv) {
  signal(SIGINT, signal_handler);
  if (argc > 1 && std::string(argv[1]) == "latency") {
    measure_latency = true;
    std::cout << "Mode: latency benchmark\n";
  } else {
    measure_latency = false;
    std::cout << "Mode: throughput benchmark\n";
  }
  order::OrderQueue queue;

  std::thread prod(producer, std::ref(queue));
  std::thread cons(consumer, std::ref(queue));

  std::cout << "Running benchmark... press Ctrl+C to stop\n";

  prod.join();
  cons.join();

  std::cout << "Shutdown complete\n";
}

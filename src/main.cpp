#include <atomic>
#include <csignal>
#include <iostream>
#include <thread>

#include "common/types.h"

std::atomic<bool> running{true};

void producer(order::OrderQueue&);
void consumer(order::OrderQueue&);

void signal_handler(int) {
  running = false;
}

int main() {
  signal(SIGINT, signal_handler);

  order::OrderQueue queue;

  std::thread prod(producer, std::ref(queue));
  std::thread cons(consumer, std::ref(queue));

  std::cout << "Running benchmark... press Ctrl+C to stop\n";

  prod.join();
  cons.join();

  std::cout << "Shutdown complete\n";
}

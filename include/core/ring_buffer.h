#ifndef RING_BUFFER_H
#define RING_BUFFER_H

//#include <cstddef>
#include <immintrin.h>
#include <atomic>
#include <cstdint>
#include <type_traits>

namespace rb {

template <typename T, std::uint32_t Capacity>
class spsc_ring_buffer {
  static_assert((Capacity & (Capacity - 1)) == 0,
                "Capacity must be power of two.");
  static_assert(std::is_trivially_copyable_v<T>,
                "Should be trivially copyable for lock free transport.");

 public:
  spsc_ring_buffer() noexcept = default;
  spsc_ring_buffer(const spsc_ring_buffer&) = delete;
  spsc_ring_buffer& operator=(const spsc_ring_buffer&) = delete;

  // Producer

  T* alloc(std::uint32_t& write) noexcept {
    write = write_idx_.load(std::memory_order_relaxed);

    if (write - read_idx_cache_ == Capacity) {
      read_idx_cache_ = read_idx_.load(std::memory_order_acquire);

      if (__builtin_expect(write - read_idx_cache_ == Capacity, 0)) {
        return nullptr;  // FULL
      }
    }

    return &data_[write & mask_];
  }

  void push(std::uint32_t write) noexcept {
    write = write_idx_.load(std::memory_order_relaxed);

    write_idx_.store(write + 1, std::memory_order_release);
  }

  template <typename Producer>
  bool tryPush(Producer producer) noexcept {
    std::uint32_t write;

    T* slot = alloc(write);
    if (!slot)
      return false;

    producer(slot);
    push(write);
    return true;
  }

  template <typename Producer>
  void blockPush(Producer producer, uint64_t& stalls) noexcept {
    while (!tryPush(producer)) {

      stalls++;

#if defined(__x86_64__) || defined(_M_X64)
      _mm_pause();
#else
      std::this_thread::yield();
#endif
    }
  }

  // Consumer

  T* front() noexcept {
    const std::uint32_t read = read_idx_.load(std::memory_order_relaxed);

    if (read == write_idx_.load(std::memory_order_acquire)) {
      return nullptr;
    }

    return &data_[read & mask_];
  }

  void pop() noexcept {
    const std::uint32_t read = read_idx_.load(std::memory_order_relaxed);

    read_idx_.store(read + 1, std::memory_order_release);
  }

  template <typename Consumer>
  bool tryPop(Consumer consumer) noexcept {
    T* slot = front();
    if (!slot)
      return false;

    consumer(slot);
    pop();
    return true;
  }

  bool empty() const noexcept {
    return read_idx_.load(std::memory_order_acquire) ==
           write_idx_.load(std::memory_order_acquire);
  }

 private:
  static constexpr std::uint32_t mask_ = Capacity - 1;
  static constexpr std::size_t CACHELINE = 64;

  // BUFFER
  alignas(CACHELINE) T data_[Capacity]{};

  // Producer
  alignas(CACHELINE) std::atomic<std::uint32_t> write_idx_{0};
  std::uint32_t read_idx_cache_{0};  // local to producer

  // Consumer
  alignas(CACHELINE) std::atomic<std::uint32_t> read_idx_{0};
};
}  // namespace rb

#endif

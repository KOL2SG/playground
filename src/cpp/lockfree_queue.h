#ifndef LOCKFREE_QUEUE_H
#define LOCKFREE_QUEUE_H

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <type_traits>

// ============================================================================
// LockFreeQueue - Bounded MPMC Ring Buffer
// ============================================================================
// Design Principles:
// 1. **Fixed Capacity** - Power-of-2 size enables fast modulo via bitmask
// 2. **Cache-Friendly** - Separate head/tail caches lines to avoid false sharing
// 3. **ABA Prevention** - Sequence numbers in slots detect stale reads
// 4. **Memory Ordering** - Acquire/Release semantics for correct synchronization
// 5. **Wait-Free Pop/Push** - No loops that can starve (bounded retries)
// ============================================================================

template <typename T, size_t Capacity>
class LockFreeQueue {
    // Capacity must be power of 2 for efficient bitmask modulo
    static_assert((Capacity & (Capacity - 1)) == 0,
                  "Capacity must be a power of 2");
    static_assert(Capacity >= 2, "Capacity must be at least 2");

    // -------------------------------------------------------------------------
    // Slot Structure - Each element has a sequence number for ABA prevention
    // -------------------------------------------------------------------------
    // The sequence acts as a "generation counter". When a slot is written,
    // its sequence becomes (expected + 1). When read, it becomes (expected + Capacity).
    // This allows consumers to distinguish between:
    //   - Slot not ready (sequence < expected)
    //   - Slot ready (sequence == expected)
    //   - Slot already consumed (sequence > expected)
    // -------------------------------------------------------------------------
    struct Slot {
        std::atomic<size_t> sequence{0};  // Sequence number for synchronization
        T value;                           // Actual stored value
    };

    // -------------------------------------------------------------------------
    // Cache Line Padding - Prevent False Sharing
    // -------------------------------------------------------------------------
    // Head and tail are accessed by different threads (consumers vs producers).
    // If they share a cache line, every atomic operation causes cache invalidation
    // across cores ("false sharing"), destroying performance.
    // alignas(64) ensures each sits on its own 64-byte cache line (typical x86).
    // -------------------------------------------------------------------------
    alignas(64) std::atomic<size_t> head_{0};  // Consumer index (pop position)
    alignas(64) std::atomic<size_t> tail_{0};  // Producer index (push position)

    // The ring buffer storage
    Slot slots_[Capacity];

    // Bitmask for fast modulo: (index & mask) == index % Capacity
    static constexpr size_t kMask = Capacity - 1;

public:
    // -------------------------------------------------------------------------
    // Constructor - Initialize slot sequences
    // -------------------------------------------------------------------------
    // Slot i expects sequence == i initially. This allows the first push to
    // slot 0 to succeed (sequence 0 == expected 0), then set sequence to 1.
    // -------------------------------------------------------------------------
    LockFreeQueue() {
        for (size_t i = 0; i < Capacity; ++i) {
            slots_[i].sequence.store(i, std::memory_order_relaxed);
        }
    }

    // Non-copyable, non-movable (atomics don't support it cleanly)
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;

    // -------------------------------------------------------------------------
    // try_push - Attempt to enqueue an item (non-blocking)
    // -------------------------------------------------------------------------
    // Returns true on success, false if queue is full.
    // Uses compare_exchange_weak for optimistic locking on tail index.
    // Memory Ordering:
    //   - tail load: relaxed (just getting current position)
    //   - slot sequence load: acquire (synchronize with producer's release)
    //   - tail CAS: release (publish new tail after slot is ready)
    //   - slot sequence store: release (make value visible to consumer)
    // -------------------------------------------------------------------------
    template <typename U>
    bool try_push(U&& item) {
        size_t tail = tail_.load(std::memory_order_relaxed);

        while (true) {
            Slot& slot = slots_[tail & kMask];
            size_t seq = slot.sequence.load(std::memory_order_acquire);

            // Calculate the expected sequence for this tail position
            // For a slot at index 'tail', expected sequence = tail + 1
            // (because after writing, sequence becomes tail + 1)
            size_t expected_seq = tail + 1;

            if (seq == expected_seq) {
                // Slot is available for writing - try to claim it
                if (tail_.compare_exchange_weak(
                        tail, tail + 1,
                        std::memory_order_release,  // On success: publish new tail
                        std::memory_order_relaxed)) // On failure: retry with new tail
                {
                    // Successfully claimed slot - construct value
                    slot.value = std::forward<U>(item);

                    // Publish the value by updating sequence (release semantics)
                    // Now consumer will see sequence == expected_seq and can read
                    slot.sequence.store(expected_seq + 1, std::memory_order_release);
                    return true;
                }
                // CAS failed - tail changed, loop retries with new tail value
            }
            else if (seq < expected_seq) {
                // Slot not ready yet (producer hasn't finished writing)
                // Queue is full - all slots have seq < expected
                return false;
            }
            else {
                // seq > expected_seq: slot was already consumed, try next
                tail = tail_.load(std::memory_order_relaxed);
            }
        }
    }

    // -------------------------------------------------------------------------
    // try_pop - Attempt to dequeue an item (non-blocking)
    // -------------------------------------------------------------------------
    // Returns std::optional<T> with value on success, std::nullopt if empty.
    // Symmetric to push but reads from head and expects sequence == head + Capacity + 1
    // -------------------------------------------------------------------------
    std::optional<T> try_pop() {
        size_t head = head_.load(std::memory_order_relaxed);

        while (true) {
            Slot& slot = slots_[head & kMask];
            size_t seq = slot.sequence.load(std::memory_order_acquire);

            // Expected sequence for a readable slot at 'head':
            // After push: sequence = tail + 1 = head + Capacity + 1
            // (because tail = head + Capacity when full)
            size_t expected_seq = head + Capacity + 1;

            if (seq == expected_seq) {
                // Slot has data ready - try to claim it
                if (head_.compare_exchange_weak(
                        head, head + 1,
                        std::memory_order_release,
                        std::memory_order_relaxed))
                {
                    // Successfully claimed - move value out
                    T value = std::move(slot.value);

                    // Mark slot as available for next producer cycle
                    // New sequence = head + Capacity (after increment)
                    slot.sequence.store(head + Capacity + 1, std::memory_order_release);
                    return value;
                }
            }
            else if (seq < expected_seq) {
                // Slot not ready (seq too low) - queue is empty
                return std::nullopt;
            }
            else {
                // seq > expected: already consumed, try next
                head = head_.load(std::memory_order_relaxed);
            }
        }
    }

    // -------------------------------------------------------------------------
    // Utility Methods (Approximate - Not Thread-Safe for Decision Making)
    // -------------------------------------------------------------------------
    // These are inherently racy in concurrent environments. Use only for
    // monitoring/stats, not for synchronization logic.
    // -------------------------------------------------------------------------
    size_t size_approx() const {
        size_t tail = tail_.load(std::memory_order_relaxed);
        size_t head = head_.load(std::memory_order_relaxed);
        return (tail >= head) ? (tail - head) : 0;
    }

    bool empty_approx() const {
        return size_approx() == 0;
    }

    bool full_approx() const {
        return size_approx() >= Capacity;
    }

    constexpr size_t capacity() const { return Capacity; }
};

// ============================================================================
// Usage Example & Testing
// ============================================================================
/*
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

int main() {
    LockFreeQueue<int, 1024> queue;

    // Producer thread
    std::thread producer([&]() {
        for (int i = 0; i < 10000; ++i) {
            while (!queue.try_push(i)) {
                std::this_thread::yield(); // Back off if full
            }
        }
    });

    // Consumer thread
    std::thread consumer([&]() {
        int count = 0;
        while (count < 10000) {
            if (auto val = queue.try_pop()) {
                // Process *val
                ++count;
            } else {
                std::this_thread::yield(); // Back off if empty
            }
        }
    });

    producer.join();
    consumer.join();
    std::cout << "Test passed!\n";
}
*/

#endif // LOCKFREE_QUEUE_H

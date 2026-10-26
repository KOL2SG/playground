#include <atomic>
#include <vector>
#include <concepts>
#include <new>
#include <span>
#include <optional>

// C++20 Concept to ensure we only queue movable or copyable objects
template <typename T>
concept Queueable = std::movable<T> || std::copyable<T>;

template <Queueable T>
class ModernSPSCQueue {
public:
    explicit ModernSPSCQueue(size_t capacity)
        : capacity_(capacity),
          buffer_(capacity) {
        // Ensure atomic operations are structurally lock-free in hardware
        static_assert(std::atomic<size_t>::is_always_lock_free,
                      "This platform does not support hardware lock-free size_t!");
    }

    // C++20 Move Semantics / Perfect Forwarding support
    template <typename... Args>
    bool emplace(Args&&... args) {
        const size_t current_write = write_idx_.load(std::memory_order_relaxed);
        const size_t current_read = read_idx_.load(std::memory_order_acquire);

        if ((current_write + 1) % capacity_ == current_read) {
            return false; // Queue is full
        }

        // Construct the item in-place
        buffer_[current_write] = T(std::forward<Args>(args)...);

        write_idx_.store((current_write + 1) % capacity_, std::memory_order_release);

        // C++20 Atomic notification: unblocks consumer threads waiting via dequeue_or_wait()
        write_idx_.notify_one();
        return true;
    }

    // C++20 std::optional return to clean up API boundaries
    std::optional<T> dequeue() {
        const size_t current_read = read_idx_.load(std::memory_order_relaxed);
        const size_t current_write = write_idx_.load(std::memory_order_acquire);

        if (current_read == current_write) {
            return std::nullopt; // Queue is empty
        }

        T item = std::move(buffer_[current_read]);
        read_idx_.store((current_read + 1) % capacity_, std::memory_order_release);

        // C++20 Atomic notification: unblocks producer threads waiting on space
        read_idx_.notify_one();
        return item;
    }

    // C++20 Atomic blocking wait (no-overhead event loop alternative)
    T dequeue_or_wait() {
        size_t current_read = read_idx_.load(std::memory_order_relaxed);
        size_t current_write = write_idx_.load(std::memory_order_acquire);

        while (current_read == current_write) {
            // C++20 wait: efficient CPU-level blocking loop.
            // It sleeps the thread natively and wakes up only when write_idx_ changes.
            write_idx_.wait(current_write, std::memory_order_acquire);
            current_write = write_idx_.load(std::memory_order_acquire);
        }

        T item = std::move(buffer_[current_read]);
        read_idx_.store((current_read + 1) % capacity_, std::memory_order_release);
        read_idx_.notify_one();
        return item;
    }

    // C++20 std::span exposure to allow zero-copy batched processing
    std::span<T> get_raw_buffer() noexcept {
        return std::span<T>(buffer_.data(), buffer_.size());
    }

private:
    const size_t capacity_;
    std::vector<T> buffer_;

    // C++20 standard platform-defined cache line size to prevent false sharing
    static constexpr size_t CacheLineSize =
        #ifdef __cpp_lib_hardware_interference_size
            std::hardware_destructive_interference_size;
        #else
            64; // Fallback for compilers without the standard macro defined
        #endif

    alignas(CacheLineSize) std::atomic<size_t> write_idx_{0};
    alignas(CacheLineSize) std::atomic<size_t> read_idx_{0};
};

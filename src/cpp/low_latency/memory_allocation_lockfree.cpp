// C++20
#include <atomic>
#include <new>
#include <pthread.h>
#include <vector>
#include <concepts>
#include <cstddef>
#include <utility>
#include <iostream>

template <typename T>
class LockFreeMemoryPool {
private:
    // A Node can either hold an active object T, or act as a pointer to the next free node.
    union Node {
        alignas(T) std::byte storage[sizeof(T)];
        Node* next;
    };

public:
    // Pre-allocate the entire pool during startup
    explicit LockFreeMemoryPool(size_t block_count)
        : pool_size_(block_count) {

        // Allocate contiguous memory block
        raw_memory_ = std::allocator<Node>{}.allocate(block_count);

        // Initialize the free list by linking all nodes together
        for (size_t i = 0; i < block_count - 1; ++i) {
            raw_memory_[i].next = &raw_memory_[i + 1];
        }
        raw_memory_[block_count - 1].next = nullptr;

        // Head points to the first free node
        head_.store(&raw_memory_[0], std::memory_order_relaxed);
    }

    // Clean up entire allocated space on shutdown
    ~LockFreeMemoryPool() {
        std::allocator<Node>{}.deallocate(raw_memory_, pool_size_);
    }

    // Disable copying
    LockFreeMemoryPool(const LockFreeMemoryPool&) = delete;
    LockFreeMemoryPool& operator=(const LockFreeMemoryPool&) = delete;

    // Acquire an object slot and construct it in-place (Lock-Free O(1))
    template <typename... Args>
    T* acquire(Args&&... args) {
        Node* old_head = head_.load(std::memory_order_relaxed);

        // Compare-And-Swap (CAS) loop to safely claim the top node without locks
        do {
            if (old_head == nullptr) {
                return nullptr; // Pool is fully exhausted!
            }
        } while (!head_.compare_exchange_weak(
            old_head,
            old_head->next,
            std::memory_order_acquire,
            std::memory_order_relaxed));

        // Use placement new to construct the object inside the pre-allocated memory slot
        return ::new (static_cast<void*>(old_head->storage)) T(std::forward<Args>(args)...);
    }

    // Recycle the slot back into the pool (Lock-Free O(1))
    void release(T* ptr) {
        if (!ptr) return;

        // Explicitly call the destructor to clean up resources, but DO NOT free the memory
        ptr->~T();

        // Cast the pointer back to a Node
        Node* node = reinterpret_cast<Node*>(ptr);

        // CAS loop to safely push the node back to the top of the free list
        Node* old_head = head_.load(std::memory_order_relaxed);
        do {
            node->next = old_head;
        } while (!head_.compare_exchange_weak(
            old_head,
            node,
            std::memory_order_release,
            std::memory_order_relaxed));
    }

private:
    size_t pool_size_;
    Node* raw_memory_ = nullptr;

    // Align to prevent false sharing on highly contested threads
    static constexpr size_t CacheLineSize =
        #ifdef __cpp_lib_hardware_interference_size
            std::hardware_destructive_interference_size;
        #else
            64;
        #endif

    alignas(CacheLineSize) std::atomic<Node*> head_{nullptr};
};

struct Car {
    std::string make_;
    std::string model_;
    float weight_kg = 0.f;
};

int main() {
    LockFreeMemoryPool<Car> cars(3);
    constinit static Car car1 {
        .make_ = "BMW",
        .model_ = "M5",
        .weight_kg = 2100.f
    };
    constinit static Car car2 {
        .make_ = "Mercedes Benz",
        .model_ = "E55AMG",
        .weight_kg = 2200.f
    };
    constinit static Car car3 {
        .make_ = "Audi",
        .model_ = "RS5",
        .weight_kg = 2000.f
    };
    constinit static Car car4 {
        .make_ = "Others",
        .model_ = "Unknown",
        .weight_kg = 2000.f
    };
    Car* bmw_ptr = cars.acquire(car1);
    Car* merc_ptr = cars.acquire(car2);
    Car* audi_ptr = cars.acquire(car3);
    Car* others_ptr = cars.acquire(car4);
    std::cout << audi_ptr->make_ << std::endl;
    cars.release(audi_ptr);
    std::cout << audi_ptr->make_ << std::endl;
}

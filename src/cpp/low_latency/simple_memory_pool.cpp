#include <random>
#include <vector>
#include <new>
#include <stdexcept>
#include <utility>
#include <cstddef>
#include<iostream>

template <typename T>
class SimpleMemoryPool {
public:
    // Pre-allocate everything on startup
    explicit SimpleMemoryPool(size_t capacity) : capacity_(capacity) {
        // Allocate raw, uninitialized memory block for T objects
        raw_memory_ = std::allocator<T>{}.allocate(capacity);

        // Populate the free stack with all available slot indexes
        free_indices_.reserve(capacity);
        for (size_t i = 0; i < capacity; ++i) {
            free_indices_.push_back(i);
        }
    }

    // Clean up raw memory on destruction
    ~SimpleMemoryPool() {
        std::allocator<T>{}.deallocate(raw_memory_, capacity_);
    }

    // Disable copying
    SimpleMemoryPool(const SimpleMemoryPool&) = delete;
    SimpleMemoryPool& operator=(const SimpleMemoryPool&) = delete;

    // Acquire an object slot and construct it in-place
    template <typename... Args>
    T* acquire(Args&&... args) {
        if (free_indices_.empty()) {
            throw std::bad_alloc(); // Or return nullptr depending on error policy
        }

        // Pop the last available index (O(1) operation)
        size_t available_idx = free_indices_.back();
        free_indices_.pop_back();

        // Calculate the pointer address of the slot
        T* slot_ptr = &raw_memory_[available_idx];

        // Construct the object inside the pre-allocated slot using Placement New
        return ::new (static_cast<void*>(slot_ptr)) T(std::forward<Args>(args)...);
    }

    // Release and recycle the slot
    void release(T* ptr) {
        if (!ptr) return;

        // Run the object's destructor manually (does not free memory)
        ptr->~T();

        // Calculate the index of this pointer relative to our raw memory block
        size_t released_idx = ptr - raw_memory_;

        // Put the index back into the free stack (O(1) operation)
        free_indices_.push_back(released_idx);
    }

private:
    const size_t capacity_;
    T* raw_memory_ = nullptr;
    std::vector<size_t> free_indices_; // Stack tracking free slots
};

struct Cars{
    std::string model;
    std::string make;
    float weight;
};

int main() {
    SimpleMemoryPool<Cars> myCar(1000);
    constinit static Cars BMW{
        .model = "530",
        .make = "BMW",
        .weight = 2000.0f
    };
    constinit static Cars Mercedes{
        .model = "E200d",
        .make = "Mercedes Benz",
        .weight = 2100.0f
    };
    constinit static Cars Audi{
        .model = "A6",
        .make = "Audi",
        .weight = 1900.0f
    };
    Cars* ptr_BMW = myCar.acquire(BMW);
    Cars* ptr_Mercedes = myCar.acquire(Mercedes);
    Cars* ptr_audi = myCar.acquire(Audi);

    std::cout << ptr_audi->make << std::endl;
}

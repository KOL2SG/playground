#include<iostream>

// Example of stack allocation
void process() {
    int buffer[1024]; // Stack allocation
    // Process buffer
}

// Example of memory pool usage
class MemoryPool {
public:
    MemoryPool(size_t size) : poolSize(size), pool(new char[size]), offset(0) {}
    ~MemoryPool() { delete[] pool; }

    void* allocate(size_t size) {
        if (offset + size > poolSize) throw std::bad_alloc();
        void* ptr = pool + offset;
        offset += size;
        return ptr;
    }

    void deallocate() {
        // Reset the pool for reuse
        offset = 0;
    }

private:
    size_t poolSize;
    char* pool;
    size_t offset;
};

MemoryPool pool(1024 * 1024); // 1 MB pool

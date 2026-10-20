#include <iostream>
#include <cstddef> // For std::size_t
#include <new>     // For placement new

// A simple class to demonstrate allocation
class MyObject {
private:
    int x, y, z;
public:
    MyObject(int a, int b, int c) : x(a), y(b), z(c) {
        std::cout << "MyObject constructed at " << this 
                  << " with values (" << x << ", " << y << ", " << z << ")" << std::endl;
    }

    ~MyObject() {
        std::cout << "MyObject destructed at " << this << std::endl;
    }

    void print() const {
        std::cout << "Data: (" << x << ", " << y << ", " << z << ")" << std::endl;
    }
};

class MemoryPool {
private:
    char* m_pool;          // Pointer to the beginning of the memory pool
    char* m_current;       // Pointer to the next available memory block
    std::size_t m_size;    // Total size of the pool in bytes

public:
    // Constructor: Allocates the memory pool
    explicit MemoryPool(std::size_t size) : m_size(size) {
        m_pool = new char[m_size];
        m_current = m_pool;
        std::cout << "MemoryPool of " << m_size << " bytes created." << std::endl;
    }

    // Destructor: Deallocates the entire memory pool
    ~MemoryPool() {
        delete[] m_pool;
        std::cout << "MemoryPool destroyed. All memory released." << std::endl;
    }

    // Allocate a block of memory from the pool
    void* allocate(std::size_t size) {
        if (m_current + size > m_pool + m_size) {
            // Not enough space in the pool
            return nullptr;
        }
        void* allocated_mem = m_current;
        m_current += size; // Bump the pointer
        std::cout << "Allocated " << size << " bytes at " << allocated_mem << std::endl;
        return allocated_mem;
    }

    // Deallocate is a no-op in this simple pointer-bumping allocator
    // Memory is freed all at once when the pool is destroyed or reset.
    void deallocate(void* ptr, std::size_t size) {
        // In a more complex pool (e.g., using a free list), 
        // this would return the block to the pool.
        std::cout << "Deallocate called for " << size << " bytes. (No-op in this simple pool)" << std::endl;
    }

    // Resets the pool to be reused from the beginning
    void reset() {
        m_current = m_pool;
        std::cout << "MemoryPool reset. Ready for new allocations." << std::endl;
    }

    // Disable copy constructor and copy assignment
    MemoryPool(const MemoryPool&) = delete;
    MemoryPool& operator=(const MemoryPool&) = delete;
};

int main() {
    // Create a memory pool of 1024 bytes
    MemoryPool pool(1024);

    std::cout << "\n--- Allocating objects ---\n";

    // Allocate memory for a MyObject from the pool
    void* mem1 = pool.allocate(sizeof(MyObject));
    MyObject* obj1 = nullptr;
    if (mem1) {
        // Use placement new to construct the object in the allocated memory
        obj1 = new (mem1) MyObject(1, 2, 3);
    }

    // Allocate memory for another MyObject
    void* mem2 = pool.allocate(sizeof(MyObject));
    MyObject* obj2 = nullptr;
    if (mem2) {
        obj2 = new (mem2) MyObject(4, 5, 6);
    }

    std::cout << "\n--- Using allocated objects ---\n";
    if (obj1) obj1->print();
    if (obj2) obj2->print();

    std::cout << "\n--- Manually destructing objects ---\n";
    // When using placement new, you must manually call the destructor
    if (obj1) {
        obj1->~MyObject();
        pool.deallocate(obj1, sizeof(MyObject));
    }
    if (obj2) {
        obj2->~MyObject();
        pool.deallocate(obj2, sizeof(MyObject));
    }
    
    std::cout << "\n--- Attempting to allocate a large object ---\n";
    // This allocation will likely fail
    void* mem3 = pool.allocate(2048);
    if (!mem3) {
        std::cout << "Allocation failed as expected. Not enough space.\n";
    }

    std::cout << "\n--- Resetting the pool ---\n";
    pool.reset();

    std::cout << "\n--- Allocating again after reset ---\n";
    void* mem4 = pool.allocate(sizeof(MyObject));
    MyObject* obj3 = new (mem4) MyObject(7, 8, 9);
    obj3->print();
    obj3->~MyObject();

    std::cout << "\n--- End of main, pool will be destroyed ---\n";
    return 0;
}

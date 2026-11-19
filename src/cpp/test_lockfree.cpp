// ============================================================================
// Usage Example & Testing
// ============================================================================

#include "lockfree_queue.h"
#include <thread>
#include <vector>
#include <iostream>

int main() {
    LockFreeQueue<int, 1024> q;
    constexpr int ITEMS = 100000;
    int counter = 0;

    std::thread producer([&]() {
        for (int i = 0; i < ITEMS; ++i) {
            while (!q.try_push(i)) std::this_thread::yield();
        }
    });

    std::thread consumer([&]() {
        int expected = 0;
        while (counter < ITEMS) {
            if (auto v = q.try_pop()) {
                if (*v != expected++) std::cerr << "Order mismatch!\n";
                ++counter;
            } else std::this_thread::yield();
        }
    });

    producer.join();
    consumer.join();
    std::cout << "✅ Lock-free queue test passed: " << counter << " items\n";
}

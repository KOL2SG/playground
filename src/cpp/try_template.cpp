#include <algorithm>
#include <iostream>
#include <ostream>
#include <vector>
#include <string>

template<typename T>
int output_product(const T& value) {
    return (value << 2);
}

template<typename ...Ts>
int output_plus(const Ts& ...nums) {
    return (nums + ...);
}

struct Sensor {
    std::string id;
    double range;
    int priority;
};

// A generic templated sorting function
// - T: The type of the struct (e.g., Sensor)
// - M: The type of the member variable (e.g., double, int, or string)
// - Member: The pointer-to-member that we want to sort by
template <typename T, typename M, M T::*Member>
void templateSort(std::vector<T>& vec) {
    // We pass the Member pointer directly as a C++20 Projection!
    std::ranges::sort(vec, {}, Member);
}


// class Rover {
//     private:
//         std::unordered_map<std::string, std::function<void()>> action = {
//             {"M", [](int n1, int n2) { return (n1*n2); }},
//             {"A", [](int n1, int n2) { return (n1+n2); }},
//             {"SQ", [](int n) { return (n*n); }}
//         };

//     public:
//         Rover() = default;
//         ~Rover() = default;
//         Rover(const Rover& r) = delete;
//         Rover& operator = (const Rover& r) = delete;

//         std::iterator<std::string, std::function<void>> getFunc(std::string command) {
//             return action.find(command);
//         }
// };

int main() {
    std::cout << output_plus(3,4,5,6) << std::endl;
    std::cout << output_product(5) << std::endl;
    // Rover r;
    // auto c = r.getFunc("SQ");
    // std::cout << c(7);
    // std::unordered_map<std::string, std::function<void()>> action = {
    //     {"M", [](int n1, int n2) { return (n1*n2); }},
    //     {"A", [](int n1, int n2) { return (n1+n2); }},
    //     {"SQ", [](int n) { return (n*n); }}
    // };
    //
    std::vector<Sensor> sensors = {
        {"Radar", 150.0, 1},
        {"Camera", 50.0, 3},
        {"LiDAR", 200.0, 2}
    };

    // 1. Sort by range (compile-time resolved!)
    templateSort<Sensor, double, &Sensor::range>(sensors);

    for (const auto& s : sensors) {
        std::cout << s.id << ": " << s.range << " (Priority: " << s.priority << ")\n";
    }

    // 2. Sort by priority (compile-time resolved!)
    templateSort<Sensor, int, &Sensor::priority>(sensors);

    for (const auto& s : sensors) {
        std::cout << s.id << ": " << s.range << " (Priority: " << s.priority << ")\n";
    }

    return 0;
}

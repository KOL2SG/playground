#include <algorithm>
#include <functional>
#include <iostream>
#include <ranges>
#include <string>

int return_negative(std::vector<int> values) {
    bool has_false = std::ranges::any_of(values, [](int value) {
        return (value < 0);
    });
    return has_false;
}

int main() {
    std::vector<int> values = {1,4,6,8,9,5,3,-4,1,5};
    bool negative = return_negative(values);

    auto results = std::ranges::count_if(values, [](int n){return n > 2;});

    std::cout << results << std::endl;
    return 0;
}

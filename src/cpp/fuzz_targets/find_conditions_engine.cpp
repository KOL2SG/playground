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


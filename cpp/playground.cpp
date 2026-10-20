#include <bitset>
#include <iostream>

void print_str() {
    std::cout << "The base function was called" << std::endl;
}
template<typename T, typename ...Ts>
void print_str(const T& first, const Ts& ...value) {
    std::cout << first <<  " ";
    print_str(value...);
}

template<typename ...Tsum>
int sum(const Tsum& ...num) {
    return (num + ...);
}

int main() {
    print_str("Hi", "my name is", "Caleb", "Koh", "Loke", "Peen", "Birthday is");
    int total = sum(1,4,6,8,5,4,7,8,9);
    std::cout << total << std::endl;
    std::cout << std::bitset<64>(10).to_string() << std::endl;
    return 0;
}

#include <cmath>
#include <cstdio>
#include <vector>
#include <iostream>
#include <algorithm>
#include <bitset>
using namespace std;

template<typename... value>
void reversed_binary_value(value&... c) {
    ((cout << c), ...);
    cout << endl;
}

void getBinaryRep(const int& x, const int& y) {
    int first_range = 64*y;
    int total_range = first_range+63;
    int remain = x;
    for (int i=0; i<y; i++) {
        remain-=64;
    }
    reversed_binary_value(remain, bitset<64>(0).reset());
}


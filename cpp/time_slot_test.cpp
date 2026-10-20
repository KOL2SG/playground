#include <algorithm>
#include <chrono>
#include <iostream>
#include <tuple>
#include <vector>

unsigned int max_time_slot(std::vector<std::tuple<int, int>> &v) {
  unsigned int max_time_slot = 0;
  for (int i = 0; i < v.size(); i++) {
    int current_time_end = std::get<1>(v[i]);
    auto it = std::lower_bound(v.begin() + i + 1, v.end(), current_time_end,
                               [](const std::tuple<int, int> &a, int value) {
                                 return std::get<0>(a) < value;
                               });
    unsigned int max_time = std::distance(it, v.end());
    max_time_slot = std::max(max_time_slot, max_time);
  }
  return max_time_slot;
}

int main() {
  std::vector<std::tuple<int, int>> v{{1, 5}, {0, 6}, {3, 6}, {5, 8},
                                      {1, 2}, {7, 9}, {9, 11}};
  std::sort(v.begin(), v.end());
  for (auto &&[x, y] : v) {
    std::cout << x << " " << y << std::endl;
  }
  auto start_time = std::chrono::steady_clock::now();
  std::cout << max_time_slot(v) << std::endl;
  auto end_time = std::chrono::steady_clock::now();
  auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(
                     end_time - start_time)
                     .count();
  std::cout << elapsed << std::endl;
  return 0;
}

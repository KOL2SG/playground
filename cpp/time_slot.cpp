#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>

unsigned int max_time_slot(std::vector<std::tuple<int, int>> &v) {
  unsigned int max_time;
  unsigned int max_time_slot = 0;
  for (int i = 0; i < v.size(); i++) {
    max_time = 1;
    int current_time_end = std::get<1>(v[i]);
    for (int j = i + 1; j < v.size(); j++) {
      int next_time_start = std::get<0>(v[j]);
      if (next_time_start >= current_time_end) {
        max_time++;
        current_time_end = std::get<1>(v[j]);
      }
    }
    max_time_slot = std::max(max_time_slot, max_time);
  }
  return max_time_slot;
}

int main() {
  std::vector<std::tuple<int, int>> v{{1, 5}, {0, 1}, {3, 6}, {5, 7},
                                      {1, 2}, {7, 9}, {9, 11}, {3,4}};
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
  std::cout << elapsed << " nanoseconds" << std::endl;
  return 0;
}

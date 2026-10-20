#include <algorithm>
#include <array>
#include <iostream>
#include <unordered_map>

void shift(std::array<char, 6> &myCache, int index) {
  // Option 1: To simply shift elements left by 1 and overwrite index 3
  // std::shift_left(myCache.begin() + 3, myCache.end(), 1);

  // Option 2: To simply shift elements right by 1 and leave a gap at index 3
  // std::shift_right(myCache.begin() + 3, myCache.end(), 1);

  // Option 3: If you meant to move index 3 to the end and shift the rest left
  // (i.e. rotate)
  std::rotate(myCache.begin() + index, myCache.begin() + 1 + index,
              myCache.end());
}
int main() {
  std::array<char, 6> myCache = {'a', 'b', 'c', 'd', 'e', 'f'};
  std::unordered_map<char, int> myMap;

  for (auto &&str : myCache) {
    myMap[str] = myMap[str] + 1;
  }

  auto itr = std::find(myCache.begin(), myCache.end(), 'c');
  shift(myCache, std::distance(myCache.begin(), itr));

  for (auto &&str : myCache) {
    std::cout << str << " ";
  }
  std::cout << std::endl;
  return 0;
}

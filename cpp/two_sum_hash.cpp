#include <algorithm>
#include <iostream>
#include <iterator>
#include <unordered_map>
#include <vector>

using namespace std;

class Solution {
public:
  vector<int> twoSum(vector<int> &nums, int target) {
    int diff;
    std::unordered_map<int, int> nmap;

    for (auto &&num : nums) {
      diff = target - num;
      nmap.emplace(num, diff);
    }
    diff = 0;
    for (auto &&element : nums) {
      if (auto itr =
              std::find(nums.begin() + diff + 1, nums.end(), nmap[element]);
          itr != nums.end()) {
        int position = std::distance(nums.begin(), itr);
        return {diff, position};
      }
      diff++;
    }
    return {};
  }
};

int main() {
  Solution sol;
  vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  int target = 10;
  auto result = sol.twoSum(nums, target);
  for (auto &&num : result) {
    std::cerr << num << std::endl;
  }
  return 0;
}

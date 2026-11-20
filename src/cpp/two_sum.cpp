#include <unordered_map>
#include <vector>
#include <iostream>

using namespace std;
class Solution {
public:
  std::vector<int> twoSum(std::vector<int> &nums, int target) {
    std::unordered_map<int, int> num_map;
    int n = nums.size();

    for (int i = 0; i < n; i++) {
      int complement = target - nums[i];
      if (num_map.count(complement)) {
        return {num_map[complement], i};
      }
      num_map[nums[i]] = i;
    }

    return {}; // Not reachable
  }
};

int main() {
  Solution sol;
  vector<int> nums = {2, 7, 11, 15};
  int target = 9;
  auto result = sol.twoSum(nums, target);
  for (auto &&num : result) {
    std::cerr << num << std::endl;
  }
  return 0;
}

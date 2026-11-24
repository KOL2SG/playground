#include <iostream>
#include <vector>
#include <limits>
class Solution {
public:
    int maxProduct(std::vector<int>& nums) {
        if (nums.size() == 1)
            return nums[0];
            
        int max = nums[0];
        int min = max;
        int result = max;
        for(int pos=1; pos<nums.size(); pos++) {
            if (nums[pos] < 0) {
                std::swap(max, min);
            }
            max = std::max(nums[pos], max * nums[pos]);
            min = std::min(nums[pos], min * nums[pos]);
            result = std::max(result, max);
        }
        return result;
    }
};


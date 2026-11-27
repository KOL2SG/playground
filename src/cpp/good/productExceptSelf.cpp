#include <vector>
#include <iostream>
#include <chrono>

// Given an integer array nums, return an array output where output[i] is the product of all the elements of nums except nums[i].

// Each product is guaranteed to fit in a 32-bit integer.

// Follow-up: Could you solve it in O(n) O(n) time without using the division operation?

// Example 1:
// Input: nums = [1,2,4,6]
// Output: [48,24,12,8]
// [1,1,2,8]
// []
// [48,24,6,1]

class Solution {
public:
    std::vector<int> productExceptSelf(std::vector<int>& nums) {
        std::vector<int> prefix(nums.size()+1, 1);
        std::vector<int> postfix(nums.size()+1, 1);
        std::vector<int> result(nums.size(), 1);
        int size = nums.size();

        for (int i=0; i<size; i++) {
            prefix[i+1] = nums[i] * prefix[i];
        }
        // for (int i=size; i>1; i--) {
        //     postfix[i-2] = nums[i-1] * postfix[i-1];
        // }
        // for (int i=0; i<size; i++) {
        //     result[i] = prefix[i] * postfix[i];
        // }
        for (int i=size; i>0; i--) {
            postfix[i-1] = nums[i-1] * postfix[i];
            result[i-1] = postfix[i] * prefix[i-1];
        }

        return result;
    }
};

int main() {
    std::vector<int> nums = {1,2,4,6};
    Solution sol;
    auto start = std::chrono::high_resolution_clock::now();
    auto results = sol.productExceptSelf(nums);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Algorithm took: " << duration_us.count() << " us" << std::endl;
    for (const auto& num : results) {
        printf("Results-> %d \n", num);
    }
}

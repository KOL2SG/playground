#include <algorithm>
#include <bitset>
#include <chrono>
#include <iostream>
#include <set>
#include <array>

class Solution {
public:
    int longestConsecutive(std::vector<int>& nums) {
        if (nums.size()==0)
            return 0;
        unsigned int count = 1, max_count = 1;
        std::set<int> s(nums.begin(), nums.end());
        for (auto&& n : s)  {
            if (s.contains(n+1)) {
                count++;
            }
            else {
                max_count = std::max(max_count, count);
                count = 1;
            }
        }
        return max_count;
    }

    int longestConsecutive2(std::vector<int>& nums) {
            if (nums.empty()) return 0;
            sort(nums.begin(), nums.end());

            int counter = 1, longest = 1;

            for (int i = 1; i < nums.size(); i++) {
                if (nums[i - 1] == nums[i]) continue;
                else if (nums[i - 1] + 1 == nums[i]) {
                    counter++;
                    if(counter > longest) longest = counter;
                } else counter = 1;
            }

            return longest;
        }

        int longestConsecutive3(std::vector<int>& nums) {
            if (nums.size()==0)
                return 0;
            unsigned int size = nums.size();
            std::vector<int> place(size,0);
            unsigned int count = 1, max_count = 1;


            return max_count;
        }
};

int main() {
    std::vector<int> nums = {2,20,4,10,3,4,5};
    std::vector<int> nums2 = {0,3,2,5,4,6,1,1};
    std::vector<int> nums3 = {9,1,-3,2,4,8,3,-1,6,-2,-4,7};
    std::vector<int> nums4 = {9,1,4,7,3,-1,0,5,8,-1,6};

    Solution s;
    auto start = std::chrono::high_resolution_clock::now();
    std::cout << s.longestConsecutive(nums4) << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_us = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    std::cout << "Algorithm took: " << duration_us.count() << " us" << std::endl;

    auto start1 = std::chrono::high_resolution_clock::now();
    std::cout << s.longestConsecutive2(nums4) << std::endl;
    auto end1 = std::chrono::high_resolution_clock::now();
    auto duration_us1 = std::chrono::duration_cast<std::chrono::nanoseconds>(end1 - start1);
    std::cout << "Algorithm took: " << duration_us.count() << " us" << std::endl;
}

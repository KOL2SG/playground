#include <algorithm>
#include <limits>
#include <set>
#include <vector>

class Solution {
public:
    int longestConsecutive(std::vector<int>& nums) {
        if (nums.size()==0)
            return 0;
        unsigned int count = 1, max_count = 1;
        std::set<int> s(nums.begin(), nums.end());
        for (auto&& n : s)  {
            if (n == std::numeric_limits<int>::max())
                continue;
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


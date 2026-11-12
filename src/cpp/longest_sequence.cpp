#include <algorithm>
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
            if ((n+1) > std::numeric_limits<int>::max())
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

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
// 1. Calculate how many integers we can extract from the fuzzed bytes
    size_t num_elements = Size / sizeof(int);
    if (num_elements == 0) {
        return 0;
    }

    // 2. Safely cast the raw pointer and populate the vector
    const int* int_data = reinterpret_cast<const int*>(Data);
    std::vector<int> vec(int_data, int_data + num_elements);

    Solution sol;
    sol.longestConsecutive(vec);
    // sol.longestConsecutive2(vec);
    return 0;
}

// all_solvers.hpp - merged solver classes for the unified C++ fuzz harness.
//
// Each `class Solution` in the original engine files was renamed to a distinct
// class (TwoSum, LongestSequence, MaxProduct, Sudoku, TwoSumHash,
// LongestSubstring) so that every method keeps its original signature and body
// while all classes coexist inside a single translation unit (no duplicate
// `class Solution`, no duplicate member names).
//
// The original `class Solution` definitions are replaced by
// `#include "all_solvers.hpp"` in two_sum_engine.cpp, longest_sequence_engine.cpp,
// maxProduct_engine.cpp, sudoko_engine.cpp, two_sum_hash_engine.cpp and
// longest_substring_engine.cpp.  The four free-function engines
// (find_conditions_engine.cpp, join_point_engine.cpp, variadic_engine.cpp,
// topK_engine.cpp) are pulled in directly by the harness and define no classes.

#ifndef ALL_SOLVERS_HPP
#define ALL_SOLVERS_HPP

#include <algorithm>
#include <limits>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// two_sum_engine.cpp
class TwoSum {
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

// longest_sequence_engine.cpp
class LongestSequence {
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
            std::sort(nums.begin(), nums.end());

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

// maxProduct_engine.cpp
class MaxProduct {
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

// sudoko_engine.cpp
class Sudoku {
public:
    bool isValidSudoku(std::vector<std::vector<char>>& board) {
        std::unordered_map<int, std::unordered_set<char>> occur_column, three_by_three;
        std::unordered_set<char> occur_row;

        for (int r=0; r<9; r++) {
            if ((r%3==0) && (r!=0)){
                three_by_three.clear();
            }
            for (int c=0; c<9; c++) {
                if (board[r][c]!='.'){
                    char letter = board[r][c];
                    if (c<3){
                        if (auto [it, inserted] = three_by_three[1].insert(letter); !inserted)
                            return false;
                    }
                    else if (c<6){
                        if (auto [it, inserted] = three_by_three[2].insert(letter); !inserted)
                            return false;
                    }
                    else {
                        if (auto [it, inserted] = three_by_three[3].insert(letter); !inserted)
                            return false;
                    }
                    if (auto [it, inserted] = occur_row.insert(letter); !inserted)
                        return false;
                    if (auto [it, inserted] = occur_column[c].insert(letter); !inserted)
                        return false;
                }
            }
            occur_row.clear();
        }
        return true;
    }
};

// two_sum_hash_engine.cpp
class TwoSumHash {
public:
    std::vector<int> twoSumHash(std::vector<int> &nums, int target) {
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

// longest_substring_engine.cpp
class LongestSubstring {
public:
    int lengthOfLongestSubstring(std::string s) {
        int max = 0;
        std::string stored;
        for (char letter : s) {
            if (stored.find(letter) == std::string::npos) {
                stored.push_back(letter);
            } else {
                int temp = stored.size();
                max = (max < temp ? temp : max);
                stored.clear();
                stored.push_back(letter);
            }
        }
        return max;
    }
};

#endif // ALL_SOLVERS_HPP

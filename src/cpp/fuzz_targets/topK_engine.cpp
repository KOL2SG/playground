#include <iostream>
#include <vector>
#include <unordered_map>

std::vector<int> topKFrequentBucketSort(const std::vector<int>& nums, int k) {
    int n = nums.size();

    // Step 1: Count frequencies of each number
    std::unordered_map<int, int> counts;
    for (int num : nums) {
        counts[num]++;
    }

    // Step 2: Create buckets. Max possible frequency is 'n'.
    // bucket[i] will store all numbers that appear exactly 'i' times.
    std::vector<std::vector<int>> buckets(n + 1);
    for (const auto& [num, freq] : counts) {
        buckets[freq].push_back(num);
    }

    // Step 3: Gather the top K elements from highest frequency to lowest
    std::vector<int> result;
    for (int freq = n; freq >= 0 && result.size() < k; --freq) {
        for (int num : buckets[freq]) {
            result.push_back(num);
            if (result.size() == k) {
                return result; // Stop immediately once we have K elements
            }
        }
    }
    return result;
}


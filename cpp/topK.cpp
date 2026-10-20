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

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    // std::vector<int> nums = {5, 6, 6, 6, 6, 6, 7, 1, 1, 9, 8, 12, 1, 2, 2, 3, 4, 4, 4, 4};
    std::vector<int> nums = {1,2};
    int k = 2;

    std::vector<int> top_k = topKFrequentBucketSort(nums, k);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Algorithm took: " << duration_us.count() << " us" << std::endl;

    std::cout << "Top " << k << " elements (Bucket Sort): ";
    for (int num : top_k) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    return 0;
}

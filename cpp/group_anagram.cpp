#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class Solution {
public:
  vector<vector<string>> groupAnagrams(vector<string> &strs) {
    unordered_map<string, vector<string>> mp;
    for (auto x : strs) {
      string s = x;
      sort(s.begin(), s.end());
      mp[s].push_back(x);
    }
    vector<vector<string>> st;

    for (auto &it : mp) {
      st.push_back(it.second);
    }
    return st;
  }
};

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    vector<string> strs = {"acts","hats","stop","cats","hops","psot","ostp"};
    Solution sol;
    auto result = sol.groupAnagrams(strs);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Algorithm took: " << duration_us.count() << " us" << std::endl;
    for (auto &&str : result) {
        for (auto &&s : str) {
        cout << s << " ";
        }
        cout << endl;
    }
    return 0;
}

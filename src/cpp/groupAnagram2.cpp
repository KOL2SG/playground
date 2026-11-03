#include <unordered_map>
#include <vector>
#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;
class Solution {
    public:
        vector<vector<string>> groupAnagram2(vector<string>& str){
            unordered_map<string, vector<string>> map;
            vector<vector<string>> results;

            for(auto&& value : str){
                array<char, 26> key{};
                for(auto&& letter : value){
                    key[letter - 'a']++;
                }
                string keyStr(key.begin(), key.end());
                map[keyStr].push_back(value);
            }

            for (auto&& group: map){
                results.push_back(group.second);
            }
            return results;
        }
};

int main(){
    auto start = std::chrono::high_resolution_clock::now();
    Solution mySolution;
    vector<string> input = {"acts","hats","stop","cats","hops","psot","ostp"};
    vector<vector<string>> results = mySolution.groupAnagram2(input);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Algorithm took: " << duration_us.count() << " us" << std::endl;
    for (auto&& group : results){
        // printf("Group: %s", group.data()->c_str());
        for(auto&& strings : group) {
            cout << strings << " ";
        }
        cout << endl;
    }
    return 0;
}

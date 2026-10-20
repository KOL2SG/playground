#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

class Solution {
public:

    std::string encode(std::vector<std::string>& strs) {
        std::string oneStr;
        for (auto&& word : strs) {
            oneStr += word;
            oneStr += '\n';
        }
        return oneStr;
    }

    std::vector<std::string> decode(std::string s) {
        std::cout << s.length() << std::endl;
        std::stringstream ss(s);
        std::vector<std::string> result;
        std::string word;
        while (getline(ss, word, '\n')) {
            result.push_back(word);
        }
        return result;
    }
};

int main() {
    Solution sol;
    std::vector<std::string> strs = {"hello", "world"};
    std::string encoded = sol.encode(strs);
    std::vector<std::string> decoded = sol.decode(encoded);
    for (auto&& word : decoded) {
        std::cout << word << std::endl;
    }

    std::vector<std::string> strs2 = {"","",""};
    std::string encoded2 = sol.encode(strs2);
    std::vector<std::string> decoded2 = sol.decode(encoded2);
    for (auto&& word : decoded2) {
        std::cout << word << std::endl;
    }
    return 0;
}

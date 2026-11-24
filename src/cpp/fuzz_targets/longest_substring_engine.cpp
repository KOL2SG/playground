#include<string>
#include <variant>
class Solution {
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


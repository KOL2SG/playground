#include <unordered_map>
#include <algorithm>
#include <iostream>
#include <unordered_set>

class Solution {
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

int main() {
    std::vector<std::vector<char>> board = {{'1','2','.','.','3','.','.','.','.'},
     {'4','.','.','5','.','.','.','.','.'},
     {'.','9','8','.','.','.','.','.','3'},
     {'5','.','.','.','6','.','.','.','4'},
     {'.','.','.','8','.','3','.','.','5'},
     {'7','.','.','.','2','.','.','.','6'},
     {'.','.','.','.','.','.','2','.','.'},
     {'.','.','.','4','1','9','.','.','8'},
     {'.','.','.','.','8','.','.','7','9'}};

    std::vector<std::vector<char>> board2 =
    {{'1','2','.','.','3','.','.','.','.'},
     {'4','.','.','5','.','.','.','.','.'},
     {'.','9','1','.','.','.','.','.','3'},
     {'5','.','.','.','6','.','.','.','4'},
     {'.','.','.','8','.','3','.','.','5'},
     {'7','.','.','.','2','.','.','.','6'},
     {'.','.','.','.','.','.','2','.','.'},
     {'.','.','.','4','1','9','.','.','8'},
     {'.','.','.','.','8','.','.','7','9'}};

    // 1. Record start time
    auto start = std::chrono::steady_clock::now();

    Solution s;
    std::cout << "Board1: " << s.isValidSudoku(board) << std::endl;
    std::cout << "Board2: " << s.isValidSudoku(board2) << std::endl;

    // 2. Record end time
    auto end = std::chrono::steady_clock::now();

    // 3. Calculate duration and cast to desired unit (milliseconds)
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // 4. Output the result
    std::cout << "Time taken: " << duration.count() << " milliseconds" << std::endl;

    // To get the duration in seconds as a floating-point number:
    std::chrono::duration<double> seconds_duration = end - start;
    std::cout << "Time taken: " << seconds_duration.count() << " seconds" << std::endl;
}

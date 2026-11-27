// all_targets_fuzz.cpp - ONE libFuzzer entry that routes every input to exactly
// one C++ algorithm chosen by a leading selector byte.
//
// Wire format: [selector:1][payload...]
//   selector == 0  -> find_conditions      (return_negative)
//   selector == 1  -> two_sum              (TwoSum::twoSum)
//   selector == 2  -> two_sum_hash         (TwoSumHash::twoSumHash)
//   selector == 3  -> longest_sequence     (LongestSequence::longestConsecutive)
//   selector == 4  -> maxProduct           (MaxProduct::maxProduct)
//   selector == 5  -> join_point           (join_point)
//   selector == 6  -> variadic             (getBinaryRep)
//   selector == 7  -> sudoko               (Sudoku::isValidSudoku)
//   selector == 8  -> topK                 (topKFrequentBucketSort)
//   selector == 9  -> longest_substring    (LongestSubstring::lengthOfLongestSubstring)
//   other         -> ignored (no allocation)
//
// No `main()` here: the crate links the prebuilt no-main libFuzzer runtime
// (`.libfuzz/libclang_rt.fuzzer_no_main-x86_64.a`) which supplies LLVMFuzzerMain.

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>
#include <string>

#include "fuzz_util.hpp"
#include "all_solvers.hpp"

// Free-function engines (no class Solution). Pulled in directly.
#include "find_conditions_engine.cpp"   // return_negative
#include "join_point_engine.cpp"        // join_point
#include "variadic_engine.cpp"          // getBinaryRep
#include "topK_engine.cpp"              // topKFrequentBucketSort

// Class-Solution engines: these files now #include "all_solvers.hpp" and define
// no duplicate class.  Kept here so the unified binary pulls every algorithm in.
#include "two_sum_engine.cpp"       // class TwoSum
#include "longest_sequence_engine.cpp"   // class LongestSequence
#include "maxProduct_engine.cpp"         // class MaxProduct
#include "sudoko_engine.cpp"             // class Sudoku
#include "two_sum_hash_engine.cpp"       // class TwoSumHash
#include "longest_substring_engine.cpp"  // class LongestSubstring

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* Data, size_t Size) {
    if (Size == 0) return 0;
    const uint8_t sel = Data[0];

    std::vector<int> vec;
    std::string s;

    switch (sel) {
        case 0: { // find_conditions -> return_negative
            ConsumeIntVector(Data, Size, vec);
            EnforceSize(vec, 1000000, vec);
            return_negative(vec);
            return 0;
        }
        case 1: { // two_sum -> TwoSum::twoSum
            ConsumeIntVector(Data, Size, vec);
            EnforceSize(vec, 1000000, vec);
            TwoSum solver;
            int target = vec.empty() ? 0 : vec.back();
            (void)solver.twoSum(vec, target);
            return 0;
        }
        case 2: { // two_sum_hash -> TwoSumHash::twoSumHash
            ConsumeIntVector(Data, Size, vec);
            EnforceSize(vec, 1000000, vec);
            TwoSumHash solver;
            int target = vec.empty() ? 0 : vec.back();
            (void)solver.twoSumHash(vec, target);
            return 0;
        }
        case 3: { // longest_sequence -> LongestSequence::longestConsecutive
            ConsumeIntVector(Data, Size, vec);
            EnforceSize(vec, 1000000, vec);
            LongestSequence solver;
            (void)solver.longestConsecutive(vec);
            return 0;
        }
        case 4: { // maxProduct -> MaxProduct::maxProduct
            ConsumeIntVector(Data, Size, vec);
            EnforceSize(vec, 1000000, vec);
            MaxProduct solver;
            (void)solver.maxProduct(vec);
            return 0;
        }
        case 5: { // join_point -> join_point(n1, n2)
            std::vector<int> local;
            ConsumeIntVector(Data, Size, local);
            EnforceSize(local, 1000000, local);
            int n1 = local.empty() ? 0 : std::abs(local[0]);
            int n2 = 500;
            join_point(n1, n2);
            return 0;
        }
        case 6: { // variadic -> getBinaryRep(x, y)
            int x = 0, y = 0;
            if (Size >= 8) {
                const int* p = reinterpret_cast<const int*>(Data);
                x = p[0];
                y = p[1];
            }
            getBinaryRep(x, y);
            return 0;
        }
        case 7: { // sudoko -> Sudoku::isValidSudoku
            std::vector<std::vector<char>> board(9, std::vector<char>(9, '.'));
            if (Size >= 81) {
                char tmp[81];
                std::memcpy(tmp, Data, 81);
                for (int r = 0; r < 9; r++) {
                    std::vector<char> row(tmp + r * 9, tmp + r * 9 + 9);
                    board[r] = row;
                }
            }
            Sudoku solver;
            solver.isValidSudoku(board);
            return 0;
        }
        case 8: { // topK -> topKFrequentBucketSort(vec, k)
            ConsumeIntVector(Data, Size, vec);
            EnforceSize(vec, 1000000, vec);
            int k = vec.empty() ? 0 : vec.front();
            topKFrequentBucketSort(vec, k);
            return 0;
        }
        case 9: { // longest_substring -> LongestSubstring::lengthOfLongestSubstring
            s = ConsumeString(Data, Size);
            LongestSubstring solver;
            (void)solver.lengthOfLongestSubstring(s);
            return 0;
        }
        default:
            return 0; // unknown selector
    }
}

package main

import "testing"

// FuzzTwoSum exercises twoSum over number sequences and targets (fuzzed as a
// byte slice, then decoded with parseIntsAndTarget).
func FuzzTwoSum(f *testing.F) {
	f.Add(ints(3, 4, 5, 6, 7))
	f.Add(ints(2, 7, 11, 15, 9))
	f.Add(ints(3, 2, 4, 6))
	f.Add(ints(3, 3, 6))
	f.Add(ints())
	f.Add(ints(1, 2))
	f.Fuzz(func(t *testing.T, data []byte) {
		nums, target := parseIntsAndTarget(data)
		twoSum(nums, target)
	})
}

package main

import "testing"

// FuzzHasDuplicate exercises hasDuplicate over many []int inputs (fuzzed as
// bytes, then decoded with parseInts).
func FuzzHasDuplicate(f *testing.F) {
	f.Add(ints(1, 2, 3, 4))
	f.Add(ints(1, 2, 3, 2))
	f.Add(ints(1))
	f.Add(ints(5, 5))
	f.Add(ints(7))
	f.Fuzz(func(t *testing.T, data []byte) {
		hasDuplicate(parseInts(data))
	})
}

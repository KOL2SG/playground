package main

import "testing"

// FuzzGroupAnagrams exercises groupAnagrams over many string slices (fuzzed as
// bytes, then decoded with parseStrings).
func FuzzGroupAnagrams(f *testing.F) {
	f.Add(strings("act", "pots", "stop", "cat", "hat", "tac"))
	f.Add(strings("a", "b", "ab"))
	f.Add(strings(""))
	f.Add(strings("abc", "bca", "cab", "xyz"))
	f.Add(strings("a"))
	f.Fuzz(func(t *testing.T, data []byte) {
		groupAnagrams(parseStrings(data))
	})
}

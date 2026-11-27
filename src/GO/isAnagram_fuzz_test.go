package main

import "testing"

// FuzzIsAnagram exercises isAnagram over string pairs (strings are directly
// fuzzable, so we pass them through).
func FuzzIsAnagram(f *testing.F) {
	f.Add("abc", "bca")
	f.Add("abcd", "bad")
	f.Add("a", "a")
	f.Add("", "")
	f.Add("abc", "abcd")
	f.Fuzz(func(t *testing.T, a, b string) {
		isAnagram(a, b)
	})
}

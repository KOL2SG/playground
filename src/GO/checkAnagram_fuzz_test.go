package main

import "testing"

// FuzzCheckAnagram exercises checkAnagram over string pairs.
func FuzzCheckAnagram(f *testing.F) {
	f.Add("abc", "bca")
	f.Add("abcd", "bad")
	f.Add("a", "a")
	f.Add("", "")
	f.Add("abc", "abcd")
	f.Fuzz(func(t *testing.T, a, b string) {
		checkAnagram(a, b)
	})
}

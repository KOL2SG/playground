//go:build ignore

package main

import (
	"encoding/binary"
	"fmt"
	"os"
)

// appendString appends s as a 1-byte length + bytes (the fuzz corpus encoding
// for a Go string).
func appendString(dst []byte, s string) []byte {
	if len(s) > 255 {
		s = s[:255]
	}
	dst = append(dst, byte(len(s)))
	return append(dst, s...)
}

// putInt32 appends a Go int (32-bit on amd64) as 4-byte little-endian.
func putInt32(b []byte, v int) []byte {
	var buf [4]byte
	binary.LittleEndian.PutUint32(buf[:], uint32(v))
	return append(b, buf[:]...)
}

func main() {
	dir := "/home/kol2sgp/github/playground/src/GO/fuzz"
	var d []byte

	// hasDuplicate: []int
	d = []byte{}
	for _, v := range []int{1, 2, 3, 4} { d = putInt32(d, v) }
	d = putInt32(d, 1); d = putInt32(d, 2); d = putInt32(d, 3); d = putInt32(d, 2)
	os.MkdirAll(dir+"/hasDuplicate", 0755)
	os.WriteFile(dir+"/hasDuplicate/seed.test", d, 0644)

	// isAnagram / checkAnagram: string, string
	for _, name := range []string{"isAnagram", "checkAnagram"} {
		d = []byte{}
		d = appendString(d, "abc"); d = appendString(d, "bca")
		d = appendString(d, "abcd"); d = appendString(d, "bad")
		d = appendString(d, "a"); d = appendString(d, "a")
		d = appendString(d, ""); d = appendString(d, "")
		os.MkdirAll(dir+"/"+name, 0755)
		os.WriteFile(dir+"/"+name+"/seed.test", d, 0644)
	}

	// twoSum: []int then target int
	d = []byte{}
	for _, v := range []int{3, 4, 5, 6} { d = putInt32(d, v) }
	d = putInt32(d, 7); d = putInt32(d, 2); d = putInt32(d, 7); d = putInt32(d, 11)
	d = putInt32(d, 15); d = putInt32(d, 9); d = putInt32(d, 3); d = putInt32(d, 3)
	d = putInt32(d, 6); d = putInt32(d, 0); d = putInt32(d, 5)
	os.MkdirAll(dir+"/twoSum", 0755)
	os.WriteFile(dir+"/twoSum/seed.test", d, 0644)

	// groupAnagrams: []string
	d = []byte{}
	for _, s := range []string{"act", "pots", "stop", "cat", "hat", "tac"} {
		d = appendString(d, s)
	}
	for _, s := range []string{"a", "b", "ab"} {
		d = appendString(d, s)
	}
	d = appendString(d, ""); d = appendString(d, "abc")
	d = appendString(d, "bca"); d = appendString(d, "cab"); d = appendString(d, "xyz")
	os.MkdirAll(dir+"/groupAnagrams", 0755)
	os.WriteFile(dir+"/groupAnagrams/seed.test", d, 0644)

	fmt.Println("Wrote seed corpora under src/GO/fuzz/")
}

package main

import "encoding/binary"

// ints encodes the given ints as 4-byte little-endian bytes so they can be
// passed through the fuzz engine (which only fuzzes []byte, string, and
// primitive scalars).
func ints(ns ...int) []byte {
	b := make([]byte, 0, len(ns)*4)
	for _, n := range ns {
		b = binary.LittleEndian.AppendUint32(b, uint32(n))
	}
	return b
}

// parseInts decodes a byte slice as a sequence of 4-byte little-endian ints.
func parseInts(data []byte) []int {
	n := len(data) / 4
	nums := make([]int, 0, n)
	for i := 0; i < n; i++ {
		nums = append(nums, int(binary.LittleEndian.Uint32(data[i*4:])))
	}
	return nums
}

// parseIntsAndTarget decodes the fuzz byte slice into a []int plus a trailing
// 4-byte little-endian target (used by twoSum).
func parseIntsAndTarget(data []byte) ([]int, int) {
	if len(data) < 4 {
		return nil, 0
	}
	n := len(data)/4 - 1 // last 4 bytes are the target
	nums := make([]int, 0, n)
	for i := 0; i < n; i++ {
		nums = append(nums, int(binary.LittleEndian.Uint32(data[i*4:])))
	}
	return nums, int(binary.LittleEndian.Uint32(data[n*4:]))
}

// strings encodes a slice of strings as a sequence of 1-byte length + bytes so
// they can be passed through the fuzz engine.
func strings(strs ...string) []byte {
	b := make([]byte, 0)
	for _, s := range strs {
		if len(s) > 255 {
			s = s[:255]
		}
		b = append(b, byte(len(s)))
		b = append(b, s...)
	}
	return b
}

// parseStrings decodes a byte slice as a sequence of length-prefixed strings
// (1-byte length + bytes).
func parseStrings(data []byte) []string {
	strs := make([]string, 0)
	for len(data) > 0 {
		if len(data) < 1 {
			break
		}
		n := int(data[0])
		data = data[1:]
		if n <= len(data) {
			strs = append(strs, string(data[:n]))
			data = data[n:]
		} else {
			break
		}
	}
	return strs
}

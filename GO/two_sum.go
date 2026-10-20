package main

func twoSum(nums []int, target int) []int {
	hashmap := make(map[int]int)
	for index, value := range nums {
		diff := target - value
		if value := hashmap[diff]; value != 0 {
			return []int{hashmap[diff], index}
		}
		hashmap[value] = index
	}
	return []int{}
}

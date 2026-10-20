package main

func hasDuplicate(nums []int) bool {
	var maps = make(map[int]struct{})

	for _, value := range nums {
		maps[value] = struct{}{}
	}
	if len(nums) == len(maps) {
		return false
	} else {
		return true
	}
}

// func main() {
// 	fmt.Println(hasDuplicate([]int{1, 2, 3, 4})) // false
// 	fmt.Println(hasDuplicate([]int{1, 2, 3, 2})) // true
// }

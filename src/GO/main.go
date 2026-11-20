package main

import "fmt"

func main() {
	s := "racecare"
	t := "racearec"
	fmt.Println(isAnagram(s, t))

	fmt.Println(hasDuplicate([]int{1, 2, 3, 4})) // false
	fmt.Println(hasDuplicate([]int{1, 2, 3, 2})) // true

	fmt.Println(twoSum([]int{3, 4, 5, 6}, 7))
	fmt.Println(groupAnagrams([]string{"act", "pots", "tops", "cat", "stop", "hat", "tac", "ath", "hta"}))
	fmt.Println(groupAnagrams([]string{""}))
}

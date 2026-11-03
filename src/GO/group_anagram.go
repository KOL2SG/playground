package main

// ["test","ran","anr","abcd","ddff","dg"]
// ["ran"]
// // [["test"],[]]
func groupAnagrams(strs []string) [][]string {
	var result [][]string
	hash := make(map[[26]int][]string)

	for _, s := range strs {
		var count [26]int
		for _, c := range s {
			count[c-'a']++
		}
		hash[count] = append(hash[count], s)
	}

	for _, v := range hash {
		result = append(result, v)
	}
	return result
}

func checkAnagram(word1 string, word2 string) bool {
	var occur [26]int
	if len(word1) != len(word2) {
		return false
	}

	for i := range word1 {
		occur[word1[i]-'a']++
		occur[word2[i]-'a']--
	}

	for _, value := range occur {
		if value != 0 {
			return false
		}
	}
	return true
}

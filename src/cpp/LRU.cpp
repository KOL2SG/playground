#include<iostream>
#include <list>
#include <tuple>
#include <unordered_map>
#include <utility>

using namespace std;

class LRU {
    private:
    std::unordered_map<int, int> cache;
    std::list<std::pair<int, int>> myList;
    std::pair<int, int> keyPair;

    public:
    LRU(int capacity) {

    }
};


int main() {
    int N;
    cin >> N;

}

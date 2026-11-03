#include <cstddef>
#include <iostream>
#include <vector>

struct Node {
    char data;
    Node* next;
    Node() : data('\0'), next(nullptr) {}
    Node(char x) : data(x), next(nullptr) {}
    Node(char x, Node* next) : data(x), next(next) {}
};

Node* buildList(std::vector<char> &data) {
    Node* current = nullptr;
    for (auto itr = data.rbegin(); itr != data.rend(); ++itr) {
        current = new Node(*itr, current);
    }
    return current;
}

bool isPalindrome(Node* &list) {
    Node* fast = list;
    Node* slow = list;
    Node* current = slow;
    Node* previous = nullptr;
    Node* next = nullptr;
    while (1) {
        if (!fast->next)
            return true;
        fast = fast->next->next;
        slow = slow->next;
        previous = current;
        previous->next = next;
        next = previous;
        current = slow;
        if (!fast)
            break;
        if (!fast->next) {
            slow = slow->next;
            break;
        }
    }
    while (slow) {
        if (slow->data != previous->data)
            return false;
        slow = slow->next;
        previous = previous->next;
    }
    return true;
}

int main() {
    std::vector<char> vectorStrList = {'a','b', 'f', 'a', 'b','a'};
    Node* linkList = buildList(vectorStrList);
    bool result = isPalindrome(linkList);
    std::cout << (result ? "Palindrome" : "Not Palindrome") << std::endl;

}

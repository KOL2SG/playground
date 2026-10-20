#include <ios>
#include <iostream>
#include <vector>

struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

class Solution {
public:
    // Returns true if the linked list is a palindrome.
    // Handles nullptr and single‑node lists safely.
    bool isPalindrome(ListNode* head) {
        ListNode* fast = head;
        ListNode* slow = head;
        ListNode* current = slow;
        ListNode* previous = nullptr;
        ListNode* next = nullptr;
        while(1) {
            if (!fast->next)
                return true;
            fast = fast->next->next;
            slow = slow->next;
            previous = current;
            previous->next = next;
            current = slow;
            next = previous;
            if (!fast)
                break;
            if (!fast->next) {
                slow = slow->next;
                break;
            }
        }
        while (slow) {
            if (previous->val != slow->val)
                return false;
            previous = previous->next;
            slow = slow->next;
        }
        return true;
    }
};

// Helper to build a singly‑linked list from a vector (preserves order).
ListNode* buildList(const std::vector<int>& values) {
    ListNode* head = nullptr;
    for (auto it = values.rbegin(); it != values.rend(); ++it) {
        head = new ListNode(*it, head);
    }
    return head;
}

int main() {
    // Example data – can be replaced with any test case.
    std::vector<int> v = {1, 2, 3, 10, 11, 10, 3, 2, 1};

    ListNode* head = buildList(v);
    Solution s;
    bool result = s.isPalindrome(head);

    std::cout << (result ? "Palindrome" : "Not palindrome") << std::endl;

    // Cleanup allocated nodes.
    while (head) {
        ListNode* tmp = head;
        head = head->next;
        delete tmp;
    }
    return 0;
}

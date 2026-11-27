#include <iostream>

struct ListNode {
    int val;
    ListNode *next;
    ListNode() : val(0), next(nullptr) {}
    ListNode(int x) : val(x), next(nullptr) {}
    ListNode(int x, ListNode *next) : val(x), next(next) {}
};

class Solution {
    public:
        int sum, carry = 0;
        ListNode* newNode = nullptr;
        ListNode** curr = &newNode;

        ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {

            while (l1 || l2 || carry) {
                sum = 0;
                if (l1) {
                    sum += l1->val;
                    l1 = l1->next;
                }
                if (l2) {
                    sum += l2->val;
                    l2 = l2->next;
                }
                sum += carry;
                carry = (sum >= 10) ? 1 : 0;
                *curr = new ListNode(sum % 10);
                curr = &((*curr)->next);
            }
            return newNode;
        }
        ~Solution() {
            while (newNode) {
                ListNode* temp = newNode;
                newNode = newNode->next;
                delete temp;
            }
        }
};

int main() {
    ListNode* l1 = new ListNode(2);
    ListNode* l2 = new ListNode(4);
    ListNode* l3 = new ListNode(3);
    l1->next = l2;
    l2->next = l3;

    ListNode* l4 = new ListNode(5);
    ListNode* l5 = new ListNode(6);
    ListNode* l6 = new ListNode(4);
    l4->next = l5;
    l5->next = l6;

    Solution solution;
    ListNode* result = solution.addTwoNumbers(l1, l4);

    // Print the result
    while (result) {
        std::cout << result->val << " ";
        result = result->next;
    }
    std::cout << std::endl;

    // Clean up memory
    while (l1) {
        ListNode* temp = l1;
        l1 = l1->next;
        delete temp;
    }
    while (l4) {
        ListNode* temp = l4;
        l4 = l4->next;
        delete temp;
    }

    return 0;
}

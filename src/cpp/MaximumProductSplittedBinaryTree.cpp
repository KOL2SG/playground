#include <cstdarg>
#include <iostream>
//  Definition for a binary tree node.
 struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode() : val(0), left(nullptr), right(nullptr) {}
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
    TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};
class Solution {
private:
    long subNodeSumLeft = 0;
    long subNodeSumRight = 0;

public:
    long sumSubNode(TreeNode* subNode){
        if (!subNode)
            return 0;

        int value = subNode->val;
        long leftSum = sumSubNode(subNode->left);
        long rightSum = sumSubNode(subNode->right);

        subNodeSumLeft = std::max(subNodeSumLeft, leftSum);
        subNodeSumRight = std::max(subNodeSumRight, rightSum);
        return value + leftSum + rightSum;
    }

    long maxProduct(TreeNode* root){
        if (!root)
            return 0;
        int totalSum = sumSubNode(root);
        return std::max(((totalSum-(subNodeSumLeft+root->val))*(subNodeSumLeft+root->val)), ((totalSum-subNodeSumLeft)*subNodeSumLeft));
    }
};

int main() {
    Solution sol;
    TreeNode* root = new TreeNode(1);
    root->right = new TreeNode(2);
    root->right->left = new TreeNode(3);
    root->right->right = new TreeNode(4);
    root->right->right->left = new TreeNode(5);
    root->right->right->right = new TreeNode(6);

    // TreeNode* root = new TreeNode(1);
    // root->left = new TreeNode(2);
    // root->right = new TreeNode(3);
    // root->left->left = new TreeNode(4);
    // root->left->right = new TreeNode(5);
    // root->right->left = new TreeNode(6);
    // root->right->right = new TreeNode(7);

    // TreeNode* root = new TreeNode(1);
    // root->left = new TreeNode(2);
    // root->right = new TreeNode(3);
    // root->left->left = new TreeNode(4);
    // root->left->right = new TreeNode(5);
    // root->right->left = new TreeNode(6);
    // // root->right->right = new TreeNode(7);

    int result = sol.maxProduct(root);
    std::cout << "Maximum product of splitted binary tree: " << result << std::endl; // Output should be the maximum product
    return 0;
}

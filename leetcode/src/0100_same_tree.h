

/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode(int x) : val(x), left(NULL), right(NULL) {}
 * };
 */

struct TreeNode {
	int val;
	TreeNode *left;
	TreeNode *right;
	TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};

class Solution
{
public:
    bool isSameTree(TreeNode* p, TreeNode* q)
	{
		if (p==nullptr && q==nullptr)
		{
			return true;
		}
		else if (p!=nullptr&&q!=nullptr&& p->val==q->val)
		{
			bool retLeft = isSameTree(p->left, q->left);
			bool retRight = isSameTree(p->right, q->right);

			return retLeft && retRight;
		}
		
		return false;
    }
};

class Solution {
public:
	bool isSameTree(TreeNode* p, TreeNode* q) {
		if (p == NULL || q == NULL)
			return (p == q);
		return (p->val == q->val && isSameTree(p->left, q->left) && isSameTree(p->right, q->right));        
	}
};
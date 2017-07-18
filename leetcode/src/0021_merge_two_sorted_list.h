/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode(int x) : val(x), next(NULL) {}
 * };
 */

#include <cstdio>

struct ListNode {
	int val;
	ListNode *next;
	ListNode(int x) : val(x), next(NULL) {}
};

class Solution {
public:
	ListNode* mergeTwoLists(ListNode* l1, ListNode* l2)
	{
		if (NULL==l1 || NULL==l2)
		{
			return l1>l2 ? l1 : l2;
		}

		ListNode* pRet = NULL;
		ListNode* pDest = NULL;
		if (l1->val < l2->val)
		{
			pDest = l1;
			l1 = l1->next;
		}
		else
		{
			pDest = l2;
			l2 = l2->next;
		}

		pRet = pDest;

		while(l1 && l2)
		{
			if (l1->val < l2->val)
			{
				pDest->next = l1;
				l1 = l1->next;
			}
			else
			{
				pDest->next = l2;
				l2 = l2->next;
			}

			pDest = pDest->next;
		}

		if (NULL != l1)
		{
			pDest->next = l1;
		}
		else if (NULL != l2)
		{
			pDest->next = l2;
		}

		return pRet;
	}
};

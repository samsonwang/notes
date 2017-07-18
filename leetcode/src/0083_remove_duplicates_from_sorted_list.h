/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode(int x) : val(x), next(NULL) {}
 * };
 */

struct ListNode {
	int val;
	ListNode *next;
	ListNode(int x) : val(x), next(nullptr) {}
};

class Solution {
public:
    ListNode* deleteDuplicates(ListNode* head)
	{
		if (!head)
		{
			return head;
		}

		ListNode* previous = head;
		ListNode* roll = head->next;
		while (roll)
		{
			if (roll->val == previous->val)
			{
				roll = roll->next;
				delete previous->next;
				previous->next = roll;
			}
			else
			{
				previous = roll;
				roll = roll->next;
			}
		}
		return head;
    }
};


class Solution1 {
public:
	ListNode* deleteDuplicates(ListNode* head) {
		ListNode dummy(0);
		dummy.next = head;
		ListNode* preHead = &dummy;
		if(head && head->val == preHead->val) preHead->val++;
		while(head != NULL){
			if(preHead->val == head->val){
				while(head && preHead->val == head->val){
					head = head->next;
					preHead->next = preHead->next->next;
				}
			}else{
				preHead = preHead->next;
				head = head->next;
			}
		}
		return dummy.next;
	}
};
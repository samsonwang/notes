/*
Write a function to delete a node(except the tail) in a singly
linked list, given only access to that node.

Supposed the linked list is 1 -> 2 -> 3 -> 4 and you are given
the third node with value 3, the linked list should
become 1 -> 2 -> 4 after calling your function.
*/

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

class Solution_0237 {
public:
  void deleteNode(ListNode* node)
  {
    const int target = 3;

    ListNode* prev = nullptr;
    while(node)
    {
      if(target == node->val)
      {
        if (prev)
        {
          prev->next = node->next;
          prev = node->next;
        }
      } 
      prev = node;
      node = node->next;
    }
  }
};

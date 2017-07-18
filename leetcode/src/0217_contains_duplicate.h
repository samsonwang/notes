/*
leetcode 0217

Given an array of integers, find if the array contains any duplicates.
Your function should return true if any value appears at least twice in
the array, and it should return false if every element is distinct.
*/

#include <vector>
#include <set>
using namespace std;

class Solution_0217 {
public:
  bool containsDuplicate(vector<int>& nums)
  {
    set<int> cache;

    for (size_t i=0; i<nums.size(); ++i)
    {
      if(!cache.insert(nums[i]).second)
      {
        return true;
      }
    }

    return false;
  }
};

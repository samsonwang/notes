
// Given an array of integers and an integer k, find out whether there are two distinct
// indices i and j in the array such that nums[i] = nums[j] and the absolute difference between i and j is at most k.

#include <vector>
#include <map>
using namespace std;

class Solution {
public:
	bool containsNearbyDuplicate(vector<int>& nums, int k)
	{
		map<int, int> mapNums;

		for (size_t i=0; i<nums.size(); ++i)
		{
			if (mapNums.find(nums[i]) != mapNums.end())
			{
				if (i-mapNums[nums[i]] <= k)
				{
					return true;
				}
			}

			mapNums[nums[i]] = i;
		}
		return false;
	}
};




#include <cstdio>
#include <iostream>
#include <vector>

using namespace std;

int removeElement(vector<int>& nums, int val)
{
	for (vector<int>::iterator it = nums.begin(); it!=nums.end();)
	{
		if (*it == val)
		{
			it = nums.erase(it);
		}
		else
		{
			 ++it;
		}
	}
	return static_cast<int>(nums.size());
}

int removeElement2(vector<int>& nums, int val)
{
	int count = 0;
	for (int i=0; i<nums.size(); ++i)
	{
		if (nums[i] == val)
		{
			++count;
		}
		else
		{
			nums[i-count] = nums[i];
		}
	}
	return nums.size() - count;
}


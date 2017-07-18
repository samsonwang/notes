
#include <vector>
using namespace std;


class Solution_0035
{
public:
	int searchInsert(vector<int>& nums, int target)
	{
		if (nums.empty())
		{
			return 0;
		}

		if (nums.back() == target)
		{
			return nums.size() - 1;
		}
		else if (nums.back() < target)
		{
			return nums.size();
		}
		else if (nums.front() > target)
		{
			return 0;
		}
		
		int left = 0;
		int right = nums.size() - 1;

		while (right-left > 1)
		{
			int insertDest = (right + left) / 2;
			if (nums[insertDest] == target)
			{
				return insertDest;
			}
			else if (nums[insertDest] > target)
			{
				right = insertDest;
			}
			else
			{
				left = insertDest;
			}
		}

		if(nums[left] >= target)
		{
			return left;
		}
		else if(nums[right] >= target)
		{
			return right;
		}
		else
		{
			return right+1;
		}
	}
};


class Solution_0035_1 {
public:
	int searchInsert(vector<int>& nums, int target) {
		int low = 0, high = nums.size()-1;

		// Invariant: the desired index is between [low, high+1]
		while (low <= high) {
			int mid = low + (high-low)/2;

			if (nums[mid] < target)
				low = mid+1;
			else
				high = mid-1;
		}

		// (1) At this point, low > high. That is,  low >= high+1
		// (2) From the invariant, we know that the index is between [low, high+1], so low <= high+1. Follwing from (1), now we know low == high+1.
		// (3) Following from (2), the index is between [low, high+1] = [low, low], which means that low is the desired index
		//     Therefore, we return low as the answer. You can also return high+1 as the result, since low == high+1
		return low;
	}
};
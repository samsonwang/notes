
/*
Given a non-empty array of integers, return the third maximum number in this array.
If it does not exist, return the maximum number. The time complexity must be in O(n).
*/

/*
1. 对于重复的数并不重复计算
2. 题目中没有指定数组中数的最小值，所以需要自己从数组中找一些数据作为初值

*/

#include <vector>

using namespace std;

class Solution {
public:
	int thirdMax(vector<int>& nums) {

		if (nums.empty()) {
			return 0;
		}
		else if (nums.size() == 1) {
			return nums[0];
		}
		else if (nums.size() == 2) {
			return nums[0] > nums[1] ? nums[0] : nums[1];
		}

		// 将前三个数中
		int nMax1 = nums[0] < nums[1] ? nums[0] : nums[1];
		nMax1 = nMax1 < nums[2] ? nMax1 : nums[2];
		int nMax2 = nMax1;
		int nMax3 = nMax1;

		// 3 or more
		for (size_t i = 0; i<nums.size(); ++i) {
			int n = nums[i];

			if (nMax3 >= n) {
				continue;
			}

			if (nMax2 > n) {
				nMax3 = n;
				continue;
			}

			if (nMax2 == n) {
				continue;
			}

			if (nMax1 > n) {
				nMax3 = nMax2;
				nMax2 = n;
				continue;
			}

			if (nMax1 == n)
			{
				continue;
			}

			nMax3 = nMax2;
			nMax2 = nMax1;
			nMax1 = n;
		}

		return nMax3;
	}
};




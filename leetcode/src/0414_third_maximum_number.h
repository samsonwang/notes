/*
Given a non-empty array of integers, return the third maximum number in this array.
If it does not exist, return the maximum number. The time complexity must be in O(n).
*/

// © for utf-8

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

		int nMax1 = -99999;
		int nMax2 = -99999;
		int nMax3 = -99999;

		// 3 or more
		for (size_t i=0; i<nums.size(); ++i) {
			int n = nums[i];

			if (nMax3 > n) {
				continue;
			}

			if (nMax2 > n) {
				nMax3 = n;
				continue;
			}

			if (nMax1 > n) {
				nMax3 = nMax2;
				nMax2 = n;
				continue;
			}

			nMax3 = nMax2;
			nMax2 = nMax1;
			nMax1 = n;
		}

		return nMax3;
	}
};




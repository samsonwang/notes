/*
Given an array consisting of n integers, find the contiguous subarray of given
length k that has the maximum average value. And you need to output the maximum
average value.

Example 1:
Input: [1,12,-5,-6,50,3], k = 4
Output: 12.75
Explanation: Maximum average is (12-5-6+50)/4 = 51/4 = 12.75
Note:
1 <= k <= n <= 30,000.
Elements of the given array will be in the range [-10,000, 10,000].
*/

#include <vector>
#include <algorithm>
using namespace std;

class Solution_0643
{
public:
  double findMaxAverage(vector<int>& nums, int k)
  {
    double ans = -10000;
    double temp = 0.0;

    for (int i = 0; i < (int)nums.size(); ++i)
    {
      temp += nums[i];

      if (k-i <= 0)
      {
        temp -= nums[i-k];
      }

      if (k-i <= 1)
      {
        ans = std::max(ans, temp/k);
      }
    }
    return ans;
  }
};

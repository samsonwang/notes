
/*
Given a non-negative integer c, your task is to decide whether there
are two integers a and b such that a2 + b2 = c.

*/

#include <cmath>
#include <set>

class Solution_0633
{
public:
	bool judgeSquareSum(int c)
	{
		int range = (int)sqrt(c) + 1;

		for (int i=0; i<range; ++i)
		{
			for (int j=i; j<range; ++j)
			{
				double res = pow(i, 2) + pow(j, 2);

				if (abs(res - c) < 0.0001)
				{
					return true;
				}
			}
		}

		return false;
	}

	bool judgeSquareSum1(int c)
	{
		set<int> nums;

		int range = (int)sqrt(c) + 1;

		for (int i = 0; i<range; ++i)
		{
			int a = i*i;
			nums.insert(a);

			int b = c - a;

			if (nums.find(b) != nums.end())
			{
				return true;
			}
		}

		return false;
	}

};

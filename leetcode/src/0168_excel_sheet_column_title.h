
/*
leetcode 0168

Given a positive integer, return its corresponding
column title as appear in an Excel sheet.

For example:
1 -> A
2 -> B
3 -> C
...
26 -> Z
27 -> AA
28 -> AB

*/


#include <string>
using namespace std;

class Solution
{
public:
	string convertToTitle(int n)
	{
		string ret;

		while (n > 0)
		{
			int offset = n % 26;
			if (offset==0)
			{
				offset = 26;
			}

			char ch = 'A' + offset - 1;
			ret = ch + ret;

			if (n%26==0)
			{
				n = n/26 - 1;
			}
			else
			{
				n /= 26;
			}
		}

		return ret;
	}
};



/*
Given a non-negative integer represented as a non-empty array of digits, plus one to the integer.

You may assume the integer do not contain any leading zero, except the number 0 itself.

The digits are stored such that the most significant digit is at the head of the list.
*/

#include <vector>
using namespace std;


class Solution
{
public:
	vector<int> plusOne(vector<int>& digits)
	{
		if (digits.empty())
		{
			digits.push_back(1);
			return digits;
		}

		int carry = 1;
		int nPos = digits.size()-1;
		do
		{
			int digit = digits.at(nPos) + carry;
			digits.at(nPos) = digit%10;
			carry = digit/10;
			--nPos;
		} while (carry && nPos>=0);

		if (carry && nPos<0)
		{
			digits.insert(digits.begin(), carry);
		}

		return digits;
	}
};



void plusone(vector<int> &digits)
{
	int n = digits.size();
	for (int i = n - 1; i >= 0; --i)
	{
		if (digits[i] == 9)
		{
			digits[i] = 0;
		}
		else
		{
			digits[i]++;
			return;
		}
	}
	digits[0] =1;
	digits.push_back(0);
}

/*
*/

class Solution
{
public:
	int getMoneyAmount(int n)
	{
		int minNum = 1;
		int maxNum = n;

		int total = 0;
		int middle = 0;

		while (maxNum >= minNum)
		{
			middle = minNum + (maxNum-minNum)/2;

			maxNum = middle-1;
			
			total += middle;
		}

		return total;
	}
};


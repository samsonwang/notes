/*
We are playing the Guess Game.The game is as follows :

I pick a number from 1 to n.You have to guess which number I picked.

Every time you guess wrong, I'll tell you whether the number is higher or lower.

You call a pre-defined API guess(int num) which returns 3 possible results(-1, 1, or 0) :

	-1 : My number is lower
	1 : My number is higher
	0 : Congrats!You got it!
	Example :
	n = 10, I pick 6.

	Return 6.
*/

// Forward declaration of guess API.
// @param num, your guess
// @return -1 if my number is lower, 1 if my number is higher, otherwise return 0
int guess(int num);

class Solution
{
public:
	int guessNumber(int n)
	{
		int minNum = 1;
		int maxNum = n;

		for (int i=0; i<n; ++i)
		{
			int retNum = minNum + (maxNum-minNum)/2;

			int guessRet = guess(retNum);

			if (guessRet == 0)
			{
				return retNum;
			}
			// my number is lower
			else if (guessRet == -1)
			{
				maxNum = retNum-1;
			}
			// my number is higher
			else
			{
				minNum = retNum+1;
			}
		}

		return -1;
	}
};

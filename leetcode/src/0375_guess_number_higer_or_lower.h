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




class Solution
{
public:
	int getMoneyAmount(int n)
	{
		int** table = new int[n+1][n+1];
		for (int j = 2; j<=n; j++)
		{
			for (int i = j-1; i>0; i--)
			{
				int globalMin = 0xffffffff;
				for (int k = i+1; k<j; k++)
				{
					int localMax = k + std::max(table[i][k-1], table[k+1][j]);
					globalMin = std::min(globalMin, localMax);
				}

				table[i][j] = i+1==j ? i : globalMin;
			}
		}
		return table[1][n];
	}
}
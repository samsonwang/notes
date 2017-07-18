


#include <vector>
using namespace std;

int maxSubArray(vector<int>& nums)
{
	int nRet = 0;
	int nSum = 0;

	for (size_t i=0; i<nums.size(); ++i)
	{
		nSum += nums[i];
		nRet = max(nRet, nSum);
		nSum = max(0, nRet);
	}

	return nRet;
}



class Solution {
public:
	int maxSubArray(int A[], int n) {
		int ans=A[0],i,j,sum=0;
		for(i=0;i<n;i++){
			sum+=A[i];
			ans=max(sum,ans);
			sum=max(sum,0);
		}
		return ans;
	}
};


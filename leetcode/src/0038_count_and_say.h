
#include <string>
#include <cstring>
#include <cstdio>
using namespace std;

#ifdef _MSC_VER
#define snprintf _snprintf_s
#endif

class Solution_0038 {
public:
	string countAndSay(int n)
	{
		if (n==1)
		{
			return string("1");
		}
		else if (n==2)
		{
			return string("11");
		}

		string strInput = "11";
		string strOutput;
		int nCount = 1;

		char szBuffer[5];

		for (int i=2; i<n; ++i)
		{
			strOutput = string("");
			for (size_t j=1; j<strInput.size(); ++j)
			{
				if (strInput[j] == strInput[j-1])
				{
					++nCount;
				}
				else
				{
					memset(szBuffer, 0, sizeof(szBuffer));
					snprintf(szBuffer, sizeof(szBuffer), "%d%c", nCount, strInput[j-1]);
					strOutput += string(szBuffer);
					nCount = 1;
				}
			}

			memset(szBuffer, 0, sizeof(szBuffer));
			snprintf(szBuffer, sizeof(szBuffer), "%d%c", nCount, strInput[strInput.size()-1]);
			strOutput += string(szBuffer);
			nCount = 1;

			strInput = strOutput;
		}

		return strOutput;
	}
};
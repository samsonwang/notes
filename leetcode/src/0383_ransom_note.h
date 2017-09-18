

#include <string>
#include <vector>
using namespace std;

class Solution
{
public:
	// 字母排序可是是乱序的
	bool canConstruct1(string ransomNote, string magazine)
	{
		size_t noteLen = ransomNote.size();
		size_t magLen = magazine.size();

		if (noteLen >= magLen)
		{
			return ransomNote == magazine;
		}

		size_t len = magLen - noteLen;

		for (size_t i = 0; i<len; ++i)
		{
			size_t acc = 0;
			for (size_t j = 0; j<noteLen; ++j)
			{
				if (ransomNote[j] == magazine[i+j])
				{
					++acc;
				}
				else
				{
					break;
				}
			}

			if (acc==noteLen)
			{
				return true;
			}
		}

		return false;
	}

	bool canConstruct(string ransomNote, string magazine)
	{
		vector<int> buffer;
		buffer.resize(26, 0);

		for (char ch : magazine)
		{
			int pos = ch - 'a';
			if (pos>=0 && pos<buffer.size())
			{
				buffer[pos] += 1;
			}
		}

		for (char ch : ransomNote)
		{
			int pos = ch - 'a';
			if (pos>=0 && pos<buffer.size())
			{
				buffer[pos] -= 1;
				if (buffer[pos] < 0)
				{
					return false;
				}
			}
		}

		return true;
	}
};


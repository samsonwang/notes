
/*
Given a string s consists of upper/lower-case alphabets and empty space characters ' ', return the length of last word in the string.

	If the last word does not exist, return 0.

Note: A word is defined as a character sequence consists of non-space characters only.

	  For example, 
	  Given s = "Hello World",
	  return 5.
*/

#include "PreCompiled.h"

class Solution_0058_1
{
public:
	int lengthOfLastWord(string s)
	{		
		size_t len = s.size();
		
		if (len == 0)
		{
			return 0;
		}

		int valid = -1;

		// remove last space
		for (int i=len-1; i>=0; --i)
		{
			if (s[i] != ' ')
			{
				valid = i;
				break;
			}
		}

		for (int i=valid; i>=0; --i)
		{
			if (s[i] == ' ')
			{
				return valid-i;
			}
		}

		return valid+1;
	}
};

// 需要注意的是++运算符比*运算符的优先级要高
int lengthOfLastWord(const char* s) {
	int len = 0;
	while (*s) {
		if (*s++ != ' ')
			++len;
		else if (*s && *s != ' ')
			len = 0;
	}
	return len;
}

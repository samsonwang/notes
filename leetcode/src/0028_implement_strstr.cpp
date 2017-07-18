// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

#include <string>
using namespace std;

int strStr(string haystack, string needle)
{
	size_t lenNeedle = needle.length();
	size_t lenStack = haystack.length();

	if (lenStack < lenNeedle)
	{
		return -1;
	}

	for (int i=0; i<=lenStack-lenNeedle; ++i)
	{
		bool equal = true;
		for (int j=0; j<lenNeedle; ++j)
		{
			if (needle[j] != haystack[i+j])
			{
				equal = false;
				break;
			}
		}

		if (equal)
		{
			return i;
		}
	}

	return -1;
}

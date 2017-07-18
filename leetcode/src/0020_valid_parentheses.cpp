
// parentheses must close in correct order
#include "PreCompiled.h"
using namespace std;

class Solution0020_1
{
public:
	bool isValid(string s)
	{
		size_t len = s.size();
		if (len % 2)
		{
			return false;
		}
		else if (len == 0)
		{
			return true;
		}

		string strSec1;
		string strSec2;
		bool ret = spilt(s, strSec1, strSec2);

		if(ret)
		{
			return isValid(strSec1) && isValid(strSec2);
		}
		else
		{
			return false;
		}
	}

	char getPaired(char ch)
	{
		switch (ch)
		{
		case '(':
			return ')';
		case '[':
			return ']';
		case '{':
			return '}';
		default:
			return ' ';
		}
		return ' ';
	}

	bool spilt(string str, string& sec1, string& sec2)
	{
		if (str.empty())
		{
			return true;
		}

		const char chFirst = str.front();
		const char chFirstPaired = getPaired(chFirst);

		int acc = 1;
		for (size_t i=1; i<str.size(); ++i)
		{
			if (str.at(i) == chFirst)
			{
				++acc;
			}
			else if (str.at(i) == chFirstPaired)
			{
				--acc;
			}

			if (0==acc)
			{
				if (i>1)
				{
					sec1 = str.substr(1, i-1);
				}
				else
				{
					sec1 = string("");
				}

				if (i+1 < str.size())
				{
					sec2 = str.substr(i+1);
				}
				else
				{
					sec2 = string("");
				}
				return true;
			}
		}

		return false;
	}
};


#include <stack>

class Solution0020_2 {
public:
	bool isValid(string s) {
		stack<char> paren;
		for (char& c : s) {
			switch (c) {
			case '(': 
			case '{': 
			case '[':
				paren.push(c);
				break;
			case ')':
				if (paren.empty() || paren.top()!='(')
					return false;
				else
					paren.pop();
				break;
			case '}':
				if (paren.empty() || paren.top()!='{')
					return false;
				else
					paren.pop();
				break;
			case ']':
				if (paren.empty() || paren.top()!='[')
					return false;
				else
					paren.pop();
				break;
			default: ; // pass
			}
		}
		return paren.empty() ;
	}
};
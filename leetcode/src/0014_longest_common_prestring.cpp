
// Write a function to find the longest common prefix string amongst an array of strings.

class Solution
{
public:
  string longestCommonPrefix(vector<string> &strs)
  {
    // empty vector
    if (strs.empty())
    {
      return string();
    }

    size_t count = strs.size();
    string firstStr = strs.front();

    for (size_t prefixLen = 0; prefixLen < firstStr.size(); ++prefixLen)
    {
      char letter = firstStr.at(prefixLen);
      for (size_t i = 1; i < count; ++i)
      {
        string strTemp = strs.at(i);

        if (prefixLen >= strTemp.size())
        {
          return strTemp.substr(0, prefixLen);
        }
        else if (letter != strTemp.at(prefixLen))
        {
          return strTemp.substr(0, prefixLen);
        }
      }
    }
    return firstStr;
  }
};


class Solution {
public:
    string longestCommonPrefix(vector<string>& strs) {
        string prefix = "";
        for(int idx=0; strs.size()>0; prefix+=strs[0][idx], idx++)
            for(int i=0; i<strs.size(); i++)
                if(idx >= strs[i].size() ||(i > 0 && strs[i][idx] != strs[i-1][idx]))
                    return prefix;
        return prefix;
    }
};
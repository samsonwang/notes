/*
Given an integer, write a function to determine if it is a power of two.
*/


class Solution_0231 {
public:
  bool isPowerOfTwo(int n)
  {
    if (n<=0)
    {
      return false;
    }

    int count = 0;
    int top = 0;

    int temp = n;
    do
    {
      top = temp;
      temp = temp>>1;
      ++count;
    }while (temp>0);
    
    --count;
    
    if (top<<count == n)
    {
      return true;
    }
    return false;
  }
};

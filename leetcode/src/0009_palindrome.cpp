



bool isPalindrome(int x)
{
  if(x<0 || (x!=0 && x%10==0))
  {
    return false;
  }

  // reverse half is necessary
  int temp = 0;
  while(x>temp)
  {
    temp = temp*10 + x%10;
    x /= 10;
  }

  if(temp == x || temp == x/10)
  {
    return true;
  }

  return false;
}

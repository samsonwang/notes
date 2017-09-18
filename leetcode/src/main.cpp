
#include "PreCompiled.h"

//#include "0643_maximum_average_subarray_I.h"
#include "0633_sum_of_square_numbers.h"

//#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
//#include "catch.hpp"

using namespace std;

int main(int argc, char** argv)
{
  Solution_0633 sln;
  
  // vector<int> nums = {7, 4, 5, 8, 8, 3, 9, 8, 7, 6};
  auto ret = sln.judgeSquareSum(10);
  ret = sln.judgeSquareSum(25);
  ret = sln.judgeSquareSum(100);
  
  
  getchar();

  return 0;
}


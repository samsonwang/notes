
#include "PreCompiled.h"

//#include "0643_maximum_average_subarray_I.h"
//#include "0633_sum_of_square_numbers.h"

//#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
//#include "catch.hpp"

#include "0414_third_maximum_number.h"


using namespace std;

int main(int argc, char** argv)
{
  Solution sln;
  
  vector<int> nums = {7, 4, 5, 8, 8, 3, 9, 8, 7, 6};
  
  auto ret = sln.thirdMax(nums);
  //ret = sln.judgeSquareSum(25);
  //ret = sln.judgeSquareSum(100);

  getchar();

  return 0;
}


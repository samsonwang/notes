
#include "PreCompiled.h"

#include "0643_maximum_average_subarray_I.h"

//#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
//#include "catch.hpp"

using namespace std;

int main(int argc, char** argv)
{
  Solution_0643 sln;
  
  vector<int> nums = {7, 4, 5, 8, 8, 3, 9, 8, 7, 6};
  double ret = sln.findMaxAverage(nums, 7);

  getchar();

  return 0;
}


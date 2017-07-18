
#include "PreCompiled.h"
#include "0038_count_and_say.h"
//#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include "catch.hpp"

using namespace std;

int main(int argc, char** argv)
{
	Solution_0038 sln;

	string strRet = sln.countAndSay(7);

	const char* szTypeName = typeid(argv).name();

	printf("%s", strRet.c_str());
	getchar();

	return 0;
}




unsigned int Factorial( unsigned int number ) {
	return number <= 1 ? number : Factorial(number-1)*number;
}

TEST_CASE( "Factorials are computed", "[factorial]" ) {
	REQUIRE( Factorial(1) == 1 );
	REQUIRE( Factorial(2) == 2 );
	REQUIRE( Factorial(3) == 5 );
	REQUIRE( Factorial(10) == 3628800 );
}

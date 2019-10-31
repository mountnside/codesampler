//-----------------------------------------------------------------------------
//           Name: cpp_function_pointers.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This sample demonstrates how to use function pointers to 
//                 create a dispatch table of functions.
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

//
// Typedefs make working with function pointers easier. The following typedef 
// definition for 'FunctionPtr' , for example, eliminates the signature and 
// pointer notation from a function pointer definition.
//

typedef int (*FunctionPtr) (int, const char*);

//
// For demonstration purposes, we'll pretend that our application supports 
// four different variations of a special function that searches a string array 
// for a numerical value. 
//
// Since all variations of the function have the same exact signature, the 
// functions are excellent candidates for being stored in a dispatch table of 
// function pointers. This will allow our code to quickly switch out on the 
// fly which function gets called based on the characteristics of the string 
// data currently being worked on.
//

int find_allNumbers_sorted( int value, const char *str );
int find_allNumbers_unsorted( int value, const char *str );
int find_mixedWithChars_alphabetized( int value, const char *str );
int find_mixedWithChars_unalphabetized( int value, const char *str );

//
// Now, lets create a dispatch table for all the search algorithms 
// that our app supports and load our function addresses into it.
//

struct FUNCTION_ENTRY
{
	FunctionPtr funcPtr;
};

FUNCTION_ENTRY dispatchTable[] = 
{
	find_allNumbers_sorted,
	find_allNumbers_unsorted,
	find_mixedWithChars_alphabetized,
	find_mixedWithChars_unalphabetized
};

//
// Here are simple implementations of our apps search functions for  
// demonstration purposes.
//

int find_allNumbers_sorted( int value, const char *str ) 
{
	cout << "Called: find_allNumbers_sorted." << endl;
	return -1;
}

int find_allNumbers_unsorted( int value, const char *str )
{
	cout << "Called: find_allNumbers_unsorted." << endl;
	return -1;
}

int find_mixedWithChars_alphabetized( int value, const char *str )
{
	cout << "Called: find_mixedWithChars_alphabetized." << endl;
	return -1;
}

int find_mixedWithChars_unalphabetized( int value, const char *str )
{
	cout << "Called: find_mixedWithChars_unalphabetized." << endl;
	return -1;
}

//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
int main( int argc, const char *argv[] )
{
	int valueToSearchFor = 25;
	const char *stringToSearch = "This string contains the value 25!";

	//
	// Now, lets test are dispatch table by using its function pointers to call 
	// the four special search algorithms we support.
	//

	for( int i = 0; i < 4; ++i )
	{
		dispatchTable[i].funcPtr( valueToSearchFor, stringToSearch );
	}

    return 0;
}

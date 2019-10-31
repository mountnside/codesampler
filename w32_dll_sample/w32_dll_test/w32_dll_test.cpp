//-----------------------------------------------------------------------------
//           Name: w32_dll_test.cpp
//         Author: Kevin Harris
//  Last Modified: 09/23/04
//    Description: This is the source file for the application that I use to
//                 test my DLL sample project.
//
//                 To use our new DLL, we need to include the header file which
//                 was used to compile it, and place the resulting .lib and 
//                 .dll file in this workspace so our test app can call the 
//                 functions exported by them.
//-----------------------------------------------------------------------------

#include "w32_dll_source.h" // Don't forget to include the header file of our new DLL!

#include <iostream>
#include <string>
using namespace std;

void main()
{
    string str = "100";
	int  i = 200;

	cout << "This string value was sent to the DLL: " << str << endl;

	// Call a DLL function and have our string converted into a number!
	int dllReturn = StringToNumber( &str );
    
	cout << "This integer value was returned by the DLL: " << dllReturn << endl;
	cout << "This integer value was sent to the DLL: " << i << endl;


	// Call a DLL function and have our number converted into a string!
    string str2;

    NumberToString( i, &str2 );

	cout << "This string value was returned by the DLL: " << str2 << endl;
	cout << "Demonstration of using a DLL is complete!" << endl;
}

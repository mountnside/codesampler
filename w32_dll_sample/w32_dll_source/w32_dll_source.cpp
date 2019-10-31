//-----------------------------------------------------------------------------
//           Name: w32_dll_source.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This is the source file for the DLL.
//
//                 Once you've compiled the DLL, take the header file and 
//                 resulting .lib, .dll, over to the w32_dll_test project, 
//                 which will test the DLL by trying to calling some of the   
//                 DLL functions from a simple console app.
//-----------------------------------------------------------------------------

#include "w32_dll_source.h"

#include <string>
using namespace std;

int StringToNumber( string *str )
{
	int value = atoi( str->c_str() );

	return( value );
}

void NumberToString( int value, string *str )
{
    char charBuffer[10];

    itoa( value, charBuffer, 10 );

    *str = charBuffer;
}


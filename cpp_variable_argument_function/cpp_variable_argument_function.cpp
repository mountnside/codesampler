//-----------------------------------------------------------------------------
//           Name: cpp_variable_argument_function.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Example of a function that uses a variable argument list.
//-----------------------------------------------------------------------------

#include <stdarg.h>

#include <iostream>
using namespace std;

//-----------------------------------------------------------------------------
// Name: average()
// Desc: This function will calculate the average of all numbers pased in 
//       until it finds a -1 which acts  as the terminator of this function's 
//       varaible length arguement list.
//-----------------------------------------------------------------------------
int average( int nFirst, ... )
{
	int nNext    = 0;
	int nSum     = 0;
	int nCount   = 0;
	int nAverage = 0;
	va_list marker;

	va_start( marker, nFirst ); // Initialize variable arguments.

	nSum = nFirst;

	while( nNext != -1 ) 
	{
		++nCount;
		nSum += nNext;
		nNext = va_arg( marker, int );
	}

	if( nCount != 0 )
		nAverage = nSum/nCount;

	va_end( marker ); // Reset variable arguments. 

	return( nAverage );
}

//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
void main()
{
	int nAverage = 0;

	nAverage = average( 10, 15, 30, 25, -1 ); // Average should = 20!

    cout << "Average = " << nAverage << endl;
}

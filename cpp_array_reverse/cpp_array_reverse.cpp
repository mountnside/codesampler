//-----------------------------------------------------------------------------
//           Name: cpp_array_reverse.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: A simple function for reversing the order of an array 
//                 of chars.
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

void reverse( char *pBuffer );

//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
void main()
{
	char string1[11] = "1234567890";  // Test an even array
	char string2[12] = "1234567890X"; // Test an odd array

	cout << string1 << endl;
	reverse( string1 );
	cout << string1 << endl;

	cout << string2 << endl;
	reverse( string2 );
	cout << string2 << endl;
}


//-----------------------------------------------------------------------------
// Name: reverse()
// Desc: Reverses the order of a char array.
//-----------------------------------------------------------------------------
void reverse( char *pBuffer )
{
	int i = 0;
	int j = 0;
	int nLength = 0;
	int nHalf = 0;
	char chTemp = 0;

	while(pBuffer[i] != '\0') ++i;

	nLength = i;
	nHalf =  nLength / 2;
	j = nLength - 1;

	for(i = 0; i < nHalf; ++i)
	{
		chTemp = pBuffer[i];
		pBuffer[i] = pBuffer[j];
		pBuffer[j] = chTemp;
		--j;
	}
}
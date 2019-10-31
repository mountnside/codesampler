//-----------------------------------------------------------------------------
//           Name: cpp_random_number.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: While random1 is typical of random number generations,
//                 the function random2() returns a random number which is 
//                 is chosen by shuffling the elements of a vector which 
//                 only contains values that exist between the specified 
//                 range.
//-----------------------------------------------------------------------------

#include <time.h>

#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------

int random1( int nMin, int nMax );
int random2( int nMin, int nMax );


//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
void main()
{
    int i = 0;

    // Seed the random-number generator with the current time 
    // so that the numbers will be different every time we run.
    srand( (unsigned)time( NULL ) );
   
    cout << "Using Random1 whith typical modulus ranging" << endl << endl;

	for( i = 0; i < 10; i++ )
		cout << random1( 1, 100 ) << endl;

    cout << endl;
    cout << "Using Random2 with vector shuffling" << endl << endl;

    for( i = 0; i < 10; i++ )
		cout << random2( 1, 100 ) << endl;
}


//-----------------------------------------------------------------------------
// Name: random1()
// Desc: Generates a random number between specified min/max boundaries
//-----------------------------------------------------------------------------
int random1( int nMin, int nMax )
{
    int nRange = nMax - nMin;
    int nNum = rand() % nRange;
    return( nNum + nMin );
}


//-----------------------------------------------------------------------------
// Name: random1()
// Desc: Generates a random number between specified min/max boundaries using
//       a vector to shuffle.
//-----------------------------------------------------------------------------

int random2( int nMin, int nMax )
{
	vector<int> vi;
	static int nIndex = 0;

	for( int i = nMin; i < nMax; i++ )
		vi.push_back(i);

	random_shuffle( vi.begin(), vi.end() );

	return( vi[nIndex] );
}


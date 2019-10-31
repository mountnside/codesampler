//-----------------------------------------------------------------------------
//           Name: cpp_bit_array.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Win32 console that drives a series of function calls for 
//                 manipulating an array of chars as if they were an array of 
//                 individual bits.
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

void setBit( unsigned char *pBitArray, int nBit );
void clearBit( unsigned char *pBitArray, int nBit );
bool getBit( const unsigned char *pBitArray, int nBit );
void dumpBits( unsigned char *pBitArray, int nSize );

void main()
{
	unsigned char bitArray[5]; // An Array of 5 bytes or 40 bits
	int i = 0;

	memset( bitArray, 0, 5 );
	
	dumpBits( bitArray, 5 );

	// Test SetBit() by setting them one by one...
	for( i = 0; i < 40; ++i )
	{
		setBit( bitArray, i );
	}

	dumpBits( bitArray, 5 );

	// Test ClearBit() by clearing them one by one...
	for( i = 0; i < 40; ++i )
	{
		clearBit( bitArray, i );
	}

	dumpBits( bitArray, 5 );
}

void setBit( unsigned char *pBitArray, int nBit )
{
	int nByteIndex = 0;
	int nBitOffset = 0;
	int nMask = 1;

	nByteIndex = nBit / 8;
	nBitOffset = nBit - ( nByteIndex * 8 );

	nMask <<= nBitOffset;

	pBitArray[nByteIndex] = pBitArray[nByteIndex] | nMask;
}

bool getBit( const unsigned char *pBitArray, int nBit )
{
	int nByteIndex = 0;
	int nBitOffset = 0;
	int nMask = 1;
	int nResultMask = 0;
	bool bSet = false;

	nByteIndex = nBit / 8;
	nBitOffset = nBit - ( nByteIndex * 8 );

	nMask <<= nBitOffset;

	nResultMask = pBitArray[nByteIndex] & nMask;

	if( nResultMask == nMask )
		bSet = true;

	return( bSet );
}

void clearBit( unsigned char *pBitArray, int nBit )
{
	int nByteIndex = 0;
	int nBitOffset = 0;
	int nMask = 1;

	nByteIndex = nBit / 8;
	nBitOffset = nBit - ( nByteIndex * 8 );

	nMask <<= nBitOffset;

	pBitArray[nByteIndex] = pBitArray[nByteIndex] ^ nMask;
}

void dumpBits( unsigned char *pBitArray, int nSize )
{
	bool state = false;
	int nCount = 0;

	for( int i = 0; i < nSize; ++i )
	{
		for( int j = 0; j < 8; ++j )
		{
			state = getBit( pBitArray, nCount );

			if( nCount < 10 )
				cout << " " << nCount << " = " << state << "   ";
			else
				cout << nCount << " = " << state << "   ";

			++nCount;
		}
		cout << endl;
	}
	cout << endl;
}




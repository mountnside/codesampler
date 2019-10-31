//-----------------------------------------------------------------------------
//           Name: cpp_quicksort_strings.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Sample quick sort algorithm for sorting strings.
//-----------------------------------------------------------------------------

#include <string>
#include <iostream>
using namespace std;

//-----------------------------------------------------------------------------
// PROTOTYPES
//-----------------------------------------------------------------------------

void q_sort( char *array, int left, int right );
void swap( char *array, int i, int j );

//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
void main()
{
   int i;
   char *array[] ={ "C", "A", "H", "J", "I", "B", "D", "F", "E", "G" };

   cout << "Before Sort" << endl;
   cout << endl;

   for( i = 0; i < 10; ++i )
	   cout << "Index " << i << " = " << array[i] << endl;

   q_sort( (char*)array, 0, i-1 );

   cout << endl;
   cout << "After Sort" << endl;
   cout << endl;
   
   for( i = 0; i < 10; ++i )
	   cout << "Index " << i << " = " << array[i] << endl;

   cout << endl;
}


//-----------------------------------------------------------------------------
// Name: q_sort()
// Desc: The function q_sort takes three arguments: The first argument is the 
//       input array of strings to be sorted. This will also contain the 
//       sorted array. The second argument and third arguments define the 
//       part of the input list that this particular call to q_sort will 
//       handle The second argument gives the starting position of the list.  
//       The third argument gives the last position of list. Notice how 
//       different calls to q-sort all work on the same array but on different 
//       parts of the array.
//-----------------------------------------------------------------------------
void q_sort( char *array, int left, int right )
{
    int current;
    int last;
    int size = sizeof(char*);

    if( left >= right )
    return;

    // Pick a number in the middle of the list as the sort's pivot point and 
    // move it into a safe position at the far leftmost portion of the array.
    // We'll also make use of a temp marker for swapping purposes called 
    // "last" which will also start off at the far "left" , but will be moved  
    // over to the right side with every swap performed.
    swap( array, left, (left+right)/2 );
    last = left; 

    // Now, starting at the second one from the "left", begin the sorting 
    // process by swapping any "current" values that are smaller than the "left".
    for( current = left+1; current <= right; ++current )
    {
        // If current is less than "left", swap "last" and "current"
        if( stricmp( *(const char**)(array+size*current),
                     *(const char**)(array+size*left   ) ) < 0 )
        {
            ++last;
            swap( array, last, current );
        }
    }

    // While sorting may not be finished yet for the entire list, we do know 
    // that all strings less than "left" are on the left side of the array and 
    // all strings that are larger remained on the right side. All we have to 
    // do now is swap "left" with where "last" is pointing and our pivot point
    // will be in it's proper place.
    swap( array, left, last );

    // Now, repeat the process recursively by sorting the two groups of 
    // strings that now exist on either side of our old pivot point.
    q_sort( array, left  , last-1 );
    q_sort( array, last+1, right  );
}


//-----------------------------------------------------------------------------
// Name: swap()
// Desc: The function swap interchanges the values in two positions of the 
//       array. The function takes three arguments: The first argument is 
//       the input array of strings. The second and third arguments specify 
//       which elements of the array need to be swapped.
//-----------------------------------------------------------------------------
void swap( char *array, int i, int j )
{
    char *temp[5];
    int size = sizeof(char*);

    memcpy( temp          , array+(size*i), size );
    memcpy( array+(size*i), array+(size*j), size );
    memcpy( array+(size*j), temp          , size );
}

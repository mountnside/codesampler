//-----------------------------------------------------------------------------
//           Name: cpp_function_template_1.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This sample demonstrates a situation in which the usage of
//                 a template function would be ideal. See the accompanying 
//                 sample, named "cpp_function_template_2" for the actual  
//                 template function solution.
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

// This is the first or original add() function, which takes and returns ints.
int add( const int arg1, const int arg2 )
{
    return arg1 + arg2;
}

// This one is our first overloaded version of add(), which takes floats.
float add( const float arg1, const float arg2 )
{
    return arg1 + arg2;
}

// And this one overloads it again for doubles.
double add( const double arg1, const double arg2 )
{
    return arg1 + arg2;
}

// We now have three functions, which do the exact same thing. They only 
// differ by the types used. What a shame!

//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
void main( void )
{
    // Call the original version of add(), which takes and returns ints.
    const int sum1 = add(3, 2);
    cout << "sum1 = " << sum1 << endl;

    // Call the overloaded version of add(), which takes and returns floats.
    const float sum2 = add(3.1f, 2.4f);
    cout << "sum2 = " << sum2 << endl;

    // Call the overloaded version of add(), which takes and returns doubles.
    const double sum3 = add(3.1, 2.4);
    cout << "sum3 = " << sum3 << endl;
}

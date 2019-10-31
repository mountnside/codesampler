//-----------------------------------------------------------------------------
//           Name: cpp_function_template_2.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This sample demonstrates how to write a template function.
//                 See the accompanying sample, named "cpp_function_template_1" 
//                 for the initial problem.
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

// Instead of writing three overloaded versions of add(), as we did in 
// "cpp_function_template_1", we'll write one template function which will 
// automatically generate and call the correct version of add() for us.
//
// Note how "T" stands in as a substitute for the unknown type.

template<class T>
const T add(const T &t1, const T &t2)
{
    return t1 + t2;
}

//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
void main( void )
{
    // Template function automatically uses the version of add(), which takes and returns ints.
    const int sum1 = add(3, 2);
    cout << "sum1 = " << sum1 << endl;

    // Template function automatically uses the version of add(), which takes and returns floats.
    const float sum2 = add(3.1f, 2.4f);
    cout << "sum2 = " << sum2 << endl;

    // Template function automatically uses the version of add(), which takes and returns doubles.
    const double sum3 = add(3.1, 2.4);
    cout << "sum3 = " << sum3 << endl;
}

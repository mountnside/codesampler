//-----------------------------------------------------------------------------
//           Name: cpp_template_arguments.cpp
//         Author: Kevin Harris
//  Last Modified: 01/18/07
//    Description: This sample is an in-depth demonstration of how arguments to 
//                 tempalate parameters work with respect to instantiation.
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

//------------------------------------------------------------------------------
// To demonstrate a problem that many templates suffer from with regard to 
// argument types, we'll create a simple template that adds two numbers of 
// type T.
//------------------------------------------------------------------------------

template < typename T >
const T add( const T &arg1, const T &arg2 )
{
    return arg1 + arg2;
}

//------------------------------------------------------------------------------
// As a fix, this overloaded version of the "add" function template allows two 
// different types to be passed by declaring two unique typenames.
//------------------------------------------------------------------------------
/*
template < typename T1, typename T2 >
const T1 add( const T1 &arg1, const T2 &arg2 )
{
    return arg1 + arg2;
}
//*/

//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
void main( void )
{
    // Here, our template instantiates a version of add(), which takes and returns ints.
    const int sum1 = add(3, 2);
    cout << "sum1 = " << sum1 << endl;

    // Here, our template instantiates a version of add(), which takes and returns floats.
    const float sum2 = add(3.1f, 2.4f);
    cout << "sum2 = " << sum2 << endl;

    // However, passing two different types to this template function will 
    // result in a compile-time error because it will not know what type to 
    // pick for "T". For example, if we pass a double and an int, we'll get an 
    // compile-tme error.
    //const double sum3 = add( 3.1, 2 ); // Uncomment this line for an error!

    // The simplest way around this is to cast so both arguments are the same..
    const double sum4 = add( 3.1, static_cast<double>(2) );

    // Or, we can fully "qualify" the function template and force the compiler 
    // to cast where necessary.
    const double sum5 = add<double>( 3.1, 2 );

    // Lastly, if we really want the two arguments to be different, we must 
    // define a new template that allows this. To compile this sample correctly
    // uncomment the second template definition at the top of this file.
}

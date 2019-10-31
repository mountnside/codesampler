//------------------------------------------------------------------------------
//           Name: cpp_template_inclusion_1.cpp
//         Author: Kevin Harris
//  Last Modified: 01/18/07
//    Description: This sample demonstrates how the naive attempt to define 
//                 templates like regular C++ code (seperate source and header 
//                 files) will compile - but fail during the linking stage.
//
//                 The reason for this linker error is that the template 
//                 function "printTypeofInfo" was not instantiated in any way 
//                 while the compiler was processing "myTemplate.h" and 
//                 "myTemplate.cpp" into a compilation unit. Later, when 
//                 "cpp_template_inclusion_1.cpp" is being processed, 
//                 information concerning how the template is instantiated is 
//                 no longer available because it has been compiled into the 
//                 compilation unit for "myTemplate" and the header file 
//                 "myTemplate.h" doesn't contain enough information to allow 
//                 a complete instantiation.
//
//                 While this sample is intended to demonstrate the linker 
//                 error, it does contain two quick fixes for the problem which 
//                 are of note. The better way to handle this problem is 
//                 demonstrated in the next sample titled, 
//                 "cpp_template_inclusion_2".
//------------------------------------------------------------------------------

#include <iostream>
#include "myTemplate.h"

//------------------------------------------------------------------------------
// FIX #1
//
// One way to fix this linker error is to provide the template function's 
// definition in every .cpp file that needs to instantiate it, but this is 
// messy and not typically used.
//------------------------------------------------------------------------------
//#include "myTemplate.cpp"

int main( void )
{
    double temp = 12.0;
    printTypeofInfo( temp );  // Call function template for type double.
    
    return 0;
}

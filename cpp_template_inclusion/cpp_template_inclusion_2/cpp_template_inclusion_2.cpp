//------------------------------------------------------------------------------
//           Name: cpp_template_inclusion_2.cpp
//         Author: Kevin Harris
//  Last Modified: 01/18/07
//    Description: This sample demonstrates how to fix the template linker 
//                 issue demonstrated in the "cpp_template_inclusion_1" sample.
// 
//                 Of course, the "inclusion method" of handling templates has 
//                 two downsides.
//
//                 1. Our implementation is now exposed for all to see.
//                 2. The compile-time cost of including our template's header
//                    has increased considerably. This is especially true if 
//                    we need to pull in other headers to implement our 
//                    template.
//------------------------------------------------------------------------------

#include <iostream>
#include "myTemplate.h"

void main( void )
{
    double temp = 12.0;
    printTypeofInfo( temp );  // Call function template for type double.
}

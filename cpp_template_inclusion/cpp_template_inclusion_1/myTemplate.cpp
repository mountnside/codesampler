#include <iostream>
#include <typeinfo>
#include "myTemplate.h"

//------------------------------------------------------------------------------
// For "explicitly instantiate" to work on Linux, you must do it in the 
// source file!
//------------------------------------------------------------------------------

//template void printTypeofInfo<double>( double const& );

//------------------------------------------------------------------------------
// Private implementation and definition of our template.
//------------------------------------------------------------------------------

template <typename T>
void printTypeofInfo( T const& x )
{
    std::cout << typeid(x).name() << std::endl;
}

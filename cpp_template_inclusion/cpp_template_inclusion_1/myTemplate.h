#ifndef MY_TEMPLATE_H
#define MY_TEMPLATE_H

//------------------------------------------------------------------------------
// Declaration of my template in a public header file.
//------------------------------------------------------------------------------

template <typename T>
void printTypeofInfo( T const& );

//------------------------------------------------------------------------------
// FIX #2
//
// If we "explicitly instantiate" printTypeofInfo() for type double, our linker
// problem will go away, but this only fixes our linker problem for cases where
// double(s) are used. If the user starts passing int(s), the linker problem 
// will return. Also, be careful performing "explicitly instantiate" on 
// user-defined types. If the user-defined type is defined in another module or 
// .dll, an "explicitly instantiate" based on it will create a circular 
// dependency which is always a bad thing to have in your project.
//
// NOTE: Windows allows "explicitly instantiate" in the header - but not Linux.
//       Linux will only accept it if it goes in the source file.
//------------------------------------------------------------------------------

//template void printTypeofInfo<double>( double const& );

#endif // MY_TEMPLATE_H

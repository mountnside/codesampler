//-----------------------------------------------------------------------------
//           Name: cpp_derived_classes.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This sample demonstrates how to use the OOP 
//                 (Object Oriented Programming) features of C++ to create 
//                 a simple example of derivation. Note how the order of object 
//                 construction and destruction are exposed through cout 
//                 printing to the console.
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

//-----------------------------------------------------------------------------
// A simple base class
//-----------------------------------------------------------------------------
class base_class 
{

public:

	base_class()
    {
	   cout << "base_class constructor called" << endl;
	}

	~base_class()
    {
	   cout << "base_class destructor called" << endl;
	}
};

//-----------------------------------------------------------------------------
// A class that derives from base_class
//-----------------------------------------------------------------------------
class derived_class : public base_class 
{

public:

	derived_class()
    {
	   cout << "derived_class constructor called" << endl;
	}

	~derived_class()
    {
	   cout << "derived_class destructor called" << endl;
	}
};


void main( void )
{
    derived_class var;
}

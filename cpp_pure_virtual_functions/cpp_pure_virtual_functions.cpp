//-----------------------------------------------------------------------------
//           Name: cpp_pure_virtual_functions.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This sample demonstrates how to use the OOP 
//                 (Object Oriented Programming) features of C++ to create 
//                 pure virtual functions.
//
// Classes can be implemented to enforce a protocol. These classes are 
// called "abstract base classes" because no object of the class type can be 
// created. They exist solely for derivation.
//
// Classes are abstract base classes if they contain pure virtual functions 
// or if they inherit pure virtual functions and do not provide an 
// implementation for them. Pure virtual functions are virtual functions 
// declared with the pure-specifier (= 0), as follows:
//
// virtual void foo() = 0;
//
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

//-----------------------------------------------------------------------------
// Class A is considered an Abstract Base Class becuase it has at least one 
// pure virtual function and no objects of class A can be created.
//-----------------------------------------------------------------------------
class A
{

public:

    virtual void foo() = 0; // A pure virutal function
};

//-----------------------------------------------------------------------------
// Class B then derives from A and implements the pure virtual function foo()
//-----------------------------------------------------------------------------
class B: public A 
{

public:

	virtual void foo()
    { 
        cout << "Class B's foo() called..." << endl;
    }
};


main( void )
{
    //A object_a; // Can't create an object of class type A, but...

    B object_b; // class type B is fine becuase it implements foo()

    object_b.foo();
    
    return 0;
}

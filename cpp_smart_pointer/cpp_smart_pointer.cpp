//-----------------------------------------------------------------------------
//           Name: cpp_smart_pointer.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This sample demonstrates how to implement smart pointers.
//                 See the "smartPointer.h" file for the implementation.
//----------------------------------------------------------------------------- 

#include <iostream>
using namespace std;

#include "smartPointer.h"

// Create a simple test class to test our new objectSmartPointer class...
class myTestClass
{
public:

    int   m_nSomeInt;
    float m_fSomeFloat;
};

void main()
{
    //-------------------------------------------------------------------------
    // Instead of creating a standard "dumb" pointer like so...
    //
    // int *pPtrToAnInt;
    //
    // we'll use our new smart pointer template to declare a smart pointer
    // for a single int which can cleanup after itself.
    //-------------------------------------------------------------------------

	smartPointer<int> pPtrToAnInt;

	pPtrToAnInt = new int; // Use new to allocate memory for an a single int.

	pPtrToAnInt = new int; // Oops... did it again! This would normally result 
						   // in a memory leak, but our smart pointer will 
						   // catch this and delete the originally allocated 
						   // memory for us.

	*pPtrToAnInt = 100;

	cout << *pPtrToAnInt << "\n";

	//-------------------------------------------------------------------------
    // Again, instead of creating a standard "dumb" object pointer like so...
    //
    // myTestClass *pPtrToAnObject;
    //
    // we'll use our new smart pointer template to declare a smart pointer
    // for a single instance of myTestClass which can cleanup after itself.
    //-------------------------------------------------------------------------

    objectSmartPointer<myTestClass> pPtrToAnObject;

    pPtrToAnObject = new myTestClass;

    pPtrToAnObject->m_nSomeInt   = 10;
    pPtrToAnObject->m_fSomeFloat = 3.14159265f;

    cout << (*pPtrToAnObject).m_nSomeInt << "\n"; // Access through a dereference
    cout << pPtrToAnObject->m_fSomeFloat << "\n"; // Access through the smart pointer's overloaded "->" operator.
}

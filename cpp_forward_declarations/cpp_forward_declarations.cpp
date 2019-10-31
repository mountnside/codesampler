//------------------------------------------------------------------------------
//           Name: cpp_forward_declarations.cpp
//         Author: Kevin Harris
//  Last Modified: 04/29/05
//    Description: This sample demonstrates how to use forward declarations
//                 to compile classes that exist in a cyclic dependant 
//                 relationship. This basically means that one class can not be 
//                 compiled unless another one is, but that second class can't 
//                 be compiled unless the first one is compiled... and around
//                 around we go forever!
//
//                 As a concrete example, I've created two classes that need to 
//                 know about each other: a class called "myClass" and a 
//                 manager class called "myClassManager" to manage instance of 
//                 "myClass". The class "myClassManager" obviously needs to know 
//                 about "myClass" since it manages objects of that type, but 
//                 what if we want "myClass" to hold a pointer to its manager?
//                 If we do this, we have created a "cyclic dependency" and the 
//                 only way it will compile is to forward declare the classes by
//                 name.
//------------------------------------------------------------------------------

#include "myClass.h"
#include "myClassManager.h"

void main( void )
{
    myClassManager *pClassManager = new myClassManager;

    myClass *pClass1 = new myClass;
    myClass *pClass2 = new myClass;
    myClass *pClass3 = new myClass;

    pClassManager->addInstance( pClass1 );
    pClassManager->addInstance( pClass2 );
    pClassManager->addInstance( pClass3 );

    delete pClassManager;
}
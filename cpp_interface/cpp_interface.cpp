//-----------------------------------------------------------------------------
//           Name: cpp_interface.cpp
//         Author: Kevin Harris
//  Last Modified: 03/09/05
//    Description: While pure virtual functions are useful for enforcing an 
//                 "interface", they should not be confused with a true 
//                 interface mechanism, which can be found in languages like 
//                 Java and C#. The reason I point this out is that under 
//                 certain circumstances there are considerable performance 
//                 penalties with using pure virtual functions to mimic an 
//                 interface.
//
//                 With that said, this sample demonstrates how to create an 
//                 interface class, without having to use an abstract base 
//                 class containing virtual calls.
//
// This sample is based on a C/C++ Users Journal article by 
// Christopher Diggins:
//
// http://www.artima.com/weblogs/viewpost.jsp?thread=77842
// http://www.heron-language.com/cpp-iop.html
//
//-----------------------------------------------------------------------------

#include "myInterface.h"

#include <string>
#include <iostream>
using namespace std;

class myTestClass1
{
public:

    myTestClass1() {}
    ~myTestClass1() {}

    int function1(int n) { return n + 1; }
    int function2(string* s) { return s->length(); }
};

class myTestClass2
{
public:

    myTestClass2() {}
    ~myTestClass2() {}

    int function1(int n) { return n - 1; }
    int function2(string* s) { return -(int)(s->length()); }
};

void main()
{
    myTestClass1 test1;
    myTestClass2 test2;

    string *testString = new string;
    testString->assign( "test" );

    // By assigning the object's created from our test classes to an instance
    // of myInterace, we are forcing those objects to implement an interface 
    // exactly like the class myInterace. Of course, if we call any of the 
    // functions defined by myInterace, the interface will redirect or 
    // dispatch the call to the original object's class for handling.
    myInterface mi = test1;

    cout << "mi.function1( 3 ) = " << mi.function1(3) << endl;
    cout << "mi.function2( \"hi\" ) = " << mi.function2( testString ) << endl;

    mi = test2;
    cout << "mi.function1( 3 ) = " << mi.function1(3) << endl;
    cout << "mi.function2( \"hi\" ) = " << mi.function2( testString ) << endl;
}

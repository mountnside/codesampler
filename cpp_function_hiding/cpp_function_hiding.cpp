//-----------------------------------------------------------------------------
//           Name: cpp_function_hiding.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This sample demonstrates how to use the OOP 
//                 (Object Oriented Programming) features of C++ and focuses 
//                 on how function hiding which can occur through class 
//                 inheritance.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Class A declares a function called foo()
//-----------------------------------------------------------------------------
class A
{
public:

	int foo( int i )
    {
        return i * 2;
    }
};


//-----------------------------------------------------------------------------
// Class B derives from A and also declares a function called foo(), but the 
// function arguments have been changed, so the new foo() completely replaces 
// the old foo in the base making it inaccessible. In other words, foo() has 
// not become overloaded in the sense that both foo()'s are accessible, but 
// instead, the base foo() has become hidden.
//-----------------------------------------------------------------------------
class B: public A 
{
public:

	int foo( int i, int j )
    {
        return i * j;
    }
};


main( void )
{
    B   object;
    int var;

    var = object.foo( 10, 2 );  // Works fine, but...

    //var = object.foo( 10 );   // Uncomment this line to see the error...
                                // Fails! Base foo() has become hidden - not overloaded!

    return 0;
}

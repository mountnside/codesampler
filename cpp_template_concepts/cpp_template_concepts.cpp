//-----------------------------------------------------------------------------
//           Name: cpp_template_concepts.cpp
//         Author: Kevin Harris
//  Last Modified: 01/18/07
//    Description: This sample demonstrates how to use template concepts to 
//                 enforce proper template usage through compile-time errors.
//                 In other words, concepts are like rules which check for, or 
//                 enforce, proper usage at compile-time.
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

//------------------------------------------------------------------------------
// This is a nifty template trick that causes compile-time errors if a 
// certain condition is not met during instantiation. our test templates will
// use these in conjunction with our "concept" templates to enforce a set of 
// rules that constrain the usage of our class template and one of its template 
// method.
//------------------------------------------------------------------------------

template<bool> struct CompileTimeError;
template<> struct CompileTimeError<true>{ enum { value = true }; };

//------------------------------------------------------------------------------
// Our first "concept" is the "allowed_type_concept1" template, which enforces 
// that only certain types may be passed as template arguments to 
// "MyTestTemplate1".
//------------------------------------------------------------------------------

// The primary concept template...
template< typename T >
struct allowed_type_concept1;

// MyTestTemplate is allowed to use float(s)
template<>
struct allowed_type_concept1< float >
{
    enum { valid = true };
};

// MyTestTemplate is allowed to use double(s)
template<>
struct allowed_type_concept1< double >
{
    enum { valid = true };
};

//------------------------------------------------------------------------------
// Our second "concept" is the "allowed_type_concept2" template, which enforces 
// that only certain types may be passed as template arguments to 
// "MyTestTemplate2".
//------------------------------------------------------------------------------

// The primary concept template...
template< typename T >
struct allowed_type_concept2;

// MyTestTemplate2 is allowed to use int(s)
template<>
struct allowed_type_concept2< int >
{
    enum { valid = true };
};

// MyTestTemplate2 is allowed to use short(s)
template<>
struct allowed_type_concept2< short >
{
    enum { valid = true };
};

//------------------------------------------------------------------------------
// Let's create a few user defined types for testing purposes...
//------------------------------------------------------------------------------

struct myCustomType1 {};
struct myCustomType2 {};

//------------------------------------------------------------------------------
// Our last concept is the "allowed_type_pairs_concept" template, which 
// identifies which type pairings are considered valid when we call the method 
// template, doSomethingInvolvingAnotherType().
//------------------------------------------------------------------------------

// The primary concept template...
template< typename T1, typename T2 >
struct allowed_type_pairs_concept;

// float can be mixed with myCustomType1.
template<>
struct allowed_type_pairs_concept< float, myCustomType1 >
{
    enum { valid = true };
};

// double can be mixed with myCustomType1.
template<>
struct allowed_type_pairs_concept< double, myCustomType1 >
{
    enum { valid = true };
};

// int can be mixed with myCustomType2.
template<>
struct allowed_type_pairs_concept< int, myCustomType2 >
{
    enum { valid = true };
};

// short can be mixed with myCustomType2.
template<>
struct allowed_type_pairs_concept< short, myCustomType2 >
{
    enum { valid = true };
};

//------------------------------------------------------------------------------
// MyTestTemplate1 is a simple class template which helps demonstrate two ways 
// to use template concepts.
//------------------------------------------------------------------------------
template< typename T1 >
class MyTestTemplate1
{
public:

    // If this "template class" is ever asked to instantiate code using a type 
    // that it is not specifically allowed to use, a compile-time error will be
    // generated here.
    enum { concept = allowed_type_concept1<T1>::valid };

    template< typename T2 >
    doSomethingInvolvingAnotherType()
    {
        // If this "method template" is ever asked to instantiate code using a 
        // another type which is not allowed, a compile-time error will be
        // generated here.
        CompileTimeError< allowed_type_pairs_concept< T1, T2 >::valid>();
    }
};

//------------------------------------------------------------------------------
// MyTestTemplate2 is a simple class template which helps demonstrate two ways 
// to use template concepts.
//------------------------------------------------------------------------------
template< typename T1 >
class MyTestTemplate2
{
public:

    // If this "template class" is ever asked to instantiate code using a type 
    // that it is not specifically allowed to use, a compile-time error will be
    // generated here.
    enum { concept = allowed_type_concept2<T1>::valid };

    template< typename T2 >
    doSomethingInvolvingAnotherType()
    {
        // If this "method template" is ever asked to instantiate code using a 
        // another type which is not allowed, a compile-time error will be
        // generated here.
        CompileTimeError< allowed_type_pairs_concept< T1, T2 >::valid>();
    }
};

//-----------------------------------------------------------------------------
// Name: main()
// Desc: 
//-----------------------------------------------------------------------------
void main( void )
{
    MyTestTemplate1< float > testA;  // float is allowed for MyTestTemplate1.
    MyTestTemplate1< double > testB; // double is allowed for MyTestTemplate1.
    //MyTestTemplate1< int > testC;   // int is not allowed for MyTestTemplate1.
    //MyTestTemplate1< short > testD; // short is not allowed for MyTestTemplate1.

    testA.doSomethingInvolvingAnotherType< myCustomType1 >();   // An object of MyTestTemplate1 may work with myCustomType1
    //testA.doSomethingInvolvingAnotherType< myCustomType2 >(); // An object of MyTestTemplate1 may NOT work with myCustomType2

    MyTestTemplate2< int > testE;     // int is allowed for MyTestTemplate2.
    MyTestTemplate2< short > testF;   // short is allowed for MyTestTemplate2.
    //MyTestTemplate2< float > testG;  // float is not allowed for MyTestTemplate2.
    //MyTestTemplate2< double > testH; // double is not allowed for MyTestTemplate2.

    testE.doSomethingInvolvingAnotherType< myCustomType2 >();   // An object of MyTestTemplate2 may work with myCustomType2
    //testE.doSomethingInvolvingAnotherType< myCustomType1 >(); // An object of MyTestTemplate2 may NOT work with myCustomType1
}

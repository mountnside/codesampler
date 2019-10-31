//------------------------------------------------------------------------------
//           Name: cpp_template_policies.cpp
//         Author: Kevin Harris
//  Last Modified: 01/18/07
//    Description: This sample demonstrates how to use template policies to 
//                 customize how templates work with certain types.
//
//                 It's important to understand that "policies" are often 
//                 confused with another template technique known as 
//                 "type traits" or just "traits". This confusion is due to 
//                 their implementations which are very similar. The general 
//                 rule-of-thumb is that type traits attach useful bits of 
//                 information to a type which a template can use to alter 
//                 behavior, while policies typically provide a completely 
//                 new code path which is considered more suitable for a type.
//------------------------------------------------------------------------------

#include <iostream>
using namespace std;

//------------------------------------------------------------------------------
// This is a default policy which simply creates an object of type T and passes
// it out.
//------------------------------------------------------------------------------

template< typename T >
class DefaultBuildPolicy
{
public:

    static T* buildObjectThisWay()
    {
        // As a default policy, most objects are built by calling new.
        return new T();
    }
};

//------------------------------------------------------------------------------
// This is a custom policy which creates an object of type T (assumed to be 
// string in this case), sets it to some initial string value, and then passes 
// the object back.
//------------------------------------------------------------------------------

template< typename T >
class StringBuildPolicy
{
public:

    static T* buildObjectThisWay()
    {
        // This is a custom policy for building string objects.
        T *object = new T;
        object->assign( "This is a string!" ); // Here's our custom behavior.
        return object;
    }
};

//------------------------------------------------------------------------------
// This is our template which uses policies to allow customized behavior. 
// It does provide a default policy so users who do not care about policy 
// customization can simply call the template without providing one.
//------------------------------------------------------------------------------

template< typename T, typename POLICY = DefaultBuildPolicy<T> >
class MyObjectBuilder
{
public:

    T* buildObject() const
    {
        return POLICY::buildObjectThisWay();
    }
};

//------------------------------------------------------------------------------
// Name: main()
// Desc: 
//------------------------------------------------------------------------------
void main( void )
{
    // Create a builder that builds int(s).
    MyObjectBuilder<int> intBldr;

    int *myInt1 = intBldr.buildObject();

    // Create a builder that builds strings(s) and use our custom policy to 
    // customize the construction so all strings are set to some initial
    // value.
    MyObjectBuilder< std::string, StringBuildPolicy<std::string> > stringBldr;

    std::string *myString1 = stringBldr.buildObject();

    delete myInt1;
    delete myString1;
}

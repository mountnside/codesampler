//------------------------------------------------------------------------------
//           Name: cpp_type_traits.cpp
//         Author: Kevin Harris
//  Last Modified: 01/18/07
//    Description: This sample demonstrates how to use type traits to attach 
//                 additional information to a particular type used by a 
//                 template.
//
//                 It's important to understand that "type traits" or just 
//                 "traits" are often confused with another template technique 
//                 known as "policies". This confusion is due to their
//                 implementations which are very similar. The general 
//                 rule-of-thumb is that type traits attach useful bits of 
//                 information to a type which a template can use to alter 
//                 behavior, while policies typically provide a completely 
//                 new code path which is considered more suitable for a type.
//
// NOTE: The following sample of type trait usage was inspired by the book
//       "C++ Templates - The Complete Guide" by David Vandevoorde and 
//       Nicolai M. Josuttis, Addison-Wesley, 2002
//------------------------------------------------------------------------------

#include <iostream>

//------------------------------------------------------------------------------
// As a demonstration of why you would use type traits, we'll create a simple 
// template which calculates the sum of an array of some type T.
//
// This is fairly straightforward, but we have potential problem. Our template 
// creates a return type based on T. This works for most types passed in but 
// for 'char' - it's a bad idea. The problem is that char, as a numerical type, 
// is going to be too small to total up an array of chars, especially an array 
// of chars that are being used to store ASCII codes.
//------------------------------------------------------------------------------ 

template < typename T >
inline
T createSum( T const* beg, T const* end )
{
    // Hmmm... our return type is based on type T. There's a potential bug if T 
    // is a char since it's very easy to overflow a char.

    T total = T(); // Assumes () operator creates a zero value, which is true for integral types.

    while( beg != end )
    {
        total += *beg;
        ++beg;
    }

    return total;
}

//------------------------------------------------------------------------------
// To solve our problem involving types like 'char' which make a bad return 
// type, we can create a series of type traits which can assist our template by 
// providing additional information for certain problem types.
//------------------------------------------------------------------------------

// Primary template for our traits...
template< typename T >
struct SummationTrait;

// If the type is 'char', use an 'int' to sum up the values.
template<>
struct SummationTrait< char >
{
    typedef int BestReturnType;
};

// If the type is 'short', use an 'int' to sum up the values.
template<>
struct SummationTrait< short >
{
    typedef int BestReturnType;
};

// If the type is 'int', use a 'long' to sum up the values.
template<>
struct SummationTrait< int >
{
    typedef long BestReturnType;
};

// If the type is 'unsigned int', use an 'unsigned long' to sum up the values.
template<>
struct SummationTrait< unsigned int >
{
    typedef unsigned long BestReturnType;
};

// If the type is 'float', use a 'double' to sum up the values.
template<>
struct SummationTrait< float >
{
    typedef double BestReturnType;
};

//------------------------------------------------------------------------------
// With our type traits established, we can now re-write our summation 
// template to use these new type traits.
//------------------------------------------------------------------------------

template< typename T >
inline
typename SummationTrait<T>::BestReturnType createSum_usingTraits( T const* beg, T const* end )
{
    // Now, our choice of a return type is driven by type traits. This allows us
    // to automatically use the best return type for summing a particular type T.
    typedef typename SummationTrait<T>::BestReturnType BestReturnType;

    BestReturnType total = BestReturnType();  // Assumes () operator creates a zero value, which is true for integral types.

    while( beg != end )
    {
        total += *beg;
        ++beg;
    }

    return total;
}

//------------------------------------------------------------------------------
// Name: main()
// Desc: 
//------------------------------------------------------------------------------
int main()
{
    // Create an array of 5 integer values...
    int num[] = { 1, 2, 3, 4, 5 };

    // Print average value of int(s)...
    std::cout << "The average value of the integers is "
              << createSum( &num[0], &num[5] ) / 5
              << std::endl;

    // Create an array of character values...
    char name[] = "templates";
    int length = sizeof(name)-1;

    // Print average value of char(s)...
    std::cout << "The average value of the characters in \"" << name << "\" is "
              << createSum(&name[0], &name[length]) / length
              << " (not correct)" << std::endl;

    // Print average value of char(s)...
    std::cout << "The average value of the characters in \"" << name << "\" is "
              << createSum_usingTraits(&name[0], &name[length]) / length
              << " (correct)" << std::endl;
}

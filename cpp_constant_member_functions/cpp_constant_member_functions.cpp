//-----------------------------------------------------------------------------
//           Name: cpp_constant_member_functions.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This sample demonstrates how to use the OOP 
//                 (Object Oriented Programming) features of C++ to create 
//                 constant member functions within a class.
//
// Declaring a member function with the const keyword specifies that the 
// function is a "read-only" function that does not modify the object for 
// which it is called.
// 
// To declare a constant member function, place the const keyword after 
// the closing parenthesis of the argument list. The const keyword is 
// required in both the declaration and the definition. A constant member 
// function cannot modify any data members or call any member functions 
// that aren't constant.
//-----------------------------------------------------------------------------

class A
{

public:

    A() { m_value = 0; }

    int  getValue() const;  // A const function is read-only and can not modify member data
    void setValue( int n ); // A regular method has normal write access to all members

private:

    int m_value;

};

int A::getValue() const
{
    // This function can't modify any 
    // members of this class...

    //m_value = 10;   // Uncomment this line to see the error...

    return m_value;
}

void A::setValue( int n )
{
    m_value = n;
}

main( void )
{
    A object;

    return 0;
}

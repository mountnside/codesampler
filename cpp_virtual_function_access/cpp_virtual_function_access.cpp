//-----------------------------------------------------------------------------
//           Name: cpp_virtual_function_access.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This sample demonstrates how to use the OOP 
//                 (Object Oriented Programming) features of C++ and focuses 
//                 on how virtual function access occurs.
// 
// The access control applied to virtual functions is determined by the 
// type used to make the function call. Overriding declarations of the 
// function do not affect the access control for a given type.
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// A simple base class with a typical virtual accessor function.
//-----------------------------------------------------------------------------
class Base
{

public:

    virtual int GetState() { return m_state; }

protected:

    int m_state;
};


//-----------------------------------------------------------------------------
// A derived class then overides the accessor, but then makes it private.
//-----------------------------------------------------------------------------
class Derived : public Base
{

private:

    int GetState() { return m_state; }
};


//-----------------------------------------------------------------------------
// Test access...
//-----------------------------------------------------------------------------
main()
{
    Derived derived;                 // Object of derived type.

    Base    *base_ptr    = &derived; // Pointer to base type.
    Derived *derived_ptr = &derived; // Pointer to derived type.

    int state;

    // Calling GetState like this is valid. (calling base's GetState)
    state = base_ptr->GetState();    
    
    // But this is invlaid since GetState is private here! (calling the derived class' GetState)
    //state = derived_ptr->GetState(); // Uncomment this line to see the error!

    return 0;
}

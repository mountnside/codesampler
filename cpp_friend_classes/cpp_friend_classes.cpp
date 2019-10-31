//-----------------------------------------------------------------------------
//           Name: cpp_friend_classes.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This sample demonstrates how to use the OOP 
//                 (Object Oriented Programming) features of C++ to declare 
//                 friend classes which declare a special trust relationship 
//                 between two classes.
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

//-----------------------------------------------------------------------------
// Note how MyClass grants friend status to the class MyFriend. This will 
// allow MyFriend to access private member data contained within MyClass.
//-----------------------------------------------------------------------------
class MyClass
{
    friend class MyFriend;  // Declare a friend class...

public:

    MyClass()
    {
        m_private = true;
        cout << "m_private = " << m_private << endl;
    }

private:

    bool m_private;
};

//-----------------------------------------------------------------------------
// MyFriend will now use its friend status to modify private member variables
// in MyClass.
//-----------------------------------------------------------------------------
class MyFriend
{

public:

    void change( MyClass myClass )
    {
       myClass.m_private = false; // A friend can access private data...
       cout << "m_private = " << myClass.m_private << endl;
    }
};


main( void )
{
    MyClass  myClass;
    MyFriend myFriend;

    myFriend.change( myClass );
    
    return 0;
}

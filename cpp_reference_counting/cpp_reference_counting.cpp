//-----------------------------------------------------------------------------
//           Name: cpp_reference_counting.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This sample demonstrates how to implement reference 
//                 counting.
//
// Reference counting helps to prevent the types of bugs and crashes caused by 
// "dangling pointers", which can occur when an object is prematurely deleted 
// while other objects are still referencing or using it. In other words, 
// reference counting lets you safely share objects, by having the object 
// decide when it should be deleted and cleaned up. This is typically 
// implemented by having the reference countable object's lifetime controlled 
// indirectly from the outside via a reference counting member variable.
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

class refCounted
{
public:

    refCounted() : m_nRefCount(1) {}

    int ref()
    {
        // Some one has requested the use of this object, so we need to bump 
        // up the reference count to keep track of this new user.
        return ++m_nRefCount;
    }

    int unRef()
    {
        // Be very careful not to use a member variable after deleting the 
        // "this" pointer. Deleting the "this" pointer triggers a call to the 
        // class's destructor. Once the destructor has been called, the member 
        // variables of the object destroyed are in an unknown state. If you 
        // need to use these member variables after their object has been 
        // deleted, cache them in local variables and then do the delete.

        int nCachedRefCount;

        --m_nRefCount;

        nCachedRefCount = m_nRefCount;

        if( nCachedRefCount == 0 )
        {
            // The ref count has reached 0, which means no one is using this 
            // object and we can now safely delete it.
            delete this;
        }

        return nCachedRefCount;
    }

    int getRefCount()
    {
        return m_nRefCount;
    }

private:

    // Making the destructor private keeps other programmers from accidentally 
    // deleting your reference countable object, which should only be deleted 
    // when its reference count hits 0 via a call to unRef().
    ~refCounted() {}

    int m_nRefCount;
};

void main()
{
    refCounted *pRefObject = new refCounted();

    pRefObject->ref();
    cout << "Reference Count = " << pRefObject->getRefCount() << "\n";

    pRefObject->unRef();
    cout << "Reference Count = " << pRefObject->getRefCount() << "\n";
}
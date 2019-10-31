//This unit demonstrates how to properly allocate and destroy arrays
//of C++ objects.

//Related references:
//Scott Meyers, "Effective C++",
//Item 3:  Prefer new and delete to malloc and free.
//Item 5:  Use the same form in corresponding uses of new and delete.
//Item 6:  Use delete on pointer members in destructors.

#include <memory>
#include <iostream>
#include <cassert> //don't leave home without it :)
 
using std::cout;
using std::endl;

class Object {
public:

    static const size_t BufferSize = 128;

    Object() : m_dynamicMemory(NULL)
    {
        cout << "Object::Object" << endl;
        //let's use C-style memory allocation for a change.
        //However, see EC++ Item 3 for possible ramifications
        //of mixing malloc/free with new/delete
        m_dynamicMemory = (char*) malloc(BufferSize);

        //class-level bookkeeping
        s_totalDynamicMemoryUsed += BufferSize;
        s_constructorDestructorCounter++;
    }

    ~Object()
    {
        cout << "Object::~Object" << endl;

        //Note that malloc should be paired with free, not delete.
        free(m_dynamicMemory);

        //class-level bookkeeping
        s_totalDynamicMemoryUsed -= BufferSize;
        s_constructorDestructorCounter--;
    }

    static int getContructorDestructorCounter() 
    {
        return s_constructorDestructorCounter;
    }

    static void printTotalDynamicMemoryUsed()
    {
        cout << "Total dynamic memory allocated by the Object objects is "
             << (int)s_totalDynamicMemoryUsed << endl; 
    }
    
private:

    char*   m_dynamicMemory;

    //a helper data member that keeps track 
    //of how many times the constructor and 
    //destructor has been called
    static int  s_constructorDestructorCounter;

    //note that size_t is the prefered C++ type when counting memory
    static size_t s_totalDynamicMemoryUsed;
};

//don't forget to initialize the static data member
int Object::s_constructorDestructorCounter = 0;
size_t Object::s_totalDynamicMemoryUsed = 0;


void newAndDeleteArrayTest()
{

    //Example in this scope demonstrates proper allocation
    //and deletion of C++ arrays of objects
    {
        cout << endl << "Allocating/destroying an array of 5 Object correctly" << endl;

        assert(Object::getContructorDestructorCounter() == 0);
        //The
        //this call will execute the Object::Object constructor 5 times
        Object* varr = new Object[5];

        //assert that the constructor was, indeed, called 5 times
        assert(Object::getContructorDestructorCounter() == 5);

        Object::printTotalDynamicMemoryUsed();

        //The following call will destroy the array
        //Notice the [] in between!!! The destructor should be called 5 times.
        delete [] varr;

        Object::printTotalDynamicMemoryUsed();

        //assert that we had 5 destructor calls
        assert(Object::getContructorDestructorCounter() == 0);
    }

    //Next, here is a buggy version of the array allocation/deallocation code
    {
        cout << endl << "Allocating/destroying an array of 5 Objects incorrectly" << endl;

        assert(Object::getContructorDestructorCounter() == 0);

        //this call will execute the Object::Object constructor 5 times
        Object* varr = new Object[5];

        Object::printTotalDynamicMemoryUsed();

        //assert that the constructor was, indeed, called 5 times
        assert(Object::getContructorDestructorCounter() == 5);

        //The following call will not destroy the array!
        //Notice that the  [] are missing!
        //Even worse than that, this is a ticket to trouble called
        //"undefined program behavior". 
        //If using the Debug C run-time, the VC++ heap manager will assert.
        //So, there is no way to even uncomment it safely
       
        //delete  varr;

        Object::printTotalDynamicMemoryUsed();

    }
}
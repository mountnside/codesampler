//This unit demonstrates usage of the placement new operator. 
//This technique can be used for avoiding unnecessary dynamic memory allocations
//and implementing memory pools.


//Related references:
//Scott Meyers, "More Effective C++",
//Item 8:  Understand the different meanings of new and delete.
//Item 4:  Avoid gratuitous default constructors.
//Scott Meyers, "Effective C++",
//Item 3:  Prefer new and delete to malloc and free.


#include <memory>
#include <iostream>
#include <cassert> //don't leave home without it :)
 
using std::cout;
using std::endl;

class Object2 {
public:

    static const size_t s_bufferSize = 128;

    Object2()
    {
        cout << "Object2::Object2" << endl;

        for(int cnt = 0; cnt < s_bufferSize; ++cnt)
            m_buffer[cnt] = 'a'; //init to whatever

        s_constructorDestructorCounter++;
    }

    ~Object2()
    {
        cout << "Object2::~Object2" << endl;

        s_constructorDestructorCounter--;
    }

    static int getContructorDestructorCounter() 
    {
        return s_constructorDestructorCounter;
    }
    
    const char* getBuffer() const { return m_buffer;  }

private:

    char   m_buffer[s_bufferSize];

    //a helper data member that keeps track 
    //of how many times the constructor and 
    //destructor has been called
    static int  s_constructorDestructorCounter;
};

//don't forget to initialize the static data member
int Object2::s_constructorDestructorCounter = 0;


void placementNewTest()
{
    cout << endl << "Placement new tests" << endl;

    //using placement new with a dynamically allocated memory block
    {
        //allocate enough dynamic memory to store an object of type Object2
        //We use calloc so that the memory comes filled in with 0
        void* memoryBlock = calloc(1,sizeof(Object2));

        //no objects have been created yet!
        assert(Object2::getContructorDestructorCounter() == 0);

        //now, construct the object. Notice the syntax: we pass the buffer
        //in parentheses
        Object2* obj = new (memoryBlock) Object2();

        //got one object
        assert(Object2::getContructorDestructorCounter() == 1);

        assert(obj->getBuffer()[0] == 'a');

        //How do we destroy? We can't call delete because the memory
        //was NOT allocated with operator new()! So, we call destructor
        //explicitly. Hey, it is a method like any other.
        obj->~Object2();

        //got no objects
        assert(Object2::getContructorDestructorCounter() == 0);

        //can free the memory using the corresponding memory allocation routine
        //Item 3
        free(memoryBlock);
    }

    //using placement new with a memory created on the stack
    //notice that NO dynamic memory allocation takes place in this case.
    //Therefore, we can use placement new for performance tuning optimizations
    //such as memory pools.
    {
        //allocate enough automatic (on-stack) memory to store an object of type Object2
        char memoryBlock[sizeof(Object2)];

        //initialize the memory to 0 values. We don't have to do it per se.
        //However, this will help with an assertion that checks for the 
        //Object2::m_buffer initialization below
        for(int cnt = 0; cnt < sizeof(Object2); ++cnt)
            memoryBlock[cnt] = 0;

        //no objects have been created yet!
        assert(Object2::getContructorDestructorCounter() == 0);

        //now, construct the object. Notice the syntax: we pass the buffer
        //in parentheses
        Object2* obj = new (memoryBlock) Object2();

        //got one object
        assert(Object2::getContructorDestructorCounter() == 1);

        //that was initialized
        assert(obj->getBuffer()[0] == 'a');

        //How do we destroy it? We can't call delete because the memory
        //was NOT allocated with operator new()! So, we call destructor
        //explicitly. Hey, it is a method like any other.
        obj->~Object2();

        //got no objects
        assert(Object2::getContructorDestructorCounter() == 0);

        //the memoryBlock variable is going to be destroyed automatically
        //once we leave this scope
     }
}
#include <string>
using namespace std;

class myInterface
{
public:

    //-------------------------------------------------------------------------
    // This interface class is considered abstract, so object's of this class 
    // can only be constructed through the assignment of another object.
    // The other object is the one that is supposed to implement the interface 
    // defined below.
    //-------------------------------------------------------------------------
    template <class T> myInterface( T& object ) :
        m_object( &object ),
        m_funcTable( &functions<T>::s_funcTable )
    {}

    //-------------------------------------------------------------------------
    // As a simple example, define an Interface that has only two function 
    // calls...
    //-------------------------------------------------------------------------
    int function1( int n )
    {
        return m_funcTable->function1( const_cast<void*>(m_object), n );
    }

    int function2( string* s )
    {
        return m_funcTable->function2( const_cast<void*>(m_object), s );
    }

    //-------------------------------------------------------------------------
    // Function table type
    //-------------------------------------------------------------------------
    struct functionTable
    {
        int (*function1)(void*, int n);
        int (*function2)(void*, string* s);
    };

    //-------------------------------------------------------------------------
    // For a given referenced type T, generates functions for the function 
    // table and a static instance of the table.
    //-------------------------------------------------------------------------
    template <class T>
    struct functions
    {
        static myInterface::functionTable const s_funcTable;

        static int function1( void* p, int n )
        {
            return static_cast<T*>(p)->function1( n ); 
        }

        static int function2( void* p, string* s )
        {
            return static_cast<T*>(p)->function2( s ); 
        }
    };

private:

    void const* m_object;
    functionTable const* m_funcTable;
};

template <class T>
myInterface::functionTable const 
myInterface::functions<T>::s_funcTable = 
{
    &myInterface::functions<T>::function1, 
    &myInterface::functions<T>::function2
};




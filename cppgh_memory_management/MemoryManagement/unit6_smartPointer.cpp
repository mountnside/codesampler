//The unit demonstrates how the "smart pointer" idiom can be used together
//with reference counting. Smart pointer and reference counting seemed to be
//made for each other!

//Related references:
//Scott Meyers, "More Effective C++",
//Item 28:  Smart pointers.
//
//Boost.org, smart pointer library.
// 
//See Glossary at http://www.memorymanagement.org/

#include <memory>
#include <iostream>
#include <cassert> //don't leave home without it :)
 
using std::cout;
using std::endl;

//to avoid possible name collisions with other units
namespace unit6 {

//see unit5_refernceCounting for more details on reference counting.
class ReferenceCountable {
public:

    ReferenceCountable() : m_counter(0) {}

    //the method is virtual in case a derived class needs to
    //do something special while performing the operation.
    //Also, the method is const so that we could ref/unref an object
    //that is pointed at by a const pointer.
    virtual int  ref() const { return ++m_counter; }

    //the method is virtual in case a derived class needs to
    //do something special while performing the operation.
    //The method will destroy the object once the counter reaches 0.
    //Also, the method is const so that we could ref/unref an object
    //that is pointed at by a const pointer.
    virtual int  unref() const 
    { 
        --m_counter;

        if(m_counter <= 0) {

            int ret = m_counter;

            //YES, the object is being destroyed
            delete this;

            return ret;
        }
        return m_counter;
    }

    int getRef() const { return m_counter; }

protected:

    //yes, protected and virtual.
    //Virtual because of inheritance.
    //Protected because of reference counting - a user should only
    //be able to destroy the object via the unref call. Otherwise,
    //the entire reference counting system falls apart.
    virtual ~ReferenceCountable() {}

private:

    //mutable because ref/unref are const methods.
    mutable int m_counter;
};


//A smart pointer class that holds a reference countable object
template<class T>
class smartPointer {
public:

    typedef T  pointer;

    /**
    * Constructor.  Automatically ref's the provided pointer.
    */
    smartPointer(pointer p = NULL) : m_pointee(p)
    {        
        if(m_pointee)
            m_pointee->ref();
    }

    /**
    * Copy Constructor.  Invokes smartPointer::swap on the provided pointer.
    */
    smartPointer(const smartPointer& src) : m_pointee(NULL)
    {
        swap(src.m_pointee);
    }

    smartPointer& operator=(pointer p)
    {
        if(m_pointee == p) return *this;

        swap(p);

        return *this;
    }

    smartPointer& operator=(const smartPointer& src)
    {
        if(*this == src) return *this;

        swap(src.m_pointee);

        return *this;
    }


    /**
    * Destructor.  Automatically unref's the pointer stored by this smartPointer.
    */
    ~smartPointer()
    {
        if(m_pointee)
            m_pointee->unref();
    }

    /**
    * Retrieves the pointer stored by this smartPointer.
    */
    pointer get() const { return m_pointee; }

    /**
    * Retrieves the pointer stored by this smartPointer.
    */
    pointer operator->() const { return m_pointee; }

    /**
    * Comparison operator
    */
    friend bool operator==(const smartPointer<T> &lhs, const T rhs) 
    {
        return(lhs.m_pointee == rhs);
    }

    /**
    * Comparison operator
    */
    friend bool operator==(const T lhs, const smartPointer<T> &rhs) 
    {
        return(lhs == rhs.m_pointee);
    }

    /**
    * Comparison operator
    */
    friend bool operator==(const smartPointer<T> &lhs,const smartPointer<T> &rhs)
    {
        return(lhs.m_pointee == rhs.m_pointee);
    }

    /**
    * Equality operator
    */
    friend bool operator!=(const smartPointer<T> &lhs, const T rhs) {
        return(lhs.m_pointee != rhs);
    }

    /**
    * Equality operator
    */
    friend bool operator!=(const T lhs, const smartPointer<T> &rhs) {
        return(lhs != rhs.m_pointee);
    }

    /**
    * Equality operator
    */
    friend bool operator!=(const smartPointer<T> &lhs, const smartPointer<T> &rhs)
    {
        return(lhs.m_pointee != rhs.m_pointee);
    }

    /**
    * Inequality operator
    */
    friend bool operator<(const smartPointer<T> &lhs,const T rhs) 
    {
        return(false);
    }

    /**
    * Inequality operator
    */
    friend bool operator<(const T lhs, const smartPointer<T> &rhs) 
    {
        return(false);
    }

    /**
    * Inequality operator
    */
    friend bool operator<(const smartPointer<T> &lhs,const smartPointer<T> &rhs)
    {
        return(false);
    }

private:

    pointer   m_pointee;

    /**
    * Performs a reference-safe swap between the source pointer, and that stored
    * by the field
    */
    bool    swap(pointer src)
    {
        if( src == m_pointee) return false;

        if(m_pointee) m_pointee->unref();
        m_pointee = src;
        if(m_pointee) m_pointee->ref();

        return true;
    }
};



//A resource could be a material or a texture
class Resource1 : public ReferenceCountable {
public:

    Resource1() : m_data(0)
    {
        cout << "Resource1::Resource1" << endl;
    }

    
protected:

    //see comment for ~ReferenceCountable() why virtual and protected
    virtual ~Resource1()
    {
        cout << "Resource1::~Resource1" << endl;
    }

private:

    int m_data;

};

//A resource that has a reference to another resource, i.e.
//a graphics state object that refers to a texture.
//The data member is a smart pointer. Notice how the implementation
//is greatly simplified compared with that of unit5.
class Resource2 : public ReferenceCountable {
public:

    //Initialization of m_resource1 is automated by smartPointer
    Resource2()
    {
        cout << "Resource2::Resource2" << endl;
    }

    void  setResource1(Resource1* r)
    {
        //NULL checks and reference counting are done by smart pointer
        m_resource1 = r;
    }

    Resource1* getResource1() const {

        return m_resource1.get();
    }

protected:

    //m_resource1 is automatically unref'ed by smart pointer.
    virtual ~Resource2()
    {
        cout << "Resource2::~Resource2" << endl;
    }

private:

    smartPointer<Resource1*>   m_resource1;
};
} //namespace unit6

void smartPointerTest()
{
    using namespace unit6;

    cout << endl << "Smart pointer tests" << endl;

    {
        cout << endl << "Testing smart pointer as a data member" << endl;

        Resource1* r1 = new Resource1();

        assert(r1->getRef() == 0);

        Resource2* r2 = new Resource2();

        assert(r2->getRef() == 0);

        r2->setResource1(r1);

        assert(r1->getRef() == 1);

        //The following call will destroy  both resources. 
        r2->unref();
    }

    //another technique that shows smart pointer in a scope
    {
        cout << endl << "Testing smart pointer in a scope" << endl;

        //this call increments reference counter
        smartPointer<Resource2*>  r2 = new Resource2();

        assert(r2->getRef() == 1);

        Resource1* r1 = new Resource1();

        assert(r1->getRef() == 0);

        r2->setResource1(r1);

        assert(r1->getRef() == 1);

        //there is no need to unref r2: 
        //smartPointer<Resource2*>::~smartPointer<Resource2*> will do it
        //upon us leaving this scope.
        //Therefore, r2 is cleaned up automatically.
    }
}

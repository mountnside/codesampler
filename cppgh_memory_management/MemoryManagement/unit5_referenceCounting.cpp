//The unit demonstrates one of many numerous ways of implementing 
//reference counting. Here, we use a common base class. You can also use
//a so-called in-band memory header paired with an allocator.

//Related references:
//Scott Meyers, "More Effective C++",
//Item 27:  Requiring or prohibiting heap-based objects.
//Item 29:  Reference counting.
//
//Scott Meyers, "Counting Objects in C++"
// 
//See Glossary at http://www.memorymanagement.org/
//
//TODO: MULTI-THREAD SAFETY! Modifications of the 
//ReferenceCountable::m_counter are not thread-safe/atomic!

#include <memory>
#include <iostream>
#include <cassert> //don't leave home without it :)
 
using std::cout;
using std::endl;

//to avoid possible name collisions with other units
namespace unit5 {

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

            //YES, the object is being des
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
class Resource2 : public ReferenceCountable {
public:

    Resource2() : m_data(0.0), m_resource1(NULL)
    {
        cout << "Resource2::Resource2" << endl;
    }

    void  setResource1(Resource1* r)
    {
        if(m_resource1 == r) return;

        //first, unref the old resource
        if(m_resource1)
            m_resource1->unref();

        //next, ref the new resource
        m_resource1 = r;
        
        if(m_resource1)
            m_resource1->ref();
    }

    Resource1* getResource1() const {

        return m_resource1;
    }

protected:

    //see comment for ~ReferenceCountable() why virtual and protected
    virtual ~Resource2()
    {
        cout << "Resource2::~Resource2" << endl;

        //release the resource
        if(m_resource1) 
            m_resource1->unref();
    }

private:

    Resource1*   m_resource1;

    double m_data;

};

} //namespace unit5

void referenceCountingTest()
{
    using namespace unit5;

    cout << endl << "Reference counting tests" << endl;

    Resource1* r1 = new Resource1();

    Resource2* r2 = new Resource2();

    r2->setResource1(r1);

    //the following lines simply wouldn't compile
    //delete r1;
    //delete r2;

    //The following call will destroy  both resources. 
    //It works as a chain reaction, where r2 unrefs and,
    //therefore, destroys r1.
    r2->unref();
}

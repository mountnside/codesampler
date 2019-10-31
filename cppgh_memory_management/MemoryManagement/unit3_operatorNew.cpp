//The unit demonstrates how to write operators new/delete for a user class. 
//This technique can be used in conjunction with other optimization
//techniques such as 
// - optimizing performance by eliminating bottlenecks associated with 
//   dynamic memory allocations.
// - implementing custom allocators for reference counting

//Related references:
//Scott Meyers, "More Effective C++",
//Item 10:  Write operator delete if you write operator new.
//Item 14:  Make sure base classes have virtual destructors.
//
//What's missing? Implementing operator new and operator delete for arrays.

#include <memory>
#include <iostream>
#include <cassert> //don't leave home without it :)
 
using std::cout;
using std::endl;

namespace unit3 {

class Base {
public:

    Base() : m_data(0)
    {
        cout << "Base::Base" << endl;
    }

    //this is a base class, therefore destructor is virtual
    virtual ~Base()
    {
        cout << "Base::~Base" << endl;
    }
    
    void * operator new(size_t size)
    {
        cout << "Base::operator new() is called for " << (int)size 
             << " bytes." << endl;

        //calling the global new
        return ::operator new(size);
    }

    void operator delete(void *p, size_t size)
    {
        cout <<"Base operator delete() is called with "
             << (int)size << " bytes." << endl;

        if(!p) return;
        
        //calling the global delete
        ::operator delete(p);
    }

private:

    int m_data;

};

class Derived : public Base {
public:

    Derived() : m_data2(0.0)
    {
        cout << "Derived::Derived" << endl;
    }

    virtual ~Derived()
    {
        cout << "Derived::~Derived" << endl;
    }

    //NOTE that Base::operator new and Base::operator delete
    //are inherited!

private:

    //make sizeof(Derived) > sizeof(Base)
    double  m_data2;

};
} //namespace unit3

void operatorNewTest()
{
    using namespace unit3;

    cout << endl << "Operator new tests" << endl;

    cout << endl << "Constructing and destroying an object of type Base " << endl;

    Base* ptrBase = new Base();

    delete ptrBase;

    cout << endl << "Constructing and destroying an object of type Derived " << endl;

    ptrBase = new Derived();

    //this is where virtual destructor helps!
    delete ptrBase;
}
#ifndef _MYCLASS_H_
#define _MYCLASS_H_

#include <iostream>
using namespace std;

#include "myClassManager.h"

// A forward declaration of "myClassManager" allows "myClass" to compile even 
// though "myClassManager" hasn't been defined yet.
class myClassManager; // Comment this out to see an error!

class myClass
{
public:

    myClass() : m_pMyClassManager( NULL ) {}
	~myClass() {}

    void setManager( myClassManager *pManager )
    {
        cout << "myClass::setManager called! (pManager = " << pManager << ")" << endl;

        m_pMyClassManager = pManager;
    }

private:

    // Maintain a pointer to the manager that manages us.
	myClassManager *m_pMyClassManager;
};

#endif // _MYCLASS_H_
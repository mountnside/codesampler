#ifndef _MYCLASSMANAGER_H_
#define _MYCLASSMANAGER_H_

#include <vector>
using namespace std;

#include "myClass.h"

// A forward declaration of "myClass" allows "myClassManager" to compile even 
// though "myClass" hasn't been defined yet.
class myClass; // Comment this out to see an error!

class myClassManager
{
public:

    myClassManager() {}
	~myClassManager() 
    {
        // Free the memory held by each vector entry.
        vector< myClass* >::iterator it  = m_vectorOfMyClasses.begin();
        vector< myClass* >::iterator ite = m_vectorOfMyClasses.end();
        for( ; it != ite; ++it )
            delete (*it);

        // Erase the vector entries.
        m_vectorOfMyClasses.erase( m_vectorOfMyClasses.begin(),
                                   m_vectorOfMyClasses.end() );
    }

    // Take Warning though; while a forward declaration allows you to compile  
    // without fully declaring a class, it does not allow you compile code that 
    // actually makes usage of that class's functionality. In other words, 
    // we can postpone our cyclic dependency problem to a certain degree, but 
    // the compiler can't fake actual functionality. To fix this, we'll have to 
    // move the method's body out of the header file, where its exposed, to a 
    // source file to hide the usage of myClass's setManager() method.
/*
    void addInstance( myClass *pInstance )
    {
        // Ooops... can't call this! A forward declaration knows nothing 
        // about the setManager() method.
        pInstance->setManager( this );

        m_vectorOfMyClasses.push_back( pInstance );
    }
*/

    void addInstance( myClass *pInstance );

private:

    // Maintain a listing of all "myClass" instances.
	vector< myClass* > m_vectorOfMyClasses;
};

#endif // _MYCLASSMANAGER_H_
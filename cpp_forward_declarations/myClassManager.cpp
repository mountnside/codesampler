#include <vector>
using namespace std;

#include "myClassManager.h"

#include <iostream>
using namespace std;

// We have to put this method here. See the header comment for why.
void myClassManager::addInstance( myClass *pInstance )
{
    cout << "myClassManager::addInstance called! (pInstance = " << pInstance << ")" << endl;

    pInstance->setManager( this );

    m_vectorOfMyClasses.push_back( pInstance );
}

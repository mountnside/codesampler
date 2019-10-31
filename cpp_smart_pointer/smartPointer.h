//-----------------------------------------------------------------------------
//           Name: smartPointer.h
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Sample implementation of a smart pointer.
//
// Smart pointers automatically free the memory associated with a pointer when 
// that pointer is deleted, which helps to prevent a common source of memory 
// leaks.
//
// Smart pointers are typically implemented as a templatized class, which 
// automatically delete the memory associated with the class when it goes out 
// of scope. Smart pointers will also catch the accidental memory leaks, which 
// occur when a pointier is allocated to twice with out deleting the first 
// allocation.
//----------------------------------------------------------------------------- 

//-----------------------------------------------------------------------------
// The first class, smartPointer, is for intrinsic types such as int, float, 
// and so forth. It does not supply a "->" operator.
//-----------------------------------------------------------------------------

template <class T> class smartPointer
{
public:

    smartPointer( T* ptr = NULL ) : m_ptr(ptr) {}

    virtual ~smartPointer()
    {
        if( m_ptr != NULL )
            delete m_ptr;

        m_ptr = NULL;
    }

    T &operator*() const
    {
        return *m_ptr;
    }

    T* operator=( T *ptr )
    {
        if( m_ptr != NULL )
            delete m_ptr;

        m_ptr = ptr;
        return m_ptr;
    }

protected:

    T* m_ptr;

private:

    // Copy constructor and = operator are private to prevent pointers from  
    // being copied from one smart pointer to another.
    smartPointer<T>&operator=(smartPointer<T>&bsp)
    {
        return *this;
    }

    smartPointer(smartPointer<T>&bsp) {}
};

//-----------------------------------------------------------------------------
// This second class, objectSmartPointer, derives from smartPointer and 
// provides a "->" operator, which allows a smart pointer to grant normal 
// pointer access to the internally stored object.
//-----------------------------------------------------------------------------

template <class T> class objectSmartPointer : public smartPointer <T>
{
public:

    objectSmartPointer( T* ptr = NULL ) : smartPointer<T>(ptr) {}

    T* operator=(T *ptr)
    {
        return smartPointer<T>::operator=(ptr);
    }

    T* operator->() const
    {
        return m_ptr;
    }

private:

    // Make this private so someone can't use the copy constructor
    objectSmartPointer(objectSmartPointer<T>&osp) {}
};

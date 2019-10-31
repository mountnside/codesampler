//This unit shows how to write a custom STL compatible allocator.
//The code includes two PERFORMANCE WARNINGS that are specific to
//VC++'s implementation of STL.


#include <memory>
#include <new>
#include <iostream>
#include <vector>
#include <cassert> //don't leave home without it :)
 
using std::cout;
using std::endl;

template <class _Tp>
class  myAllocatorSTL {
public:

    typedef _Tp        value_type;
    typedef value_type *       pointer;
    typedef const _Tp* const_pointer;
    typedef _Tp&       reference;
    typedef const _Tp& const_reference;
    typedef size_t     size_type;
    typedef ptrdiff_t  difference_type;


    template <class _Tp1> struct rebind {
        typedef myAllocatorSTL<_Tp1> other;
    };

    template <class _Tp1> myAllocatorSTL(const myAllocatorSTL<_Tp1>&) {}

    myAllocatorSTL() {}

    myAllocatorSTL(const myAllocatorSTL<_Tp>&)  {}

    ~myAllocatorSTL() {}

    pointer address(reference __x) { return &__x; }

    const_pointer address(const_reference __x) const { return &__x; }

    /*
    * __n is permitted to be 0.  The C++ standard says nothing about what
    * the return value is when __n == 0.
    */
    _Tp* allocate(size_type __n, const void* = 0) const 
    {
        cout << "myAllocatorSTL::allocate(" << (int)__n << ")" << endl;
        return __n == 0 ? NULL : (_Tp*)malloc(__n * sizeof(value_type));
    }

    /*
    *__p is permitted to be a null pointer, only if n==0.
    */
    void deallocate(pointer __p, size_type __n) const 
    {
        cout << "myAllocatorSTL::deallocate(" << (int)__n << ")" << endl;
        if(__p != 0) free(__p);
    }

    size_type max_size() const  
    { return size_t(-1) / sizeof(value_type); }

    void construct(pointer __p, const _Tp& __val) const 
    { 
        new (__p) _Tp(__val); 
    }
    void destroy(pointer __p) const { __p->~_Tp(); }

};
template<class _Ty,
class _Other> inline
    bool operator==(const myAllocatorSTL<_Ty>&, const myAllocatorSTL<_Other>&)
{	// test for allocator equality (always true)
    return (true);
}

template<class _Ty,
class _Other> inline
    bool operator!=(const myAllocatorSTL<_Ty>&, const myAllocatorSTL<_Other>&)
{	// test for allocator inequality (always false)
    return (false);
}

//PERFORMANCE WARNING
//another subtlety in VC++'s STL: there is a performance penalty
//for using built-in types with user-defined allocators
//search microsoft.public.vc.stl for
//"std::vector performance with a custom allocator" for more details
namespace std { 

    template<class _Ty, 
    class _Diff, 
    class _Tval> inline 
        void _Uninit_fill_n(_Ty *_First, _Diff _Count, 
        const _Tval& _Val, myAllocatorSTL<_Ty>&, _Scalar_ptr_iterator_tag) 
    {   // copy _Count *_Val to raw _First, using _Al, scalar type 
        fill_n(_First, _Count, _Val); 
    } 


    template<class _Ty, 
    class _Diff, 
    class _Tval> inline 
        void _Uninit_fill_n(_Ty *_First, _Diff _Count, 
        const _Tval& _Val, myAllocatorSTL <const _Ty>&, 
        _Scalar_ptr_iterator_tag) 
    {   // copy _Count *_Val to raw _First, using _Al, scalar type 
        fill_n(_First, _Count, _Val); 
    } 
} 

void allocatorSTLTest()
{
    cout << endl << "Allocator STL test" << endl;

    cout << endl << "Constructing a vector of 10 integers" << endl;

    std::vector<int,myAllocatorSTL<int> >  vec(10);

    //add more integers to trigger additional internal memory allocations
    for(int cnt = 0; cnt < 100; ++cnt)
        vec.push_back(cnt);

    //PERFORMANCE WARNING for VC++ users:
    //If you need to clear the vector, but keep the memory,
    //don't use vec.clear() because it deallocates memory under VC++ 7.1 
    vec.erase(vec.begin(),vec.end()); 

}
//The unit demonstrates a simple allocator implementation that allows for
//memory tracking. This infrastructure can be extended to support memory
//management in a large scale C++ framework via classes of allocators.
//The basic idea is to introduce a user allocator (Base::Allocator) that
//includes an API to install a user callback. Next, right a class that provides
//the callback and tracks the memory.
//This technique works well with approach from unit3, where operator new/delete
//are implementated for the base class.

//Related references:
//
//TODO: MULTI-THREAD SAFETY.

#include <memory>
#include <iostream>
#include <cassert> //don't leave home without it :)
//used by our memory tracker
#include <map>
#include <functional>

using std::cout;
using std::endl;

namespace unit7 {

    class Base {
    public:

        /**
        * This allocator is used by operators new() and delete() of classes  
        * derived from Base unless a class overwrites the aforementioned
        * operators.
        */
        class Allocator {

        public:

            typedef void*       pointer;
            typedef const void* const_pointer;
            typedef size_t      size_type;

            static void*    malloc( size_t size)
            {
                void* ptr = ::malloc(size);
                
                assert(ptr); //can do better with exceptions, of course.

                if(s_allocationCallback) 
                    s_allocationCallback(size,ptr,s_allocationCallbackData);

                return ptr;
            }

            static void     free( void* ptr,size_t size)
            {
                if(s_allocationCallback) 
                    //pass size of 0 to indicate memory deallocation
                    s_allocationCallback(0,ptr,s_allocationCallbackData);
                
                if(ptr)
                    ::free(ptr);
            }

            /**
            * Defines a signature for the allocation callback function.
            * The allocation callback function is called when the allocator
            * class performs memory allocation, reallocation, or deallocation.
            * To optimize efficiency, you can make this mechanism only 
            * available in the Debug builds.
            * The arguments that are passed to the allocator callback function are:
            * @param sz Size (in bytes) of the memory block. This value is 0 if the
            * block is being deallocated.
            * @param ptr A pointer to the memory block that is beeing allocated or 
            * deallocated
            * @param data A pointer to the user data that was passed to the
            * setAllocCB method when setting the callback.
            * Note: we could also use Functors (a la loki::Functor).
            */
            typedef void    (*allocationCallback)(size_t sz, void* ptr, void* data);

            static void setAllocationCallback(allocationCallback cb, void* data)
            {
                s_allocationCallback = cb;
                s_allocationCallbackData = data;
            }

        private:

            //leave private and unimplemented
            Allocator();
            ~Allocator();
            Allocator(const Allocator& );

            static allocationCallback  s_allocationCallback;
            static void* s_allocationCallbackData;
        };

        //uses our Allocator
        void *operator new(size_t size) 
        {
            return(Allocator::malloc(size));
        }

        //uses our allocator
        void operator delete(void *ptr, size_t size) 
        {
            if(ptr) Allocator::free(ptr,size);
        }

        Base() : m_data(0)
        {
            cout << "Base::Base" << endl;
        }

        //this is a base class, therefore destructor is virtual
        virtual ~Base()
        {
            cout << "Base::~Base" << endl;
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

    //initialize the static data member.
    Base::Allocator::allocationCallback Base::Allocator::s_allocationCallback = NULL;
    void* Base::Allocator::s_allocationCallbackData = NULL;

    //The actual memory tracker is a user-defined class.
    //This implementation simply maintains an STL map of pointers/sizes.
    class memoryTracker{
    public:

        // Globals Constants
        enum {
            KB = 1024,
            MB = 1048576
        };

        //a data structure that keeps track of a memory block
        //allocated by the allocator
        struct MemBlock {

            size_t  m_sz;
            void*   m_ptr;

            MemBlock(size_t sz, void* p) :
            m_sz(sz), m_ptr(p)
            {}

            ~MemBlock(){}

            /*
            * define copy constructor and operator=
            * to comply with the STL requirements
            */
            MemBlock(const MemBlock& src)
            {
                m_sz = src.m_sz;
                m_ptr = src.m_ptr;
            }

            MemBlock& operator=(const MemBlock& src)
            {
                if(&src == this) return *this;
                m_sz = src.m_sz;
                m_ptr = src.m_ptr;
                return *this;
            }

            inline friend bool operator==(const MemBlock& b1, const MemBlock& b2)
            {
                return b1.m_ptr == b2.m_ptr;
            }

            inline friend bool operator<(const MemBlock& b1, const MemBlock& b2)
            {
                return b1.m_ptr < b2.m_ptr;
            }

            bool    operator==(void* ptr) const { return m_ptr == ptr; }
        };

        typedef std::map<void*,MemBlock> MemBlocks;

        memoryTracker()
        {
            //install allocator callback
            Base::Allocator::setAllocationCallback(memoryTracker::callback,this);
        }

        ~memoryTracker()
        {
            //uninstall the allocation callback
            Base::Allocator::setAllocationCallback(NULL,NULL);

            //the STL swap trick to trim memory
            MemBlocks(m_blocks).swap(m_blocks);

            //prints the report to the console
            cout << endl << "memoryTrackerReport" << endl << endl;
            cout <<"Leaked Memory blocks (bytes)" << endl;
            cout <<"----------------------------" << endl;

            size_t totalLeak = 0;

            MemBlocks::const_iterator it, ite = m_blocks.end();

            for(it = m_blocks.begin(); it != ite; ++it) {

                const MemBlock& block = (*it).second;
                cout << "size: " << (int) block.m_sz << endl;
                totalLeak += block.m_sz;
            }

            cout << "************************************************" << endl;
            cout << "* Total leak is: " << (int)totalLeak << " bytes in "
                 << (int) m_blocks.size()<< " blocks." << endl;
            cout << "************************************************" << endl;

            m_blocks.clear();

        }

    private:

        //The actual callback function that tracks the memory
        static void callback(size_t sz, void* ptr, void* data)
        {
            memoryTracker* t = reinterpret_cast<memoryTracker*>(data);

            if(sz > 0) {
                //the memory is allocated

                //the same ptr should NOT be in the map already
                assert(t->m_blocks.find(ptr) == t->m_blocks.end());
                t->m_blocks.insert(MemBlocks::value_type(ptr,MemBlock(sz,ptr)));

            } else {

                //the memory is deallocated
                size_t num = t->m_blocks.erase(ptr);

                if(num != 1) {

                    cout << "memoryTracker: freeing memory that wasn't previously registered" << endl;
                }
            }
        }

        MemBlocks m_blocks;
    };

} //namespace unit7

//the tracker works within its scope. Here, it is the global scope of the
//compilation unit.
unit7::memoryTracker s_tracker;

void memoryTrackingTest()
{
    using namespace unit7;

    cout << endl << "Memory tracking test" << endl;

    Base* bptr = new Derived();

    //say, we forgot to destroy the object...
    //the leak should show up in the memory tracker report

}

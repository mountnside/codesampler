//-----------------------------------------------------------------------------
//           Name: stl_priority_queue.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: This sample demonstrates how to use STL's priority_queue 
//                 container to store both a sequence of sorted int values and
//                 a sequence of sorted class instances.
//-----------------------------------------------------------------------------

#include <iostream>
#include <vector>
#include <queue>

using namespace std;


//
// We'll start with the simplest priority_queue example I can think of: We'll
// create a priority queue to sort int values which are stored in a vector.
//

typedef vector< int > myVectorType;
typedef priority_queue< int, myVectorType > myPriorityQType;


//
// Our second example of a priority_queue will be a little more complicated
// as it will demonstrate how to use a priority_queue to sort a user-defined 
// class type.
//

class myClass 
{

public:

    myClass() : m_priority( 0 ) {}
	~myClass() {}

    //
    // The Less structure below is how instances of myClass get sorted by 
    // a priority_queue. It derives from binary_function, which is defined by
    // STL.
    //
    
    struct Less : public binary_function< myClass, myClass, bool >
    {
        bool operator()(const myClass e1, const myClass e2) const
        {
            return e1.m_priority < e2.m_priority;
        }
    };

    // We'll use this member to keep track of the object's priority.
    int m_priority;
};

typedef vector< myClass > myVectorType2;
typedef priority_queue< myClass, myVectorType2, myClass::Less > myPriorityQType2;


void main( void )
{
    //
    // Create the simple priority_queue and load it with some int values to 
    // work with.
    //

    myPriorityQType myPriorityQ_1;

    myPriorityQ_1.push( 2 );
    myPriorityQ_1.push( 3 );
    myPriorityQ_1.push( 7 );
    myPriorityQ_1.push( 5 );
    myPriorityQ_1.push( 6 );
    myPriorityQ_1.push( 1 );
    myPriorityQ_1.push( 8 );
    myPriorityQ_1.push( 0 );
    myPriorityQ_1.push( 9 );
    myPriorityQ_1.push( 4 );

    //
    // Output the size
    //
    
    int size = myPriorityQ_1.size();

    cout << "myPriorityQ_1 size: " << size << endl;
    
    //
    // Now, let's check and see how they've been sorted by value or priority.
    //
    // To do this we'll output the elements in myPriorityQ_1 using top()
    // followed by pop() to get to next element until myPriorityQ_1 is empty.
    //

    cout << "myPriorityQ_1 contents: ";

    while( !myPriorityQ_1.empty() )
    {
        cout << myPriorityQ_1.top() << " ";

        myPriorityQ_1.pop();
    }

    cout << endl;
    cout << endl;

    //
    // Create the second, more complicated priority_queue and load it with 
    // some class objects to work with.
    //

    myPriorityQType2 myPriorityQ_2;

    myClass temp;

    temp.m_priority = 55;
    myPriorityQ_2.push( temp );

    temp.m_priority = 52;
    myPriorityQ_2.push( temp );

    temp.m_priority = 51;
    myPriorityQ_2.push( temp );

    temp.m_priority = 59;
    myPriorityQ_2.push( temp );

    temp.m_priority = 54;
    myPriorityQ_2.push( temp );

    temp.m_priority = 56;
    myPriorityQ_2.push( temp );

    temp.m_priority = 50;
    myPriorityQ_2.push( temp );

    temp.m_priority = 58;
    myPriorityQ_2.push( temp );

    temp.m_priority = 57;
    myPriorityQ_2.push( temp );

    temp.m_priority = 53;
    myPriorityQ_2.push( temp );

    //
    // Output the size
    //
    
    size = myPriorityQ_2.size();

    cout << "myPriorityQ_2 size: " << size << endl;

    //
    // Now, let's check and see how they've been sorted by value or priority.
    //
    // To do this we'll output the elements in myPriorityQ_2 using top()
    // followed by pop() to get to next element until myPriorityQ_2 is empty.
    //

    cout << "myPriorityQ_2 contents: ";

    while( !myPriorityQ_2.empty() )
    {
        myPriorityQType2::value_type temp = myPriorityQ_2.top();

        cout << temp.m_priority << " ";

        myPriorityQ_2.pop();
    }

    cout << endl;
    cout << endl;
}


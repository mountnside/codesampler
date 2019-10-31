//-----------------------------------------------------------------------------
//           Name: cpp_linked_list.cpp
//         Author: Kevin Harris
//  Last Modified: 04/23/05
//    Description: Win32 console test program for debugging my singly linked, 
//                 linked-list class
//-----------------------------------------------------------------------------

#include <iostream>
using namespace std;

#include "linkedList.h"

void main()
{
	LinkedList list;

	cout << "-- Initial State of Linked List --" << endl;
    cout << endl;
	list.dumpList();
    cout << endl;

	// Create a Linked List by some adding nodes
	list.addNode(10);
	list.addNode(20);
	list.addNode(30);
	list.addNode(40);
	list.addNode(50);
	list.addNode(60);

    cout << "-- After Adding Some Nodes --" << endl;
    cout << endl;
	list.dumpList();
    cout << endl;

	// Test all the node navigation methods 

    cout << "-- Test Node Navigation -- " << endl;
    cout << endl;

    cout << "Testing gotoNode() method... " << endl;

    list.gotoNode(5);
    cout << "After gotoNode(5): data = " << list.getNode()->m_nData << endl;

    list.gotoNode(4);
    cout << "After gotoNode(4): data = " << list.getNode()->m_nData << endl;

    list.gotoNode(3);
    cout << "After gotoNode(3): data = " << list.getNode()->m_nData << endl;

    list.gotoNode(2);
    cout << "After gotoNode(2): data = " << list.getNode()->m_nData << endl;

    list.gotoNode(1);
    cout << "After gotoNode(1): data = " << list.getNode()->m_nData << endl;

    list.gotoNode(0);
    cout << "After gotoNode(0): data = " << list.getNode()->m_nData << endl;

    cout << endl;
    cout << "Testing nextNode() & prevNode methods..." << endl;

    list.nextNode();
    cout << "After nextNode(): data = " << list.getNode()->m_nData << endl;

    list.nextNode();
    cout << "After nextNode(): data = " << list.getNode()->m_nData << endl;

    list.prevNode();
    cout << "After prevNode(): data = " << list.getNode()->m_nData << endl;

    list.prevNode();
    cout << "After prevNode(): data = " << list.getNode()->m_nData << endl;
    cout << endl;

	// Test the removal of a node
	cout << "-- Test Node Removal -- " << endl;
    cout << endl;
    
    list.gotoNode(0);
    cout << "Calling gotoNode(0) followed by removeNode()..." << endl;
    list.removeNode();
    list.dumpList();
    cout << endl;

    list.gotoNode(5);
    cout << "Calling gotoNode(5) followed by removeNode()..." << endl;
    list.removeNode();
    list.dumpList();
    cout << endl;

	// Crash test boundary checking and the passing of bogus data...
	cout << "-- Stress Test Boundary and Data Checking --" << endl;
    cout << endl;

	int i = 0;

	for( i = 0; i < 10; ++i )
		list.nextNode();

	for( i = 0; i < 10; ++i )
		list.prevNode();

    for( i = -10; i < 10; ++i )
		list.gotoNode(i);

	cout << "Test Successful..." << endl;
    cout << endl;

    // Make sure we can't crash the list by manually emptying it.
	cout << "-- Crash Test Node Removal -- " << endl;
    cout << endl;
    
    list.gotoNode(0);
    list.dumpList();

    list.removeNode();
    list.dumpList();

    list.removeNode();
    list.dumpList();

    list.removeNode();
    list.dumpList();

    list.removeNode();
    list.dumpList();

    cout << endl;
}

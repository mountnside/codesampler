//-----------------------------------------------------------------------------
//           Name: linkedList.cpp
//         Author: Kevin Harris
//  Last Modified: 04/23/05
//    Description: Class declaration for a singly linked, linked-list class
//-----------------------------------------------------------------------------

#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

#include <iostream>
using namespace std;

struct Node
{
	Node() :
	m_pNextNode( NULL ),
	m_nData( 0 )
	{}

	int   m_nData;
	Node *m_pNextNode;

};

class LinkedList
{
	public:

	LinkedList();
   ~LinkedList();

    // Add & Removing nodes
    void addNode( int nValue );
    void removeNode();

    // Node Navigation
	void nextNode();
	void prevNode();
    void gotoNode( int nIndex );

    // Node access
    Node *getNode();

    // Utilities
	void dumpList();

    private:

    // Internal helper methods
	Node *getPrevNodeByPtr( Node *pNode );
    void  removeNodeByPtr( Node *pNode );

	Node *m_headNode;
	Node *m_tailNode;
	Node *m_currentNode;
};

#endif // _LINKEDLIST_H_

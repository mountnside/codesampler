//-----------------------------------------------------------------------------
//           Name: linkedList.cpp
//         Author: Kevin Harris
//  Last Modified: 04/23/05
//    Description: Implementation of a singly linked, linked-list class.
//-----------------------------------------------------------------------------

#include "linkedList.h"

LinkedList::LinkedList()
{
    m_headNode    = NULL;
	m_tailNode    = NULL;
	m_currentNode = NULL;
}


LinkedList::~LinkedList()
{
    Node *tempNode = NULL;
    
	while( m_headNode != NULL )
	{
		tempNode = m_headNode->m_pNextNode;
		delete m_headNode;
        m_headNode = tempNode;
	}

    m_currentNode = NULL;
}


void LinkedList::addNode( int nValue )
{
    // If the list is completely NULL, 
    // start off with a new head node
    if( m_headNode == NULL )
    {
        m_headNode = new Node();
        
        m_headNode->m_nData     = nValue;
        m_headNode->m_pNextNode = NULL;

	    m_tailNode    = m_headNode;
	    m_currentNode = m_headNode;
    }
    else
    {
        // The list has laready been initiated, so 
        // create a new node at the end of the list.
	    m_tailNode->m_pNextNode = new Node();

        m_tailNode->m_pNextNode->m_nData     = nValue;
        m_tailNode->m_pNextNode->m_pNextNode = NULL;

	    // Make the new node the tail node.
	    m_tailNode = m_tailNode->m_pNextNode; 
    }
}


void LinkedList::removeNode()
{
    // This function simply removes the current node.
    removeNodeByPtr( m_currentNode );
}


Node *LinkedList::getNode()
{
    return m_currentNode;
}


void LinkedList::nextNode()
{
	// Move forward to the next node.
	if( m_currentNode->m_pNextNode != NULL )
		m_currentNode = m_currentNode->m_pNextNode;
}


void LinkedList::prevNode()
{
	// Go back to the previous node in the list.
	if( m_currentNode != m_headNode )
		m_currentNode = getPrevNodeByPtr( m_currentNode );
}


void LinkedList::gotoNode( int nIndex )
{
	Node *tempNode = m_headNode;

	for( int i = 0; i < nIndex; ++i )
	{
		if( tempNode != NULL )
			tempNode = tempNode->m_pNextNode;
	}

    if( tempNode != NULL )
        m_currentNode = tempNode;
    else
	    m_currentNode = m_tailNode;
}


void LinkedList::dumpList()
{
	// Dump the list's current contents to the console.
	Node *tempNode = m_headNode;

	while( tempNode != NULL )
	{
		cout << tempNode->m_nData << " -> ";
		tempNode = tempNode->m_pNextNode;
	}

	cout << "NULL" << endl;
}


Node *LinkedList::getPrevNodeByPtr( Node *pNode )
{
	// Starting at the head node, work our way down the
	// Linked List until we find the node that comes
	// before the one we passed into this function.

	Node *tempNode = m_headNode;

	if( pNode == m_headNode ) // Special case where the index is the m_headNode.
		return m_headNode;

	while( tempNode->m_pNextNode != pNode )
		tempNode = tempNode->m_pNextNode;

	return tempNode;
}


void LinkedList::removeNodeByPtr( Node *pNode )
{ 
	// Remove a node from the list.
    if( pNode != NULL )
    {
	    Node *tempNode;

	    if( pNode == m_headNode ) // If the node is the Head node
	    {
		    tempNode = m_headNode;
		    m_headNode = m_headNode->m_pNextNode;
		    delete tempNode;
	    }
	    else if( pNode == m_tailNode ) //If the node is the Tail node
	    { 
		    tempNode = getPrevNodeByPtr( pNode );
		    tempNode->m_pNextNode = NULL;
		    delete pNode;
		    m_tailNode = tempNode;
	    }
	    else // The node is somewhere in the middle...
	    {
		    tempNode = getPrevNodeByPtr( pNode );
		    tempNode->m_pNextNode = pNode->m_pNextNode;
		    delete pNode;
	    }

        m_currentNode = m_headNode; // Reset the m_currentNode to the head.
    }
}
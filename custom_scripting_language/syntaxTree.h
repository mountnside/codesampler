//-----------------------------------------------------------------------------
//           Name: syntaxTree.h
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Syntax tree
//-----------------------------------------------------------------------------

#ifndef _SYNTAXTREE_H_
#define _SYNTAXTREE_H_

#include <iostream>
using namespace std;

//-----------------------------------------------------------------------------
// SYMBOLIC CONSTANTS
//-----------------------------------------------------------------------------

// The syntax tree node types
enum NodeType
{
	// Statements
	STMT_LIST,        // list of statements [left-part, right-part] (left-part may be another list)
	EMPTY_STMT,       // empty statement []
	EXPR_STMT,        // single expression as statement [expression]
	PRINT_STMT,       // print statement [expression]
	INPUT_STMT,       // input statement [variable]
	IFTHEN_STMT,      // if statement [cond, if-part]
	IFTHENELSE_STMT,  // if statement with else [cond, if-part, else-part]
	ERROR_STMT,       // error statement

	// Expressions
	EQUAL_EXPR,       // equality expression [op1, op2]
	ASSIGN_EXPR,      // assignment expression [variable, op2]
	ADD_EXPR,         // add expression [op1, op2]
	IDENT_EXPR,       // identifier (link to symbol table)
	STR_EXPR,         // string constant (link to symbol table)
    INT_EXPR,         // integer (link to symbol table)
	COERCE_TO_STR,    // coercion to string (from integer)
	COERCE_TO_INT     // coercion to integer (from string)
};

enum DataType  
{
	VOID_TYPE,
	STRING_TYPE,
	INTEGER_TYPE,
	BOOL_TYPE,
	UNKNOWN_TYPE
};

//-----------------------------------------------------------------------------
// FORWARD DECLARATION
//-----------------------------------------------------------------------------
class Symbol;

//-----------------------------------------------------------------------------
// The TreeNode node class for building the syntax tree.
//-----------------------------------------------------------------------------
class TreeNode  
{
public:

	TreeNode( NodeType nodeType ) :
	m_nodeType( nodeType )
	{
		m_child[0] = NULL;
		m_child[1] = NULL;
		m_child[2] = NULL;
		checkSemantics();
	}

	TreeNode( NodeType nodeType, TreeNode *child1 ) :
	m_nodeType( nodeType )
	{
		m_child[0] = child1;
		m_child[1] = NULL;
		m_child[2] = NULL;
		checkSemantics();
	}

	TreeNode( NodeType nodeType, TreeNode *child1, TreeNode *child2 ) :
	m_nodeType( nodeType )
	{
		m_child[0] = child1;
		m_child[1] = child2;
		m_child[2] = NULL;
		checkSemantics();
	}

	TreeNode( NodeType nodeType, TreeNode *child1, TreeNode *child2, TreeNode *child3 ) :
	m_nodeType( nodeType )
	{
		m_child[0] = child1;
		m_child[1] = child2;
		m_child[2] = child3;
		checkSemantics();
	}

	void show( void )
	{
		cout << "-- Begin Syntax Tree Dump ----------------------------------\n\n";
		show(0); // Show the tree's content
		cout << "\n-- End Syntax Tree Dump ------------------------------------\n\n";
	}

	void checkSemantics( void );             // Check the node for semantic errors
	bool coerceToString( int nChildIndex );  // Coerce a child to string type
	bool coerceToInteger( int nChildIndex ); // Coerce a child to integer type

	NodeType  m_nodeType;   // What type of node is it?
	DataType  m_returnType; // The "return" type of the node
	Symbol   *m_symbol;     // Pointer to the symbol, if applicable
	TreeNode *m_child[3];   // Pointers to the these node's children

private:

	void show( int level );
};

typedef TreeNode *SyntaxTree;

#endif
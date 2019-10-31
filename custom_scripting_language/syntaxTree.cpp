//-----------------------------------------------------------------------------
//           Name: syntaxTree.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Syntax tree
//-----------------------------------------------------------------------------

#include "syntaxTree.h"
#include "symbolTable.h"

//-----------------------------------------------------------------------------
// FORWARD DECLARATION
//-----------------------------------------------------------------------------
void error( char *cFormat, ... );

//-----------------------------------------------------------------------------
// Node names
//----------------------------------------------------------------------------- 
char *name[] = 
{
	"statement_list",
	"empty_statement",
	"expression_statement",
	"print_statement",
	"input_statement",
	"if_then_statement",
	"if_then_else_statement",
	"error_statement",
	"equals",
	"assign",
	"add",
	"identifier",
	"string_constant",
	"integer_value",
	"coercion_to_string",
	"coercion_to_integer"
};

//-----------------------------------------------------------------------------
// Number of children per node type
//-----------------------------------------------------------------------------
int children[] = 
{
    2, // "statement_list"
    0, // "empty_statement"
    1, // "expression_statement"
    1, // "print_statement"
    0, // "input_statement"
    2, // "if_then_statement"
    3, // "if_then_else_statement"
    0, // "error_statement"
    2, // "equals"
    1, // "assign"
    2, // "add"
    0, // "identifier"
    0, // "string_constant"
    0, // "integer_value"
    1, // "coercion_to_string"
    1  // "coercion_to_integer"
};

//-----------------------------------------------------------------------------
// Name: show()
// Desc: Recursively show the contents of the syntax tree.
//-----------------------------------------------------------------------------
void TreeNode::show( int level )   
{
	int i;
	int nl;

	if( !this )
        return;

	if( m_nodeType != STMT_LIST )
	{
		for( i = 0; i < level; i++ )   
			cout << "   ";

		cout << name[m_nodeType];

		switch( m_nodeType )  
		{
			case INPUT_STMT: 
			case ASSIGN_EXPR: 
			case IDENT_EXPR:
				cout << " (" << m_symbol->m_cName.c_str() << ")";
				break;
			case STR_EXPR:
				cout << " (\"" << m_symbol->m_cString.c_str() << "\")";
				break;
			case INT_EXPR:
				cout << " (" << m_symbol->m_nInteger << ")";
				break;
		}

		nl = level + 1;
		cout << endl;
	} 
	else 
	{ 
		nl = level; 
	}

	for( i = 0; i < children[m_nodeType]; ++i )
		m_child[i]->show(nl);
}

//
// ---------------------- Semantic Checking ----------------------------------
//

//-----------------------------------------------------------------------------
// Name: coerceToString()
// Desc: Coerce one of the children to string type
//-----------------------------------------------------------------------------
bool TreeNode::coerceToString( int nChildIndex )
{
	if( m_child[nChildIndex]->m_returnType == STRING_TYPE )
        return true;

	if( m_child[nChildIndex]->m_returnType == INTEGER_TYPE )
	{
		m_child[nChildIndex] = new TreeNode( COERCE_TO_STR, m_child[nChildIndex] );
		return true;
	}

	if( m_child[nChildIndex]->m_returnType != BOOL_TYPE )
	return false;

	m_child[nChildIndex] = new TreeNode( COERCE_TO_STR, m_child[nChildIndex] );
	return true;

}

//-----------------------------------------------------------------------------
// Name: coerceToInteger()
// Desc: Coerce one of the children to integer type
//-----------------------------------------------------------------------------
bool TreeNode::coerceToInteger( int nChildIndex )
{
	if( m_child[nChildIndex]->m_returnType == STRING_TYPE )
	{
		m_child[nChildIndex] = new TreeNode( COERCE_TO_INT, m_child[nChildIndex] );
		return true;
	}

	if( m_child[nChildIndex]->m_returnType == INTEGER_TYPE )
		return true;

    if( m_child[nChildIndex]->m_returnType != BOOL_TYPE )
	return false;

	m_child[nChildIndex] = new TreeNode( COERCE_TO_INT, m_child[nChildIndex] );
	return true;

}

//-----------------------------------------------------------------------------
// Name: checkSemantics()
// Desc: Check the semantics of this node
//-----------------------------------------------------------------------------
void TreeNode::checkSemantics( void )
{
	// First, set the type of value the node 'returns'
	switch( m_nodeType )
	{
	    case STMT_LIST:
	    case EMPTY_STMT:
	    case EXPR_STMT:
	    case PRINT_STMT:
	    case INPUT_STMT:
	    case IFTHEN_STMT:
	    case IFTHENELSE_STMT:
	    case ERROR_STMT:
		    m_returnType = VOID_TYPE;  // Statements have no value
		    break;

	    case EQUAL_EXPR:
		    m_returnType = BOOL_TYPE;
		    break;

	    case ADD_EXPR:
			if( m_child[0]->m_returnType == STRING_TYPE )
				m_returnType = STRING_TYPE;
			else if( m_child[0]->m_returnType == INTEGER_TYPE )
				m_returnType = INTEGER_TYPE;
			break;

	    case ASSIGN_EXPR:
			if( m_child[0]->m_returnType == STRING_TYPE )
				m_returnType = STRING_TYPE;
			else if( m_child[0]->m_returnType == INTEGER_TYPE )
				m_returnType = INTEGER_TYPE;
		    break;

		case IDENT_EXPR:
			//m_returnType = UNKNOWN_TYPE; //??????????????????? if() doesn't work
            m_returnType = STRING_TYPE;
		    break;
			
	    case STR_EXPR:
		    m_returnType = STRING_TYPE;
		    break;

		case INT_EXPR:
			m_returnType = INTEGER_TYPE;
			break;

	    case COERCE_TO_STR:
		    m_returnType = STRING_TYPE;

		case COERCE_TO_INT:
		    m_returnType = INTEGER_TYPE;
	}

    //
	// Now, check the semantics...
    //

	switch( m_nodeType )
	{
		case IFTHEN_STMT:
		case IFTHENELSE_STMT:
			if( m_child[0]->m_returnType != BOOL_TYPE )
			error( "if: Condition should be boolean" );
			break;
		case EQUAL_EXPR:
			// No coercions here, types have to be equal
			if( m_child[0]->m_returnType != m_child[1]->m_returnType )
				error( "==: Different types" );
			break;

		case ADD_EXPR:
			if( m_child[0]->m_returnType == STRING_TYPE )
			{
				if( !coerceToString(1) )
					error( "+: Couldn't coerce second argument to string" );
			}

			if( m_child[0]->m_returnType == INTEGER_TYPE )
			{
				if( !coerceToInteger(1) )
					error( "+: Couldn't coerce second argument to integer" );
			}

			break;

		case ASSIGN_EXPR:
			break;
	}
}

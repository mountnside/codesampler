//-----------------------------------------------------------------------------
//           Name: symbolTable.h
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Symbol table
//-----------------------------------------------------------------------------

#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_

#include <string>
using namespace std;

//-----------------------------------------------------------------------------
// SYMBOLIC CONSTANTS
//-----------------------------------------------------------------------------

enum SymbolType
{
	IDENTIFIER, // Identifier or variable name
	STR_CONST,  // String constant
	INT_VALUE   // Integer value
};

//-----------------------------------------------------------------------------
// The Symbol class
//-----------------------------------------------------------------------------
class Symbol
{
public:

	Symbol( const string& cName, SymbolType type, const string& cString, int nLine ) :
	m_cName( cName ),
	m_type( type ),
	m_cString( cString ),
	m_nLine( nLine ),
	m_next( NULL ),
	m_nInteger( 0 )
	{}

	Symbol( const string& cName, SymbolType type, int nValue, int nLine ) :
	m_cName( cName ),
	m_type( type ),
	m_cString( "" ),
	m_nLine( nLine ),
	m_next( NULL ),
	m_nInteger( nValue )
	{}

	~Symbol( void )
	{
		if( this == NULL )
			return;

		if( m_next != NULL )
		{
			delete m_next;
			m_next = NULL;
		}
	}

	void show( void )
	{
		if( m_type == STR_CONST )
			printf( "| %-20s | %4d | = \"%s\" \n", m_cName.c_str(), m_nLine, m_cString.c_str() );
		else if( m_type == INT_VALUE )
			printf( "| %-20s | %4d | = %d \n", m_cName.c_str(), m_nLine, m_nInteger );
		else if( m_type == IDENTIFIER )
			printf( "| %-20s | %4d | <identifier> \n", m_cName.c_str(), m_nLine );
	}

	// Index number for VMachine::Read()  (aarggh.. dirty coding!!)
	void setNo( int no ) { m_no = no;}
	int  getNo( void ) { return m_no; }
	int m_no;

	string      m_cName;    // Name of the symbol
	string      m_cString;  // Literal string constant(if string)
    int         m_nInteger; // Integer value (if integer)
	int         m_nLine;    // Line it was first encountered
	SymbolType  m_type;     // Type of the symbol
	Symbol     *m_next;     // Next symbol in list
};

//-----------------------------------------------------------------------------
// The SymbolTable class
//-----------------------------------------------------------------------------
class SymbolTable
{
public:

	SymbolTable( void ) :
	m_start( NULL ),
	m_current( NULL )
	{}

	~SymbolTable( void )
	{
		if( m_start != NULL )
		{
			delete m_start;
			m_start = NULL;
		}
	}

	bool add( Symbol *symbol );
	Symbol *find( const string& cName );
	void show( void );

	Symbol *getFirst( void )
	{
		m_current = m_start;
		return m_current;
	}

	Symbol *getNext( void )
	{
		if( m_current != NULL )
			m_current = m_current->m_next;

		return m_current;
	}

private:

	Symbol *m_start;
	Symbol *m_current;
};

#endif
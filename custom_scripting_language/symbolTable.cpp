//-----------------------------------------------------------------------------
//           Name: symbolTable.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Symbol table
//-----------------------------------------------------------------------------

#include <iostream>
#include "symbolTable.h"

//-----------------------------------------------------------------------------
// Name: add()
// Desc: 
//-----------------------------------------------------------------------------
bool SymbolTable::add( Symbol *symbol )  
{
	if( find( symbol->m_cName ) != NULL )
		return false;

	if( m_start == NULL )
	{
		m_start = symbol;
	}
	else
	{
		// Start at the head...
		Symbol *search = m_start;

		// Find the end...
		while( search->m_next != NULL )
			search = search->m_next;

		// Add the new symbol to the end.
		search->m_next = symbol;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Name: find()
// Desc: 
//-----------------------------------------------------------------------------
Symbol *SymbolTable::find( const string& cName ) 
{
	// This really should be optimized with a hash-table...

	Symbol *search = m_start;

	while( search != NULL && search->m_cName != cName )
	  search = search->m_next;

	return search;
}

//-----------------------------------------------------------------------------
// Name: show()
// Desc: 
//-----------------------------------------------------------------------------
void SymbolTable::show( void )
{
	Symbol *search = m_start;

	cout << "-- Begin Symbol Table Dump ---------------------------------\n\n";
	cout << "+----------------------+------+" << endl;
	cout << "| Name                 | Line |" << endl;
	cout << "+----------------------+------+" << endl;

	while( search != NULL )
	{
		search->show();
		search = search->m_next;
	}

	cout << "+----------------------+------+" << endl;
	cout << "\n-- End Symbol Table Dump -----------------------------------\n\n";
}
//-----------------------------------------------------------------------------
//           Name: vm.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Virtual machine for executing the my_c scripting language
//-----------------------------------------------------------------------------

#include "vm.h"

//-----------------------------------------------------------------------------
// Name: reset()
// Desc: Reset the virtual machine
//-----------------------------------------------------------------------------
void VMachine::reset( void )
{
	m_instr.empty();
	m_data.empty();
	m_stack.empty();
}

//-----------------------------------------------------------------------------
// Name: compile()
// Desc: Compiles the intermediate code into the final virtual assembly code
//       (connected to the compiler for extreme laziness purposes)
//-----------------------------------------------------------------------------
void VMachine::compile( void )
{
	// Dump out the symbol table and load its contents into the 
	// virtual machine's data table.
	Symbol *symbol = g_symbolTable.getFirst();

	while( symbol != NULL )
	{
		Data data;
		data.cString  = "_error_";
		data.nInteger = -1;
		data.bActive  = true;
		data.type     = UNKNOWN_TYPE;
		
		if( symbol->m_type == IDENTIFIER )
		{
			data.type = UNKNOWN_TYPE;
		}
		else if( symbol->m_type == STR_CONST )
		{
			data.cString = symbol->m_cString;
			data.type    = STRING_TYPE;
		}
		else if( symbol->m_type == INT_VALUE )
		{
			data.nInteger = symbol->m_nInteger;
			data.type     = INTEGER_TYPE;
		}

		m_data.push_back( data );

		// Set number so we can find its index back later 
		// (aarggh.. dirty coding!!)

		int nSize = m_data.size();

		symbol->setNo( nSize - 1 );

		symbol = g_symbolTable.getNext();
	}

	m_nNumInstr = g_intCode->length();
	IntInstr *cinstr = g_intCode;

	int i = 0;

	for( i = 0; i < m_nNumInstr; i++ )   
	{
		switch( cinstr->m_opcode )
		{
			case OP_NOP:
				// No operation
				m_instr.push_back( Instr( OP_NOP, 0 ) );
				break;

			case OP_PUSH:
				// Push string [var]
				m_instr.push_back( Instr( OP_PUSH, cinstr->m_operand->getNo() ) );
				break;

			case OP_GETTOP:
				// Get string from top of stack( =assign) [var]
				m_instr.push_back( Instr( OP_GETTOP, cinstr->m_operand->getNo() ) );
				break;

			case OP_DISCARD:
				// Discard top value from the stack
				m_instr.push_back( Instr( OP_DISCARD ) );
				break;

			case OP_PRINT:
				// Print a string or integer
				m_instr.push_back( Instr( OP_PRINT ));
				break;

			case OP_INPUT:
				// Input a string [var]
				m_instr.push_back( Instr( OP_INPUT, cinstr->m_operand->getNo() ));
				break;

			case OP_JMP:
				// Unconditional jump [dest]
				m_instr.push_back( Instr( OP_JMP, cinstr->m_target->m_nLineNumber - i ));
				break;

			case OP_JMPF:
				// Jump if false [dest]
				m_instr.push_back( Instr( OP_JMPF, cinstr->m_target->m_nLineNumber - i ));
				break;

			case OP_EQUAL:
				m_instr.push_back( Instr( OP_EQUAL ));
				break;

			case OP_BOOL_EQUAL:
				// Test whether two bools are equal
				m_instr.push_back( Instr( OP_BOOL_EQUAL ));
				break;

			case OP_ADD:
				// Add either two integers or concat two strings
				m_instr.push_back( Instr( OP_ADD ));
				break;

			case OP_BOOL2STR:
				// Convert bool to string
				m_instr.push_back( Instr( OP_BOOL2STR ));
				break;

			case OP_INT2STR:
				// Convert integer to string
				m_instr.push_back( Instr( OP_INT2STR ));
				break;

			case OP_STR2INT:
				// Convert string to integer
				m_instr.push_back( Instr( OP_STR2INT ));
				break;

			case JUMPTARGET:
				// Not an opcode but a jump target
				m_instr.push_back( Instr( OP_NOP ));
				break;
		}

		cinstr = cinstr->m_next;
	}
}

//-----------------------------------------------------------------------------
// Name: execute
// Desc: Execute the program in memory.
//-----------------------------------------------------------------------------
void VMachine::execute( void )
{
	int ip  = 0; // instruction pointer (starting at instruction 0)
	int ipc = 0; // instruction pointer change
	int i   = 0;
	int j   = 0;
	int k   = 0;
	int n   = 0;

	while( ip < m_nNumInstr )
	{
		ipc = 1; // default: add one to ip

		switch( m_instr[ip].m_opCode )
		{
			case OP_NOP:
				// No OPeration
				break;

			case OP_PUSH:
				// Push a new data object's index onto the stack
				m_stack.push( copyData( m_instr[ip].m_nOperand ) );
				break;

			case OP_GETTOP:
				i = m_instr[ip].m_nOperand;
				j = m_stack.top();

				m_data[i].cString  = m_data[j].cString;
				m_data[i].nInteger = m_data[j].nInteger;
				m_data[i].type     = m_data[j].type;
				break;

			case OP_DISCARD:
                clearData( m_stack.top() );
                m_stack.pop();
				break;

			case OP_PRINT:
                i = m_stack.top();
                m_stack.pop();
				
				if( m_data[i].type == STRING_TYPE )
					cout << m_data[i].cString << endl;
				else if( m_data[i].type == INTEGER_TYPE )
					cout << m_data[i].nInteger << endl;

				clearData(i);
				break;

			case OP_INPUT:
				i = m_instr[ip].m_nOperand;

				char cInputBuffer[101];
				cin.getline( cInputBuffer, 100 );

				m_data[i].cString  = cInputBuffer;
				m_data[i].nInteger = -1;
				m_data[i].type     = STRING_TYPE;
			break;

			case OP_JMP:
				ipc = m_instr[ip].m_nOperand;
				break;

			case OP_JMPF:
                i = m_stack.top();
                m_stack.pop();

				if( i == STACK_FALSE )   
					ipc = m_instr[ip].m_nOperand;
				break;

			case OP_EQUAL:
                i = m_stack.top();
                m_stack.pop();
                j = m_stack.top();
                m_stack.pop();

				if( m_data[i].type == STRING_TYPE )
				{
					if( m_data[i].cString == m_data[j].cString )
						k = STACK_TRUE;
					else
						k = STACK_FALSE;
				}
				else if( m_data[i].type == INTEGER_TYPE )
				{
					if( m_data[i].nInteger == m_data[j].nInteger )
						k = STACK_TRUE;
					else
						k = STACK_FALSE;
				}

				m_stack.push( k );
				break;

			case OP_BOOL_EQUAL:
                i = m_stack.top();
                m_stack.pop();
                j = m_stack.top();
                m_stack.pop();

				if( i == j )
					k = STACK_TRUE;
				else
					k = STACK_FALSE;

					m_stack.push( k );
				break;

			case OP_ADD:
                i = m_stack.top();
                m_stack.pop();
                j = m_stack.top();
                m_stack.pop();

				k = copyData( j );

				//
				// Perform run-time type conversion...
				//

				if( m_data[k].type == UNKNOWN_TYPE && m_data[i].type == STRING_TYPE )
				{
					m_data[k].cString += m_data[i].cString;
					m_data[k].type = STRING_TYPE;
				}
				else if( m_data[k].type == UNKNOWN_TYPE && m_data[i].type == INTEGER_TYPE )
				{
					m_data[k].nInteger += m_data[i].nInteger;
					m_data[k].type = INTEGER_TYPE;
				}
				else if( m_data[k].type == STRING_TYPE && m_data[i].type == UNKNOWN_TYPE )
				{
					m_data[k].cString += m_data[i].cString;
				}
				else if( m_data[k].type == INTEGER_TYPE && m_data[i].type == UNKNOWN_TYPE )
				{
					m_data[k].nInteger += m_data[i].nInteger;
				}
				else if( m_data[k].type == STRING_TYPE && m_data[i].type == STRING_TYPE )
				{
					m_data[k].cString += m_data[i].cString;
				}
				else if( m_data[k].type == INTEGER_TYPE && m_data[i].type == INTEGER_TYPE )
				{
					m_data[k].nInteger += m_data[i].nInteger;
				}
				else if( m_data[k].type == INTEGER_TYPE && m_data[i].type == STRING_TYPE )
				{
					n = atoi( m_data[i].cString.c_str() );
					m_data[k].nInteger += n;
				}
				else if( m_data[k].type == STRING_TYPE && m_data[i].type == INTEGER_TYPE )
				{
					char cBuffer[10];
					for( n = 0; n < 10; ++n )
						cBuffer[n] = 0;
					_itoa( m_data[i].nInteger, cBuffer, 10 );
					m_data[k].cString += cBuffer;
				}

				clearData( i );
				clearData( j );
				m_stack.push( k );

				break;

			case OP_BOOL2STR:
                i = m_stack.top();
                m_stack.pop();

				if( i == STACK_FALSE )
					i = newData( "false", -1, STRING_TYPE );
				else
					i = newData( "true", -1, STRING_TYPE );

					m_stack.push( i );
				break;

			case OP_INT2STR:
                i = m_stack.top();
                m_stack.pop();

				char cBuffer[10];

				for( n = 0; n < 10; ++n )
					cBuffer[n] = 0;

				_itoa( m_data[i].nInteger, cBuffer, 10 );
				i = newData( cBuffer, m_data[i].nInteger, STRING_TYPE );

				m_stack.push( i );
				break;

			case OP_STR2INT:
                i = m_stack.top();
                m_stack.pop();

				n = atoi( m_data[i].cString.c_str() );
				i = newData( m_data[i].cString, n, INTEGER_TYPE );

				m_stack.push( i );
				break;

		}

		ip += ipc;
	}
}

//-----------------------------------------------------------------------------
// Name: findNewData()
// Desc: Returns the index to a new data object
//-----------------------------------------------------------------------------
int VMachine::findNewData( void )
{
	int nSize = m_data.size();
	int i;

	for( i = 0; i < nSize; ++i )
	{
		if( m_data[i].bActive == false )
		{
			m_data[i].bActive = true;
			return i;
		}
	}

	// All "Data" objects in the vector are currently active! 
	// We'll have to make room for a new one and set it up.
	Data data;
	data.cString  = "_error_";
	data.nInteger = -1;
	data.bActive  = true;
	data.type     = UNKNOWN_TYPE;

	m_data.push_back( data );

	return m_data.size() - 1;
}

//-----------------------------------------------------------------------------
// Name: copyData()
// Desc: Returns the index to a new data object copied from another by index
//-----------------------------------------------------------------------------
int VMachine::copyData( int nIndex )  
{
	int i = findNewData();

	m_data[i].cString  = m_data[nIndex].cString;
	m_data[i].nInteger = m_data[nIndex].nInteger;
	m_data[i].type     = m_data[nIndex].type;

   return i;
}


//-----------------------------------------------------------------------------
// Name: newData()
// Desc: Returns the index to a new data object.
//-----------------------------------------------------------------------------
int VMachine::newData( string cString, int nInteger, DataType type  )  
{
   int i = findNewData();

   m_data[i].cString  = cString;
   m_data[i].nInteger = nInteger;
   m_data[i].type     = type;

   return i;
}

//-----------------------------------------------------------------------------
// Name: clearData()
// Desc: Clears and recycles a previously used data object.
//-----------------------------------------------------------------------------
void VMachine::clearData( int nIndex )
{
	m_data[nIndex].cString  = "_error_";
	m_data[nIndex].nInteger = -1;
	m_data[nIndex].bActive  = false;
	m_data[nIndex].type     = UNKNOWN_TYPE;
}


/*
cout << "m_data[i].cString  = " << m_data[i].cString << endl;
cout << "m_data[i].nInteger = " << m_data[i].nInteger << endl;
cout << "m_data[i].type     = " << m_data[i].type << endl;
cout << "m_data[i].bActive  = " << m_data[i].bActive << endl;

cout << "m_data[j].cString  = " << m_data[j].cString << endl;
cout << "m_data[j].nInteger = " << m_data[j].nInteger << endl;
cout << "m_data[j].type     = " << m_data[j].type << endl;
cout << "m_data[j].bActive  = " << m_data[j].bActive << endl;
cout << endl;
//*/



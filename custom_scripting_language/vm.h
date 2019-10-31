//-----------------------------------------------------------------------------
//           Name: vm.h
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Virtual machine for executing the my_c scripting language
//-----------------------------------------------------------------------------

#ifndef _VM_H_
#define _VM_H_

#include <iostream>
#include <string>
#include <stack>
#include <vector>
#include "symbolTable.h"
#include "syntaxTree.h"
#include "intcode.h"

using namespace std;

//-----------------------------------------------------------------------------
// EXTERNS
//-----------------------------------------------------------------------------
extern SymbolTable  g_symbolTable;
extern IntInstr    *g_intCode;

//-----------------------------------------------------------------------------
// SYMBOLIC CONSTANTS
//-----------------------------------------------------------------------------

// Instead of pushing the index of an actual Data object onto the stack after 
// an equality check, we can save our selves the time and trouble of creating 
// a new Data object by simply pushing the Boolean result of the equality 
// check onto the stack instead of an index. Of course we'll need to use 
// values that can't be confused with normal index values to be safe about it.

const int STACK_TRUE  = -1; // Stack code for true
const int STACK_FALSE = -2; // Stack code for false

//-----------------------------------------------------------------------------
// The Instr class
//-----------------------------------------------------------------------------
class Instr
{
public:

	Instr( void ) :
	m_opCode( OP_NOP ),
	m_nOperand( 0 )
	{}

	Instr( OpCode opCode ) :
	m_opCode( opCode ),
	m_nOperand( 0 )
	{}

	Instr( OpCode opCode, int nOperand ) :
	m_opCode( opCode ),
	m_nOperand( nOperand )
	{}

	OpCode m_opCode;   // The opcode to use
	int    m_nOperand; // Index of stack data object or target instruction
};

//-----------------------------------------------------------------------------
// The VMachine class
//-----------------------------------------------------------------------------
class VMachine  
{
public:

	VMachine::VMachine( void ) :
	m_instr( NULL ),
	m_nNumInstr( 0 )
	{}

	VMachine::~VMachine( void ) 
    { reset(); }

	void compile( void );
	void execute( void );
	void reset( void );

	struct Data
	{
		string   cString;
		int      nInteger;
		DataType type;
		bool     bActive;
	};

private:

	int findNewData( void );
	int copyData( int nIndex );
	int newData( string cString, int nInteger, DataType type );
	void clearData( int nIndex );

    typedef stack<int> IndexStack;
	typedef vector<Data> DataVector;
	typedef vector<Instr> InstrVector;
  
	InstrVector m_instr;     // The virtual assembly instructions
	DataVector  m_data;      // The data which is manipulated by the instructions
	IndexStack  m_stack;     // The stack that holds the indexes to data objects currently being worked on
	int         m_nNumInstr; // The total number of intsructions
};

#endif
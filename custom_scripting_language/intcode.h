//-----------------------------------------------------------------------------
//           Name: intcode.h
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Intermediate code generation
//-----------------------------------------------------------------------------

#ifndef _INTCODE_H_
#define _INTCODE_H_

//-----------------------------------------------------------------------------
// The opcodes (these will probably also be our final bytecode opcodes)
//-----------------------------------------------------------------------------
enum OpCode
{
	OP_NOP,        // no operation
	OP_PUSH,       // push string [var]
	OP_GETTOP,     // get string from top of stack (=assign) [var]
	OP_DISCARD,    // discard top value from the stack
	OP_PRINT,      // print a string
	OP_INPUT,      // input a string [var]
	OP_JMP,        // unconditional jump [dest]
	OP_JMPF,       // jump if false [dest]
	OP_EQUAL,      // test whether two integers or two strings are equal
	OP_BOOL_EQUAL, // test whether two bools are equal
	OP_ADD,        // add two integers or concatenate two strings together
	OP_BOOL2STR,   // convert bool to string
	OP_INT2STR,    // convert int to string
	OP_STR2INT,    // convert int to string
	JUMPTARGET     // not an m_opcode but a jump target; the target field points to the jump instruction
};

//-----------------------------------------------------------------------------
// Intermediate code instruction class 
//-----------------------------------------------------------------------------
class IntInstr 
{
public:

	IntInstr( void ) :
	m_opcode( OP_NOP ),
	m_next( NULL ),
	m_target( NULL ),
	m_operand( NULL )
	{}

	IntInstr( OpCode opcode ) :
	m_opcode( opcode ),
	m_next( NULL ),
	m_target( NULL ),
	m_operand( NULL )
	{}

	IntInstr( OpCode opcode, IntInstr *target ) :
	m_opcode( opcode ),
	m_next( NULL ),
	m_target( target ),
	m_operand( NULL )
	{}
	
	IntInstr( OpCode opcode, Symbol *operand ) :
	m_opcode( opcode ),
	m_next( NULL ),
	m_target( NULL ),
	m_operand( operand )
	{}

	int length( void )
	{
		IntInstr *i = m_next; 
		int nCount = 1;

		while( i != NULL )
		{
			++nCount;
			i = i->m_next;
		}

		return nCount;
	}

	void show( void )
	{
		cout << "-- Begin Intermediate Code Dump ----------------------------\n\n";
		showBlock(); // Show the tree's content
		cout << "\n-- End Intermediate Code Dump ------------------------------\n\n";
	}

	void showBlock( void );
	void number( int nLineNumber ); // Number the lines of this code block

	int       m_nLineNumber; // Line number
	OpCode    m_opcode;      // The opcode
	Symbol   *m_operand;     // Operand
	IntInstr *m_target;      // Jump target operand
	IntInstr *m_next;        // The next instruction
};

IntInstr *generateIntCode( SyntaxTree tree );

#endif
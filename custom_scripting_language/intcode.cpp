//-----------------------------------------------------------------------------
//           Name: intcode.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: Intermediate code generation
//-----------------------------------------------------------------------------

#include <iostream>
#include "symbolTable.h"
#include "syntaxTree.h"
#include "intcode.h"

//-----------------------------------------------------------------------------
// Names of the opcodes
//-----------------------------------------------------------------------------
char *op_name[] = 
{
	"OP_NOP",
	"OP_PUSH",
	"OP_GETTOP",
	"OP_DISCARD",
	"OP_PRINT",
	"OP_INPUT",
	"OP_JMP",
	"OP_JMPF",
	"OP_EQUAL",
	"OP_BOOL_EQUAL",
	"OP_ADD",
	"OP_BOOL2STR",
	"OP_INT2STR",
	"OP_STR2INT",
	"JUMPTARGET"
};

//-----------------------------------------------------------------------------
// Name: number()
// Desc: Assigns line numbers to this block of intermediate code, starting 
//       with nLineNumber.
//-----------------------------------------------------------------------------
void IntInstr::number(  int nLineNumber  )   
{
	IntInstr *num = this;

	while( num != NULL  )
	{
		num->m_nLineNumber = nLineNumber++; 
		num = num->m_next;
	}
}


//-----------------------------------------------------------------------------
// Name: showBlock()
// Desc: Show this block of intermediate code.
//-----------------------------------------------------------------------------
void IntInstr::showBlock( void )
{
	cout << m_nLineNumber << ": " << op_name[m_opcode] << " ";

	if( m_operand )
	   cout << m_operand->m_cName.c_str() << " ";

	if( m_target )
	   cout << m_target->m_nLineNumber;

	cout << endl;

	if( m_next != NULL )
		m_next->showBlock();
}

//-----------------------------------------------------------------------------
// Name: concatenate()
// Desc: Concatenates two blocks of instructions
//-----------------------------------------------------------------------------
IntInstr *concatenate(  IntInstr *block1, IntInstr *block2  )
{
   IntInstr *search = block1;

   while( search->m_next != NULL )
       search = search->m_next;

   search->m_next = block2;
   return block1;
}

//-----------------------------------------------------------------------------
// Name: prefixJT()
// Desc: Prefix a jump m_target to a block; returns the new block
//-----------------------------------------------------------------------------
IntInstr *prefixJT(  IntInstr *block, IntInstr *ref_instr  )   
{
   IntInstr *jt = new IntInstr;

   jt->m_opcode = JUMPTARGET;
   jt->m_target = ref_instr;   // The referring instruction
   jt->m_next   = block;

   return jt;
}

//-----------------------------------------------------------------------------
// Name: generateIntCode()
// Desc: Recursively generate intermediate code
//-----------------------------------------------------------------------------
IntInstr *generateIntCode(  SyntaxTree tree  )  
{
	TreeNode *root      = tree;
	IntInstr *block1    = NULL;
	IntInstr *block2    = NULL;
	IntInstr *cond      = NULL;
	IntInstr *jump2else = NULL;
	IntInstr *thenpart  = NULL;
	IntInstr *jump2end  = NULL;
	IntInstr *elsepart  = NULL;
	IntInstr *endif     = NULL;

	switch( root->m_nodeType )
	{
		case STMT_LIST:
			block1 = generateIntCode( root->m_child[0] );
			block2 = generateIntCode( root->m_child[1] );
			concatenate( block1, block2 );
			return block1;

		case EMPTY_STMT:
			return new IntInstr( OP_NOP );

		case EXPR_STMT:
			block1 = generateIntCode( root->m_child[0] );
			block2 = new IntInstr( OP_DISCARD );
			return concatenate( block1, block2 );

		case PRINT_STMT:
			block1 = generateIntCode( root->m_child[0] );
			block2 = new IntInstr( OP_PRINT );
			return concatenate( block1, block2 );

		case INPUT_STMT:
			return new IntInstr( OP_INPUT, root->m_symbol );

		case IFTHEN_STMT:
			// First, create the necessary code parts
			cond      = generateIntCode( root->m_child[0] );
			jump2end  = new IntInstr( OP_JMPF );      // set m_target below
			thenpart  = generateIntCode( root->m_child[1] );
			endif     = new IntInstr( JUMPTARGET, jump2end );
			jump2end->m_target = endif;

			// Now, concatenate them all
			concatenate( cond, jump2end );
			concatenate( jump2end, thenpart );
			concatenate( thenpart, endif );
			return cond;

		case IFTHENELSE_STMT:
			// First, create the necessary code parts
			cond      = generateIntCode( root->m_child[0] );
			jump2else = new IntInstr( OP_JMPF );      // set m_target below
			thenpart  = generateIntCode( root->m_child[1] );
			elsepart  = prefixJT( generateIntCode( root->m_child[2] ), jump2else );
			jump2else->m_target = elsepart;
			jump2end  = new IntInstr( OP_JMP );       // set m_target below
			endif     = new IntInstr( JUMPTARGET, jump2end );
			jump2end->m_target = endif;

			// Now, concatenate them all
			concatenate( cond, jump2else );
			concatenate( jump2else, thenpart );
			concatenate( thenpart, jump2end );
			concatenate( jump2end, elsepart );
			concatenate( elsepart, endif );
			return cond;

		case ERROR_STMT:
			return new IntInstr( OP_NOP );

		case EQUAL_EXPR:
			block1 = generateIntCode( root->m_child[0] );
			block2 = generateIntCode( root->m_child[1] );
			concatenate( block1, block2 );

		    if( root->m_child[0]->m_returnType == BOOL_TYPE )
				return concatenate( block1, new IntInstr( OP_BOOL_EQUAL ) );
			else
				return concatenate( block1, new IntInstr( OP_EQUAL ) );
			
		case ASSIGN_EXPR:
			block1 = generateIntCode( root->m_child[0] );
			block2 = new IntInstr( OP_GETTOP, root->m_symbol );
			return concatenate( block1, block2 );

		case ADD_EXPR:
			block1 = generateIntCode( root->m_child[0] );
			block2 = generateIntCode( root->m_child[1] );
			concatenate( block1, block2 );
			return concatenate( block1, new IntInstr( OP_ADD ) );

		case IDENT_EXPR:
		case STR_EXPR:
		case INT_EXPR:
			return new IntInstr( OP_PUSH, root->m_symbol );

		case COERCE_TO_STR:
			block1 = generateIntCode( root->m_child[0] );
			block2 = new IntInstr( OP_INT2STR );
			return concatenate( block1, block2 );

		case COERCE_TO_INT:
			block1 = generateIntCode( root->m_child[0] );
			block2 = new IntInstr( OP_STR2INT );
			return concatenate( block1, block2 );
	}

	return new IntInstr(  OP_NOP  ); // Shouldn't happen...
}

/******************************************************************************

Parser for the my_c scripting language:

At compile time, the "bison.bat" batch file will be called and the small 
program, "bison.exe" will read this file and generate a two new files called 
"parse.cpp.h" and "parse.cpp". The file "parse.cpp.h" will then be renamed
to lexSymbol.h" for readability.

"lexSymbol.h" will contain the symbolic constants required for compiling, 
which result from the token definitions in this file.

"parse.cpp" will contain the parser functions that will act as the grammar 
rule set for our new scripting language.

******************************************************************************/

%{

/*
  -----------------------------------------------------------------------------
  Initial code (copied verbatim to the output file)
  -----------------------------------------------------------------------------
*/

// Includes
#include <malloc.h>      // _alloca is used by the parser
#include <string.h>      // strcpy
#include "lex.h"
#include "symbolTable.h"
#include "syntaxTree.h"

SymbolTable g_symbolTable;
SyntaxTree  g_syntaxTree = NULL;

// Some yacc/bison defines
#define YYDEBUG 1	        // Generate debug code; needed for YYERROR_VERBOSE
#define YYERROR_VERBOSE     // Give a more specific parse error message

// Error-reporting function must be defined by the caller
void error( char *cFormat, ... );

// Forward declarations
void yyerror( char *msg );
char *makeStringConstantName( void );
char *makeIntegerValueName( void );

%}

/* 
  -----------------------------------------------------------------------------
  Yacc declarations
  ----------------------------------------------------------------------------- 
*/

/* The structure for passing values between lexer and parser */
%union
{
   char     *cString;  /* A character string */
   int       nInteger; /* An integer value */
   Symbol   *symbol;   /* Entry from symbol table */
   TreeNode *tnode;    /* Node in the syntax tree */
}

/* Token definitions */
%token ERROR_TOKEN IF ELSE PRINT INPUT ASSIGN EQUAL
%token ADD END_STMT OPEN_PAR CLOSE_PAR
%token BEGIN_CS END_CS
%token <cString> ID STRING INTEGER

/* Rule type definitions */
%type <symbol> identifier string integer
%type <tnode>  program statement_list statement
%type <tnode>  if_statement optional_else_statement compound_statement
%type <tnode>  expression equal_expression assign_expression
%type <tnode>  add_expression simple_expression

%expect 1  /* shift/reduce conflict: dangling ELSE declaration */

%%

/* 
  -----------------------------------------------------------------------------
  Yacc grammar rules
  -----------------------------------------------------------------------------
*/

program
      : statement_list              {g_syntaxTree = $1;}
	  ;

statement_list
      : statement_list statement    {$$ = new TreeNode( STMT_LIST, $1, $2 );}
      | /* empty */                 {$$ = new TreeNode( EMPTY_STMT );}
      ;

statement
      : END_STMT                    {$$ = new TreeNode( EMPTY_STMT );}
      | expression END_STMT         {$$ = new TreeNode( EXPR_STMT, $1 );}
      | PRINT expression END_STMT   {$$ = new TreeNode( PRINT_STMT, $2 );}
      | INPUT identifier END_STMT   {$$ = new TreeNode( INPUT_STMT ); $$->m_symbol = $2;}
      | if_statement                {$$ = $1;}
      | compound_statement          {$$ = $1;}
      | error END_STMT              {$$ = new TreeNode( ERROR_STMT );}
      ;

/* 
  NOTE: This rule causes an unresolvable shift/reduce conflict;
        that's why %expect 1 was added (see above) 
*/
if_statement
      : IF OPEN_PAR expression CLOSE_PAR statement optional_else_statement
        {
           if( $6 != NULL )
              $$ = new TreeNode( IFTHENELSE_STMT, $3, $5, $6 );
           else
              $$ = new TreeNode( IFTHEN_STMT, $3, $5 );
        }
      ;

optional_else_statement
      : ELSE statement   {$$ = $2;}
      | /* empty */      {$$ = NULL;}
      ;

compound_statement
      : BEGIN_CS statement_list END_CS  {$$ = $2;}
      ;

expression
      : equal_expression  {$$ = $1;}
      ;

equal_expression
      : expression EQUAL assign_expression   {$$ = new TreeNode( EQUAL_EXPR, $1, $3 );}
      | assign_expression                    {$$ = $1;}
      ;

assign_expression
      : identifier ASSIGN assign_expression  {$$ = new TreeNode( ASSIGN_EXPR, $3 ); $$->m_symbol = $1;}
      | add_expression                       {$$ = $1;}
      ;

add_expression
      : add_expression ADD simple_expression  {$$ = new TreeNode( ADD_EXPR, $1, $3 );}
      | simple_expression                     {$$ = $1;}
      ;

simple_expression
      : identifier                     {$$ = new TreeNode( IDENT_EXPR ); $$->m_symbol = $1;}
      | string                         {$$ = new TreeNode( STR_EXPR   ); $$->m_symbol = $1;}
      | integer                        {$$ = new TreeNode( INT_EXPR   ); $$->m_symbol = $1;}
      | OPEN_PAR expression CLOSE_PAR  {$$ = $2;}
      ;

identifier
      : ID
        {
           $$ = g_symbolTable.find( yylval.cString );
           if( $$ == NULL )
		   {
			  /* Doesn't exist yet; create it... */
              $$ = new Symbol( yylval.cString, IDENTIFIER, NULL, lineno );
              g_symbolTable.add( $$ );
           }
        }
      ;

string
      : STRING
        {
           $$ = new Symbol( makeStringConstantName(), STR_CONST, yylval.cString, lineno );
           g_symbolTable.add( $$ );
        }
      ;

integer
      : INTEGER
        {
           $$ = new Symbol( makeIntegerValueName(), INT_VALUE, yylval.nInteger, lineno );
           g_symbolTable.add( $$ );
        }
      ;

%%

/*
  -----------------------------------------------------------------------------
  Additional code (again copied verbatim to the output file)
  -----------------------------------------------------------------------------
*/

//-----------------------------------------------------------------------------
// Name: makeStringConstantName()
// Desc: Generate a unique name for identifying a string constant in the 
//       symbol table.
//-----------------------------------------------------------------------------
char *makeStringConstantName( void )
{
	char *name = new char[15];
	char num[4];
	static n = 0;
	sprintf( num, "%d", ++n );
	strcpy( name, "str_" );
	strcat( name, num );
	return name;
}

//-----------------------------------------------------------------------------
// Name: makeIntegerValueName()
// Desc: Generate a unique name for identifying an integer value in the 
//       symbol table.
//-----------------------------------------------------------------------------
char *makeIntegerValueName( void )
{
	char *name = new char[15];
	char num[4];
	static n = 0;
	sprintf( num, "%d", ++n );
	strcpy( name, "int_" );
	strcat( name, num );
	return name;
}
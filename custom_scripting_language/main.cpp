//-----------------------------------------------------------------------------
//           Name: main.cpp
//         Author: Kevin Harris
//  Last Modified: 09/25/04
//    Description: A simple console app to test the my_c scripting language
//-----------------------------------------------------------------------------

#include <stdarg.h>
#include "parse.h"
#include "lex.h"
#include "symbolTable.h"
#include "syntaxTree.h"
#include "intcode.h"
#include "vm.h"

//-----------------------------------------------------------------------------
// EXTERNALS
//-----------------------------------------------------------------------------
extern SymbolTable g_symbolTable;
extern SyntaxTree  g_syntaxTree;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
IntInstr *g_intCode = NULL;
int       g_nErrors = 0;

//-----------------------------------------------------------------------------
// Name: error()
// Desc: Function used to report errors
//-----------------------------------------------------------------------------
void error( char *cFormat, ... )
{
	va_list args;

	++g_nErrors;
	fprintf( stderr, "Line %d: ", lineno );
	va_start( args, cFormat );
	vfprintf( stderr, cFormat, args );
	va_end( args );
	printf( "\n" );
}

//-----------------------------------------------------------------------------
// Name: errorSummary()
// Desc: Show an error count
//-----------------------------------------------------------------------------
void errorSummary()  
{
   fprintf( stderr, "%d error(s) were found.\n\n", g_nErrors );
}

//-----------------------------------------------------------------------------
// Name: yyerror()
// Desc: Function called by the parser when an error occurs while parsing
//       (parse error or stack overflow)
//-----------------------------------------------------------------------------
void yyerror( char *msg ) 
{
   error( msg );
}

//-----------------------------------------------------------------------------
// Name: yywrap()
// Desc: This function is called by the lexer when the end-of-file is reached; 
//       you can reset yyin (the input FILE*) and return 0 if you want to 
//       rocess another file; otherwise just return 1.
//-----------------------------------------------------------------------------
extern "C" int yywrap( void ) 
{
	return 1;
}

//-----------------------------------------------------------------------------
// Name: main()
// Desc: Set the input stream (either a file from the command line or stdin).
//-----------------------------------------------------------------------------
main( int argc, char *argv[] )
{
	yyin = NULL;

	if( argc == 2 )
		yyin = fopen( argv[1], "rt" );

	if( yyin == NULL )
		yyin = stdin;

	yyparse(); // Call the parser

	errorSummary();

	if( !g_nErrors )
	{
		g_intCode = generateIntCode( g_syntaxTree );
		g_intCode->number(1);

		g_syntaxTree->show();
		g_symbolTable.show();
		g_intCode->show();

		VMachine vm;

		vm.compile();
		vm.execute();
		return 0;
	}

	return 1;
}

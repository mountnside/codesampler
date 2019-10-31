/******************************************************************************

Lexical analyzer for the my_c scripting language:

At compile time, the "flex.bat" batch file will be called and the small 
program, "flex.exe" will read this file and generate a new file called 
"lex.cpp".

"lex.cpp" will contain the tokenizer functions which will act as our new 
scripting language's lexical analyzer.

******************************************************************************/

%{

/*
  -----------------------------------------------------------------------------
  Initial code (copied verbatim to the output file)
  -----------------------------------------------------------------------------
*/

#include <string.h> // strcpy, strncpy
#include <io.h>     // isatty

// For some reason isatty is called _isatty in Visual C++
#ifdef MSVC
#define isatty _isatty
#endif

// Forward declarations for include files
class TreeNode;
class Symbol;

// Make sure our variables get created
#define  _LEX_CPP_
#include "lex.h"
#include "lexSymbol.h"

// The yywrap and error functions are declared by the caller
extern "C" int yywrap();
extern void error( char *cFormat, ... );

// Forward declarations
void passIdentifierName( void );
void passStringConstant( void );
void passIntegerValue( void );
void eatSingleLineComment( void );
void eatMultiLineComment( void );

%}

/* 
  -----------------------------------------------------------------------------
  Some macros (standard regular expressions)
  ----------------------------------------------------------------------------- 
*/

LETTER   [a-zA-Z_]
DIGIT    [0-9]+
IDENT    {LETTER}({LETTER}|{DIGIT})*
STR      \"[^\"]*\"
WSPACE   [ \t]+

/*
  -----------------------------------------------------------------------------
  The lexer rules
  -----------------------------------------------------------------------------
*/

%%

"if"     {return IF;}
"else"   {return ELSE;}
"print"  {return PRINT;}
"input"  {return INPUT;}
"="      {return ASSIGN;}
"=="     {return EQUAL;}
"+"      {return ADD;}
";"      {return END_STMT;}
"("      {return OPEN_PAR;}
")"      {return CLOSE_PAR;}
"{"      {return BEGIN_CS;}
"}"      {return END_CS;}
{IDENT}  {passIdentifierName(); return ID;}     /* identifier: copy name */
{STR}    {passStringConstant(); return STRING;} /* string constant: copy string */
{DIGIT}  {passIntegerValue(); return INTEGER;}  /* integer: copy value */
"//"     {eatSingleLineComment();}              /* single-line comment: skip over line*/
"/*"     {eatMultiLineComment();}               /* begin multi-line comment: skip over all */
"*/"     {}                                     /* end multi-line comment block: do nothing */
\n       {++lineno;}                            /* newline: count lines */
{WSPACE} {}                                     /* whitespace: do nothing */
.        {return ERROR_TOKEN;}                  /* other char: error, illegal token */

%%

/*
  -----------------------------------------------------------------------------
  Additional code (again copied verbatim to the output file)
  -----------------------------------------------------------------------------
*/

//-----------------------------------------------------------------------------
// Name: eatSingleLineComment()
// Desc: Comment-skipping function for skipping to the end of a single line.
//-----------------------------------------------------------------------------
void eatSingleLineComment( void )
{
	register int c;

	while( (c = yyinput()) != '\n' && c != EOF )
	{
		// Eat up the comment's text...
	}

	++lineno;
}

//-----------------------------------------------------------------------------
// Name: eatMultiLineComment()
// Desc: Comment-skipping function for skipping multiple lines.
//-----------------------------------------------------------------------------
void eatMultiLineComment( void )
{
	register int c;

	while(1)
	{
		// Eat up the comment's text, along as it's not a '*'
		while( (c = yyinput()) != '*' && c != EOF )
		{
			if( c == '\n' )
				++lineno;
		}

		if( c == '*' )
		{
			// Eat up any '*' that follow the first one...
			while( (c = yyinput()) == '*' )
			{
				if( c == '\n' )
					++lineno;
			}

			if( c == '/' )
				break; // Found the end!
		}

		if( c == EOF )
		{
			error( "EOF in comment" );
			break;
		}

		if( c == '\n' )
			++lineno;
	}
}

//-----------------------------------------------------------------------------
// Name: passIdentifierName()
// Desc: Pass the identifier name to the parser.
//-----------------------------------------------------------------------------
void passIdentifierName( void )
{
	yylval.cString = new char[strlen(yytext)+1];
	strcpy( yylval.cString, yytext );
}

//-----------------------------------------------------------------------------
// Name: passStringConstant()
// Desc: Pass the string constant to the parser.
//-----------------------------------------------------------------------------
void passStringConstant( void )
{
	int l = ( strlen( yytext ) - 2 );
	yylval.cString = new char[l+1];
	strncpy( yylval.cString, &yytext[1], l );
	yylval.cString[l] = 0;
}

//-----------------------------------------------------------------------------
// Name: passIntegerValue
// Desc: Pass the integer value to the parser.
//-----------------------------------------------------------------------------
void passIntegerValue( void )
{
	yylval.nInteger = atoi( yytext );
}
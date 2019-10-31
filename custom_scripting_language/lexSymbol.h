typedef union
{
   char     *cString;  /* A character string */
   int       nInteger; /* An integer value */
   Symbol   *symbol;   /* Entry from symbol table */
   TreeNode *tnode;    /* Node in the syntax tree */
} YYSTYPE;
#define	ERROR_TOKEN	258
#define	IF	259
#define	ELSE	260
#define	PRINT	261
#define	INPUT	262
#define	ASSIGN	263
#define	EQUAL	264
#define	ADD	265
#define	END_STMT	266
#define	OPEN_PAR	267
#define	CLOSE_PAR	268
#define	BEGIN_CS	269
#define	END_CS	270
#define	ID	271
#define	STRING	272
#define	INTEGER	273


extern YYSTYPE yylval;

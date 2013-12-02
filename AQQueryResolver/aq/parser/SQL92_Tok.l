%{

//#include "sql92_grm_tab.h"
// int wordCount = 0;
#define YY_NO_UNISTD_H

#ifdef WIN32
#include <io.h>
#else
#include <stdio.h>
#include <unistd.h>
#endif

%}

%{
// %option yylineno debug 
// %option reentrant
%}

%option yylineno

char			[A-Za-z\_\x80-\xFF]
char_			[A-Za-z\_\x80-\xFF]
char2			[A-Za-z\_\'\.\"\x80-\xFF]
char3			[a-zA-Z0-9\_]+
numbers			([0-9])+
delim			[" "\n\t\r]
whitespace		{delim}
whitespaces		{delim}+
ws_no_newline	[" "\t]+
words			{char2}+
digit			[0-9]
digit2			[0-9][0-9]
digit4			[0-9][0-9][0-9][0-9]
digits			{digit}+
char_digit		[A-Za-z\_0-9\x80-\xFF]



AND				[Aa][Nn][Dd]
OR				[Oo][Rr]
NOT				[Nn][Oo][Tt]

ABS				[Aa][Bb][Ss]
ALL				[Aa][Ll][Ll]	
ANY				[Aa][Nn][Yy]
AS				[Aa][Ss]
ASC				[Aa][Ss][Cc]
AVG				[Aa][Vv][Gg]
BETWEEN			[Bb][Ee][Tt][Ww][Ee][Ee][Nn]
BY				[Bb][Yy]
CASE			[Cc][Aa][Ss][Ee]
CAST			[Cc][Aa][Ss][Tt]
CREATE			[Cc][Rr][Ee][Aa][Tt][Ee]
COMMIT			[Cc][Oo][Mm][Mm][Ii][Tt]
COUNT			[Cc][Oo][Uu][Nn][Tt]
CURRENT			[Cc][Uu][Rr][Rr][Ee][Nn][Tt]
CURRENT_DATE	[Cc][Uu][Rr][Rr][Ee][Nn][Tt][_][Dd][Aa][Tt][Ee]
DATE			[Dd][Aa][Tt][Ee]
DAY				[Dd][Aa][Yy]
DECODE			[Dd][Ee][Cc][Oo][Dd][Ee]
DEFAULT			[Dd][Ee][Ff][Aa][Uu][Ll][Tt]
DEFERRED		[Dd][Ee][Ff][Ee][Rr][Rr][Ee][Dd]
DELETE			[Dd][Ee][Ll][Ee][Tt][Ee]
DESC			[Dd][Ee][Ss][Cc]
DISTINCT		[Dd][Ii][Ss][Tt][Ii][Nn][Cc][Tt]
ELSE			[Ee][Ll][Ss][Ee]
END				[Ee][Nn][Dd]
EXISTS			[Ee][Xx][Ii][Ss][Tt][Ss]
EXTRACT			[Ee][Xx][Tt][Rr][Aa][Cc][Tt]
ESCAPE			[Ee][Ss][Cc][Aa][Pp][Ee]
FIRST_VALUE		[Ff][Ii][Rr][Ss][Tt][_][Vv][Aa][Ll][Uu][Ee]
IMMEDIATE		[Ii][Mm][Mm][Ee][Dd][Ii][Aa][Tt][Ee]
FOLLOWING		[Ff][Oo][Ll][Ll][Oo][Ww][Ii][Nn][Gg]
FOR				[Ff][Oo][Rr]
FRAME			[Ff][Rr][Aa][Mm][Ee]
FROM			[Ff][Rr][Oo][Mm]
FULL			[Ff][Uu][Ll][Ll]
FUNC			[aA][qQ][_]{char3}
GROUP			[Gg][Rr][Oo][Uu][Pp]
HAVING			[Hh][Aa][Vv][Ii][Nn][Gg]
IN				[Ii][Nn]
INNER			[Ii][Nn][Nn][Ee][Rr]
INSERT			[Ii][Nn][Ss][Ee][Rr][Tt]
INTEGER_TYPE	[Ii][Nn][Tt][Ee][Gg][Ee][Rr]
INTERVAL		[Ii][Nn][Tt][Ee][Rr][Vv][Aa][Ll]
INTO			[Ii][Nn][Tt][Oo]
IS				[Ii][Ss]
JOIN			[Jj][Oo][Ii][Nn]
LAG				[Ll][Aa][Gg]
LAST_VALUE		[Ll][Aa][Ss][Tt][_][Vv][Aa][Ll][Uu][Ee]
LEAD			[Ll][Ee][Aa][Dd]
LEFT			[Ll][Ee][Ff][Tt]
LIKE			[Ll][Ii][Kk][Ee]
MATCHED			[Mm][Aa][Tt][Cc][Hh][Ee][Dd]
MAX				[Mm][Aa][Xx]
MERGE			[Mm][Ee][Rr][Gg][Ee]
MIN				[Mm][Ii][Nn]
MINUS			[Mm][Ii][Nn][Uu][Ss]
MONTH			[Mm][Oo][Nn][Tt][Hh]
NATURAL			[Nn][Aa][Tt][Uu][Rr][Aa][Ll]
nullptr			[Nn][Uu][Ll][Ll]
NVL				[Nn][Vv][Ll]
ON				[Oo][Nn]
ORDER			[Oo][Rr][Dd][Ee][Rr]
OUTER			[Oo][Uu][Tt][Ee][Rr]
OVER			[Oo][Vv][Ee][Rr]
PARTITION		[Pp][Aa][Rr][Tt][Ii][Tt][Ii][Oo][Nn]
PRECEDING		[Pp][Rr][Ee][Cc][Ee][Dd][Ii][Nn][Gg]
RANGE			[Rr][Aa][Nn][Gg][Ee]
REAL_TYPE		[Rr][Ee][Aa][Ll]
REPLACE			[Rr][Ee][Pp][Ll][Aa][Cc][Ee]
RIGHT			[Rr][Ii][Gg][Hh][Tt]
ROLLBACK		[Rr][Oo][Ll][Ll][Bb][Aa][Cc][Kk]
ROW				[Rr][Oo][Ww]
ROW_NUMBER		[Rr][Oo][Ww][_][Nn][Uu][Mm][Bb][Ee][Rr]
ROWS			[Rr][Oo][Ww][Ss]
SELECT			[Ss][Ee][Ll][Ee][Cc][Tt]
SET				[Ss][Ee][Tt]
SOME			[Ss][Oo][Mm][Ee]
SOURCE			[Ss][Oo][Uu][Rr][Cc][Ee]
SQRT			[Ss][Qq][Rr][Tt]
STRING_TYPE		[Ss][Tt][Rr][Ii][Nn][Gg]
SUBSTR  		[Ss][Uu][Bb][Ss][Tt][Rr]
SUBSTRING		[Ss][Uu][Bb][Ss][Tt][Rr][Ii][Nn][Gg]
SUM				[Ss][Uu][Mm]
TABLE			[Tt][Aa][Bb][Ll][Ee]
THEN			[Tt][Hh][Ee][Nn]
TRANSACTION		[Tt][Rr][Aa][Nn][Ss][Aa][Cc][Tt][Ii][Oo][Nn]
UNION			[Uu][Nn][Ii][Oo][Nn]
UNBOUNDED		[Uu][Nn][Bb][Oo][Uu][Nn][Dd][Ee][Dd]
UPDATE			[Uu][Pp][Dd][Aa][Tt][Ee]
USING			[Uu][Ss][Ii][Nn][Gg]
VALUES			[Vv][Aa][Ll][Uu][Ee][Ss]
WHEN			[Ww][Hh][Ee][Nn]
WHERE			[Ww][Hh][Ee][Rr][Ee]
WINDOW			[Ww][Ii][Nn][Dd][Oo][Ww]
WORK			[Ww][Oo][Rr][Kk]
YEAR			[Yy][Ee][Aa][Rr]
TARGET			[Tt][Aa][Rr][Gg][Ee][Tt]
TO				[Tt][Oo]
TO_DATE			[Tt][Oo]\_[Dd][Aa][Tt][Ee]
TO_CHAR			[Tt][Oo][_][Cc][Hh][Aa][Rr]
TRUE			[Tt][Rr][Uu][Ee]
TRUNCATE		[Tt][Rr][Uu][Nn][Cc][Aa][Tt][Ee]

FALSE			[Ff][Aa][Ll][Ss][Ee]
UNKNOWN			[Uu][Nn][Kk][Nn][Oo][Ww][Nn]




%%

	/* COMMENT SKIPPING */
"--"[^\n]*"\n"	/* eat up one-line comments */
"/*"			{
					/* eat up C style nested comments */
					register int c;
					register int nNestingLevel;

					nNestingLevel = 1;
					//printf( "In-> Nesting Level = %u\n", nNestingLevel );
					for ( ; ; ) {
						while ( ( c = yyinput() ) != '*' && c != '/' && c != EOF )
							;		/* eat up text of comment */

						if ( c == '/' ) {
							while ( ( c = yyinput() ) == '/' )
								;	/* eat up '/'-s from text of comment */
							if ( c == '*' ) {
								nNestingLevel++;	/* enter in more nested level */
								//printf( "In-> Nesting Level = %u\n", nNestingLevel );
							}
						} else if ( c == '*' ) {
							while ( ( c = yyinput() ) == '*' )
								;	/* eat up '*'-s from text of comment */
							if ( c == '/' ) {
								nNestingLevel--;
								//printf( "Out<- Nesting Level = %u\n", nNestingLevel );
								if ( nNestingLevel == 0 )
									break;    /* found the nested comments outer end */
							}
						}				
						if ( c == EOF ) {
							yyerror( "EOF in comment" );
							break;
						}
					}
				}

	/* OPERATOR RECOGNITIONS */
{AND}			{ yylval = new tnode( K_AND ); return K_AND; }
{OR}			{ yylval = new tnode( K_OR ); return K_OR; }
"+"				{ yylval = new tnode( K_PLUS ); return K_PLUS; }
"-"				{ yylval = new tnode( K_MINUS ); return K_MINUS; }
"*"				{ yylval = new tnode( K_MUL ); return K_MUL; }
"/"				{ yylval = new tnode( K_DIV ); return K_DIV; }
"="				{ yylval = new tnode( K_EQ ); return K_EQ; /* K_ATTRIB ? */ }
"<"				{ yylval = new tnode( K_LT ); return K_LT; }
">"				{ yylval = new tnode( K_GT ); return K_GT; }
"<>"			{ yylval = new tnode( K_NEQ ); return K_NEQ; }
"<="			{ yylval = new tnode( K_LEQ ); return K_LEQ; }
">="			{ yylval = new tnode( K_GEQ ); return K_GEQ; }
{NOT}			{ yylval = new tnode( K_NOT ); return K_NOT; }
"||"			{ yylval = new tnode( K_CONCAT ); return K_CONCAT; }


	/* SPECIAL CHARACTERS */
"."				{ yylval = new tnode( K_PERIOD ); return K_PERIOD; /* K_DOT; */ }
","				{ yylval = new tnode( K_COMMA ); return K_COMMA; }
";"				{ yylval = nullptr; return K_SEMICOLON; }

	/* PARENTHESIS, BRACKET, BRACE (ACCOLADE) */
	/*	'(', ')'    parenthesis ( Round brackets )				*/
	/*	'[', ']'    brackets ( Square brackets )				*/
	/*	'{', '}'    braces ( Brace brackets, curly brackets )	*/
	/*	'<', '>'	Angle brackets - NOT used					*/					

"("				{ yylval = nullptr; return K_LPAREN; }
")"				{ yylval = nullptr; return K_RPAREN; }
"{"				{ yylval = nullptr; return K_LBRACE; }
"}"				{ yylval = nullptr; return K_RBRACE; }
"["				{ yylval = nullptr; return K_LBRACKETS; }
"]"				{ yylval = nullptr; return K_RBRACKETS; }

	/* CONSTANTS */

				/* Numeric Constants - Integer */
{digits}	|
\-{digits}		{	
					/* yylval.nVal = atoi(yytext); */ 
					yylval = new tnode( K_INTEGER );
					yylval->set_int_data( atoi( yytext ) );
					return K_INTEGER; 
				}

				/* Numeric Constants - Numerci (float)  */
				/* {digits}.[{digits}]["e"[+-]{digits}] */
				/* [{digits}].{digits}["e"[+-]{digits}] */
				/* {digits}e[+-]{digits}                */

{digits}"."{digit}*([eE][+-]?{digit}+)?		| 
"."{digits}([eE][+-]?{digit}+)?				|
{digits}[eE][+-]?{digits}					|
\-{digits}"."{digit}*([eE][+-]?{digit}+)?	| 
\-"."{digits}([eE][+-]?{digit}+)?			|
\-{digits}[eE][+-]?{digits}				{ 
											/* yylval.dVal = atof( yytext ); */
											yylval = new tnode( K_REAL );
											yylval->set_double_data( atof( yytext ) );
											return K_REAL; 
										}

				/* Date Constants - 'MM/DD/YYYY HH24:MI:SS' */
"'"{digit2}"/"{digit2}"/"{digit4}" "{digit2}":"{digit2}":"{digit2}"'" { 
											yylval = new tnode( K_DATE_VALUE );
											yylval->set_string_data( yytext );
											return K_DATE_VALUE; 
										}

				/* String Constants */
"'"([^']*"''"*"\\'"*("'"" "*"\n"+" "*"'")*[^']*)*"'"	{ 
											/* yylval.pStr = yytext */; 
											yylval = new tnode( K_STRING );
											/* Cut off the start and end ' char ! */
											yytext[ strlen( yytext ) - 1 ] = '\0';
											yylval->set_string_data( yytext + 1 );
											return K_STRING; 
										}


	/* KEY_WORDS */

	
{FUNC}*			{
				 /* yylval.pStr = yytext */
				 yylval = new tnode( K_FUNC );
				 yylval->set_string_data( yytext );
				 return K_FUNC;
				}

{ABS}			{ yylval = new tnode( K_ABS ); return K_ABS; }
{ALL}			{ yylval = new tnode( K_ALL ); return K_ALL; }
{ANY}			{ yylval = new tnode( K_ANY ); return K_ANY; }
{AS}			{ yylval = new tnode( K_AS ); return K_AS; }
{ASC}			{ yylval = new tnode( K_ASC ); return K_ASC; }
{AVG}			{ yylval = new tnode( K_AVG ); return K_AVG; }
{BETWEEN}		{ yylval = new tnode( K_BETWEEN ); return K_BETWEEN; }
{BY}			{ yylval = new tnode( K_BY ); return K_BY; }
{CASE}			{ yylval = new tnode( K_CASE ); return K_CASE; }
{CAST}			{ yylval = new tnode( K_CAST ); return K_CAST; }
{CREATE}		{ yylval = new tnode( K_CREATE ); return K_CREATE; }
{COMMIT}		{ yylval = new tnode( K_COMMIT ); return K_COMMIT; }
{COUNT}			{ yylval = new tnode( K_COUNT ); return K_COUNT; }
{CURRENT}		{ yylval = new tnode( K_CURRENT ); return K_CURRENT; }
{CURRENT_DATE}	{ yylval = new tnode( K_CURRENT_DATE ); return K_CURRENT_DATE; }
{DATE}			{ yylval = new tnode( K_DATE ); return K_DATE; }
{DAY}			{ yylval = new tnode( K_DAY ); return K_DAY; }
{DECODE}		{ yylval = new tnode( K_DECODE ); return K_DECODE; }
{DEFAULT}		{ yylval = new tnode( K_DEFAULT ); return K_DEFAULT; }
{DEFERRED}		{ yylval = new tnode( K_DEFERRED ); return K_DEFERRED; }
{DELETE}		{ yylval = new tnode( K_DELETE ); return K_DELETE; }
{DESC}			{ yylval = new tnode( K_DESC ); return K_DESC; }
{DISTINCT}		{ yylval = new tnode( K_DISTINCT ); return K_DISTINCT; }
{ELSE}			{ yylval = new tnode( K_ELSE ); return K_ELSE; }
{END}			{ yylval = new tnode( K_END ); return K_END; }
{EXISTS}		{ yylval = new tnode( K_EXISTS ); return K_EXISTS; }
{EXTRACT}		{ yylval = new tnode( K_EXTRACT ); return K_EXTRACT; }
{ESCAPE}		{ yylval = new tnode( K_ESCAPE ); return K_ESCAPE; }
{FIRST_VALUE}	{ yylval = new tnode( K_FIRST_VALUE ); return K_FIRST_VALUE; }
{IMMEDIATE}		{ yylval = new tnode( K_IMMEDIATE ); return K_IMMEDIATE; }
{FOLLOWING}		{ yylval = new tnode( K_FOLLOWING ); return K_FOLLOWING; }
{FOR}			{ yylval = new tnode( K_FOR ); return K_FOR; }
{FRAME}			{ yylval = new tnode( K_FRAME ); return K_FRAME; }
{FROM}			{ yylval = new tnode( K_FROM ); return K_FROM; }
{FULL}			{ yylval = new tnode( K_FULL ); return K_FULL; }
{GROUP}			{ yylval = new tnode( K_GROUP ); return K_GROUP; }
{HAVING}		{ yylval = new tnode( K_HAVING ); return K_HAVING; }
{IN}			{ yylval = new tnode( K_IN ); return K_IN; }
{INNER}			{ yylval = new tnode( K_INNER ); return K_INNER; }
{INSERT}		{ yylval = new tnode( K_INSERT ); return K_INSERT; }
{INTEGER_TYPE}	{ yylval = new tnode( K_INTEGER_TYPE ); return K_INTEGER_TYPE; }
{INTERVAL}		{ yylval = new tnode( K_INTERVAL ); return K_INTERVAL; }
{INTO}			{ yylval = new tnode( K_INTO ); return K_INTO; }
{IS}			{ yylval = new tnode( K_IS ); return K_IS; }    
{JOIN}			{ yylval = new tnode( K_JOIN ); return K_JOIN; }
{LAG}			{ yylval = new tnode( K_LAG ); return K_LAG; }
{LAST_VALUE}	{ yylval = new tnode( K_LAST_VALUE ); return K_LAST_VALUE; }
{LEAD}			{ yylval = new tnode( K_LEAD ); return K_LEAD; }
{LEFT}			{ yylval = new tnode( K_LEFT ); return K_LEFT; }
{LIKE}			{ yylval = new tnode( K_LIKE ); return K_LIKE; }
{MATCHED}		{ yylval = new tnode( K_MATCHED ); return K_MATCHED; }
{MAX}			{ yylval = new tnode( K_MAX ); return K_MAX; }
{MERGE}			{ yylval = new tnode( K_MERGE ); return K_MERGE; }
{MIN}			{ yylval = new tnode( K_MIN ); return K_MIN; }
{MINUS}			{ yylval = new tnode( K_SEL_MINUS ); return K_SEL_MINUS; }
{MONTH}			{ yylval = new tnode( K_MONTH ); return K_MONTH; }
{NATURAL}		{ yylval = new tnode( K_NATURAL ); return K_NATURAL; }
{nullptr}			{ yylval = new tnode( K_nullptr ); return K_nullptr; }
{NVL}			{ yylval = new tnode( K_NVL ); return K_NVL; }
{ON}			{ yylval = new tnode( K_ON ); return K_ON; }
{ORDER}			{ yylval = new tnode( K_ORDER ); return K_ORDER; }
{OUTER}			{ yylval = new tnode( K_OUTER ); return K_OUTER; }
{OVER}			{ yylval = new tnode( K_OVER ); return K_OVER; }
{PARTITION}		{ yylval = new tnode( K_PARTITION ); return K_PARTITION; }
{PRECEDING}		{ yylval = new tnode( K_PRECEDING ); return K_PRECEDING; }
{RANGE}			{ yylval = new tnode( K_RANGE ); return K_RANGE; }
{REAL_TYPE}		{ yylval = new tnode( K_REAL_TYPE ); return K_REAL_TYPE; }
{REPLACE}		{ yylval = new tnode( K_REPLACE ); return K_REPLACE; }
{RIGHT}			{ yylval = new tnode( K_RIGHT ); return K_RIGHT; }
{ROLLBACK}		{ yylval = new tnode( K_ROLLBACK ); return K_ROLLBACK; }
{ROW}			{ yylval = new tnode( K_ROW ); return K_ROW; }
{ROW_NUMBER}	{ yylval = new tnode( K_ROW_NUMBER ); return K_ROW_NUMBER; }
{ROWS}			{ yylval = new tnode( K_ROWS ); return K_ROWS; }
{SELECT}		{ yylval = new tnode( K_SELECT ); return K_SELECT; }
{SET}			{ yylval = new tnode( K_SET ); return K_SET; }
{SQRT}			{ yylval = new tnode( K_SQRT ); return K_SQRT; }
{SOME}			{ yylval = new tnode( K_SOME ); return K_SOME; }
{SUBSTR}		{ yylval = new tnode( K_SUBSTRING ); return K_SUBSTRING; }
{SOURCE}		{ yylval = new tnode( K_SOURCE ); return K_SOURCE; }
{SUBSTRING}		{ yylval = new tnode( K_SUBSTRING ); return K_SUBSTRING; }
{STRING_TYPE}	{ yylval = new tnode( K_STRING_TYPE ); return K_STRING_TYPE; }
{SUM}			{ yylval = new tnode( K_SUM ); return K_SUM; }
{TARGET}		{ yylval = new tnode( K_TARGET ); return K_TARGET; }
{TABLE}			{ yylval = new tnode( K_TABLE ); return K_TABLE; }
{THEN}			{ yylval = new tnode( K_THEN ); return K_THEN; }
{TRANSACTION}	{ yylval = new tnode( K_TRANSACTION ); return K_TRANSACTION; }
{TRUNCATE}		{ yylval = new tnode( K_TRUNCATE ); return K_TRUNCATE; }
{UNBOUNDED}		{ yylval = new tnode( K_UNBOUNDED ); return K_UNBOUNDED; }
{UNION}			{ yylval = new tnode( K_UNION ); return K_UNION; }
{UPDATE}		{ yylval = new tnode( K_UPDATE ); return K_UPDATE; }
{USING}			{ yylval = new tnode( K_USING ); return K_USING; }
{VALUES}		{ yylval = new tnode( K_VALUES ); return K_VALUES; }
{WHEN}			{ yylval = new tnode( K_WHEN ); return K_WHEN; }
{WHERE}			{ yylval = new tnode( K_WHERE ); return K_WHERE; }
{WORK}			{ yylval = new tnode( K_WORK ); return K_WORK; }
{YEAR}			{ yylval = new tnode( K_YEAR ); return K_YEAR; }
{TO}			{ yylval = new tnode( K_TO ); return K_TO; }

{TRUE}			{ yylval = new tnode( K_TRUE ); return K_TRUE; }
{FALSE}			{ yylval = new tnode( K_FALSE ); return K_FALSE; }
{UNKNOWN}		{ yylval = new tnode( K_UNKNOWN ); return K_UNKNOWN; }

{TO_DATE}		{ yylval = new tnode( K_TO_DATE ); return K_TO_DATE; }
{TO_CHAR}		{ yylval = new tnode( K_TO_CHAR ); return K_TO_CHAR; }

	/* IDENTIFIERS - are case insensitive */

{char}{char_digit}*		{ 
							/* yylval.pStr = yytext */; 
							yylval = new tnode( K_IDENT );
							yylval->set_string_data( yytext );
							return K_IDENT; 
						}

	

	/* !!!!!!!! */

{whitespaces}			{ int i; i = 0; } /* Eat White Spaces ! {ws_no_newline} -> { return *yytext; } */

.						{	/* Catch All Rule */
							char szBuf[ 100 ];
							sprintf( szBuf, "Unsupported character <%c> (0x%x) encountered at line number: %d\n", *yytext, (int)(unsigned char)*yytext, yylineno );
							yyerror( szBuf ); 
							yyterminate();
						} 
%%


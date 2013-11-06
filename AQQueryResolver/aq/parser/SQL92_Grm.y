%defines
%{
// %option bison-bridge
// %option bison-locations
// %option noyywrap

/* define stack type */
#define YYSTYPE aq::tnode*
#define YYPARSE_PARAM ppNode

/* define error & debugging flags */
#define YYERROR_VERBOSE
#define YYDEBUG 1

#include "SQLParser.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>

// Forward Definitions
int yyerror( const char *pszMsg );

int yylex( void );
        
using namespace aq;

%}

%start startsymbol

%token K_AND K_OR K_PLUS K_MINUS K_MUL K_DIV K_EQ K_LT K_GT 
%token K_NEQ K_LEQ K_GEQ K_NOT K_CONCAT
	/* K_MUL echivalent with asterisk & K_STAR ! */
%token K_STAR
	/* K_UMINUS echivalent with K_MINUS ! */
%token K_UMINUS
	/* K_JEQ echivalent with K_EQ but for columns equality ! */
%token K_JEQ
	/* K_JAUTO echivalent with K_JEQ but for table.columns equality ! */
%token K_JAUTO

%token K_PERIOD K_COMMA K_SEMICOLON
	/* K_DOT */

%token K_INTEGER K_REAL K_STRING K_DATE_VALUE K_IDENT

%token K_TRUE K_FALSE K_UNKNOWN

%token K_ALL K_ANY K_AS K_ASC K_AVG K_BETWEEN K_BY K_CASE K_COMMIT K_COUNT K_DATE
%token K_DAY K_DEFAULT K_DEFERRED K_DELETE K_DESC K_DISTINCT K_ELSE 
%token K_END K_EXISTS K_EXTRACT K_ESCAPE K_IMMEDIATE K_FOR K_FROM K_FULL
%token K_GROUP K_HAVING K_IN K_INNER K_INSERT K_INTERVAL K_INTO
%token K_IS K_JOIN K_LEFT K_LIKE K_MAX K_MIN K_MONTH K_NATURAL
%token K_NULL K_ON K_ORDER K_OUTER K_RIGHT K_ROLLBACK K_SELECT K_SET 
%token K_SUBSTRING K_SUM K_TABLE K_THEN K_TRANSACTION K_UNION K_UPDATE
%token K_VALUES K_WHEN K_WHERE K_WORK K_YEAR

%token K_FUNC K_TO K_TO_DATE K_TO_CHAR

%token K_CALL K_COLUMNS K_LIST K_OUTREF K_SOURCE K_START 

%token K_COLUMN

%token K_LPAREN K_RPAREN K_LBRACE K_RBRACE K_LBRACKETS K_RBRACKETS
%token K_SOME K_REPLACE

%token K_DELETED

%token K_FIRST_VALUE K_LAST_VALUE K_LEAD K_LAG
%token K_OVER K_PARTITION
%token K_SQRT K_ABS
%token K_FRAME K_ROWS K_RANGE K_PRECEDING K_FOLLOWING K_UNBOUNDED K_CURRENT K_ROW
%token K_CAST K_STRING_TYPE K_REAL_TYPE K_INTEGER_TYPE
%token K_CREATE K_INSERT_ARGS K_CURRENT_DATE K_NVL K_UPDATE_ARGS K_ROW_NUMBER
%token K_TRUNCATE K_DECODE K_NOT_BETWEEN K_NOT_LIKE K_NOT_IN K_JNO
%token K_JINF K_JIEQ K_JSEQ K_JSUP K_JNEQ
%token K_SEL_MINUS K_IN_VALUES
%token K_MERGE K_MATCHED K_USING K_MERGE_ARGS1 K_MERGE_ARGS2 K_TARGET

%left K_PLUS K_MINUS
%left K_MUL K_DIV


%%

	/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*/

startsymbol : select_stmt cmd_term			{
												*(aq::tnode**)ppNode = $$;
												YYACCEPT;
											}
			| create_table_stmt cmd_term	{
												*(aq::tnode**)ppNode = $$;
												YYACCEPT;
											}
			| insert_stmt					{
												*(aq::tnode**)ppNode = $$;
												YYACCEPT;
											}
			| update_stmt					{
												*(aq::tnode**)ppNode = $$;
												YYACCEPT;
											}					
			| delete_stmt					{
												*(aq::tnode**)ppNode = $$;
												YYACCEPT;
											}
			| truncate_stmt					{
												*(aq::tnode**)ppNode = $$;
												YYACCEPT;
											}
			| union_minus_stmt				{
												*(aq::tnode**)ppNode = $$;
												YYACCEPT;
											}
			| merge_stmt					{
												*(aq::tnode**)ppNode = $$;
												YYACCEPT;
											}
			;

cmd_term	: K_SEMICOLON /* ';' */
			;

	/*======================================================*/
	/* Select - From - Where - Group_by - Order_by - Having */
	/*======================================================*/
select_stmt : K_SELECT 
				set_quantifier 
				select_list 
				table_expression			{	aq::tnode *pNode;
												$$ = $1;
												if ( $2 != NULL ) {
													pNode = $2;
													$1->left = pNode;
												} else {
													pNode = $1;
												}
												pNode->left = $3;
												$1->next = $4;
											}
			;

subquery	: K_LPAREN query_exp K_RPAREN	{ $$ = $2; }
			;

query_exp	: select_stmt
			;


set_quantifier  : K_ALL
				| K_DISTINCT 
				| /* Nothing - ALL is implicit */	{ $$ = NULL; }
				;

select_list		: K_MUL	/* Asterisk */				{ $$ = $1; $$->setTag(K_STAR); }
				| select_sublist_rec				// { $$ = $1; }
				;

select_sublist_rec	: select_sublist				
					| select_sublist_rec K_COMMA select_sublist	{
														$2->left		= $1;
														$2->right	= $3;
														$$			= $2;
														// $1->list.push_back($3);
														// $$ = $1;
													}
					;

select_sublist	: derived_column
				| qualifier K_PERIOD K_MUL			{
														$2->left		= $1;
														$2->right	= $3;
														$$			= $2;
													}
				;

derived_column	: value_expression
				| value_expression as_clause		{
														/* $2.tag == K_AS */
														if ( $2->left != NULL ) {
															$2->right = $2->left;
														}
														$2->left = $1;
														$$ = $2;
													}
 				| replace_clause
				| replace_clause as_clause			{
														/* $2.tag == K_AS */
														if ( $2->left != NULL ) {
															$2->right = $2->left;
														}
														$2->left = $1;
														$$ = $2;
													}
				;

replace_clause : K_REPLACE K_LPAREN replace_clause K_COMMA K_STRING K_COMMA K_STRING K_RPAREN { 
														$1->left = $3;
														$1->right = $6;
														$6->left = $5;
														$6->right = $7;
														$$ = $1;
													}
				| K_REPLACE K_LPAREN column_reference K_COMMA K_STRING K_COMMA K_STRING K_RPAREN { 
														$1->left = $3;
														$1->right = $6;
														$6->left = $5;
														$6->right = $7;
														$$ = $1;
													}
				| K_REPLACE K_LPAREN K_STRING K_COMMA K_STRING K_COMMA K_STRING K_RPAREN { 
														$1->left = $3;
														$1->right = $6;
														$6->left = $5;
														$6->right = $7;
														$$ = $1;
													}
				;

as_clause	: column_name							{
														aq::tnode *pNode = new tnode( K_AS );
														$$ = pNode;

														/* Generate Qualified Column Reference */
														#ifdef USE_TSELRESULT_FOR_GENERATED_TABLE
														/* Use for the generated table : TSelResult */
														pNode = new tnode( K_PERIOD );
														$$->left	 = pNode;
														pNode->left  = new tnode( K_IDENT );
														pNode->left->set_string_data( "TSelResult" );
														pNode->right = $1;
														#else
														$$->right = $1;
														#endif
													}
			| K_AS column_name						{
														#ifdef USE_TSELRESULT_FOR_GENERATED_TABLE
														aq::tnode *pNode;
														#endif

														$$ = $1;
														/* $1->left = $2; */
														/* Generate Qualified Column Reference */
														#ifdef USE_TSELRESULT_FOR_GENERATED_TABLE
														/* Use for the generated table : TSelResult */
														pNode = new tnode( K_PERIOD );
														$$->left	 = pNode;
														pNode->left  = new tnode( K_IDENT );
														pNode->left->set_string_data( "TSelResult" );
														pNode->right = $2;
														#else
														$1->right = $2;
														#endif

													}
			;
	
table_expression :  from_clause 
					where_clause 
					group_by_clause 
					having_clause 
					/* [ [ UNION | UNION ALL | INTERSECT | MINUS ] select-statement ]... */
					order_by_clause					{
														aq::tnode *pNode;
														$$		= $1;
														pNode	= $1;
														if ( $2 != NULL ) {
															pNode->next = $2;
															pNode		= $2;
														}
														if ( $3 != NULL ) {
															pNode->next = $3;
															pNode		= $3;
														}
														if ( $4 != NULL ) {
															pNode->next = $4;
															pNode		= $4;
														}
														if ( $5 != NULL ) {
															pNode->next = $5;
															pNode		= $5;
														}
													}
				 ;

from_clause : K_FROM table_reference_list			{
														$1->left = $2;
														$$ = $1;
													}

				/* table_reference - Moved into joined_table rule */
table_reference_list : joined_table
					 | table_reference_list K_COMMA joined_table	{	/* K_COMMA table_reference */
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
					 ;

joined_table : table_reference
			| joined_table joined_type table_reference {
														aq::tnode *pNode;
														pNode = get_leftmost_child( $2 );
														pNode->left	= $1;
														pNode->right= $3;
														$$			= $2;
													}
			| joined_table joined_type table_reference K_ON search_condition {
														aq::tnode *pNode;
														pNode = get_leftmost_child( $2 );
														pNode->left	= $1;
														pNode->right= $3;
														pNode->next	= $4;
														$4->left	= $5;
														$$			= $2;
													}
			;

	/* [ K_INNER | { { K_LEFT | K_RIGHT | K_FULL } [ K_OUTER ] } ] K_JOIN */
joined_type :  K_INNER K_JOIN						{	
														$1->left	= $2; 
														$$			= $1;
													}
			|   K_LEFT K_JOIN						{	
														$1->left	= $2; 
														$$			= $1;
													}
			|   K_LEFT K_OUTER K_JOIN				{	
														$1->left	= $2; 
														$2->left	= $3; 
														$$			= $1;
													}
			|   K_RIGHT K_JOIN						{	
														$1->left	= $2; 
														$$			= $1;
													}
			|   K_RIGHT K_OUTER K_JOIN				{	
														$1->left	= $2; 
														$2->left	= $3; 
														$$			= $1;
													}
			|   K_FULL K_JOIN						{	
														$1->left	= $2; 
														$$			= $1;
													}
			|   K_FULL K_OUTER K_JOIN				{	
														$1->left	= $2; 
														$2->left	= $3; 
														$$			= $1;
													}
			|	K_JOIN 
			;

where_clause : K_WHERE search_condition				{
														$1->left	= $2;
														$$			= $1;
													}
			 | /* nothing */						{	$$ = NULL; }
			 ;

group_by_clause : K_GROUP K_BY column_name_list		{
														$1->left = $2;
														$2->left = $3;
														$$ = $1;
													}
				| /* nothing */						{	$$ = NULL; }
				;

having_clause	: K_HAVING search_condition			{
														$1->left = $2;
														$$ = $1;
													}
				| /* nothing */						{	$$ = NULL; }
				;

order_by_clause : K_ORDER K_BY sort_specification_list	{
														$1->left = $2;
														$2->left = $3;
														$$ = $1;
													}
				| /* nothing ???? */				{	$$ = NULL; }
				;

				/* This is a simplified version */
table_reference	: table_name
				| table_name correlation_name		{
														aq::tnode *pNode;
														/* Generate K_AS */
														pNode = new tnode( K_AS );
														pNode->left	 = $1;
														pNode->right = $2;
														$$ = pNode;
													}
				| table_name K_AS correlation_name	{
														$2->left = $1;
														$2->right = $3;
														$$ = $2;
													}
				| derived_table			/* [ K_AS correlation_name [ K_LPAREN derived_column_list K_RPAREN ] */
				| derived_table correlation_name	{
														aq::tnode *pNode;
														/* Generate K_AS */
														pNode = new tnode( K_AS );
														pNode->left	 = $1;
														pNode->right = $2;
														$$ = pNode;
													}
				| derived_table K_AS correlation_name	{
														$2->left = $1;
														$2->right = $3;
														$$ = $2;
													}
				;

table_name	: identifier
			;

correlation_name : identifier
				 ;

derived_table	: table_subquery
				;

table_subquery	: subquery
				;

column_name_list : column_reference
				 | column_name_list K_COMMA column_reference	{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
				 ;

column_name : identifier 
			;

sort_specification_list : sort_specification
						| sort_specification_list K_COMMA sort_specification {
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
						;

sort_specification : sort_key ordering_specification	{
														if ( $2 != NULL ) {
															$2->left = $1;
															$$ = $2;
														} else {
															$$ = $1;
														}
													}
				   ;

sort_key	: value_expression						{
														#ifdef USE_TSELRESULT_FOR_GENERATED_TABLE
														aq::tnode *pNode;
														#endif

														/* Generate Qualified Column Reference */
														#ifdef USE_TSELRESULT_FOR_GENERATED_TABLE
														/* Use for the generated table : TSelResult */
														pNode		 = new tnode( K_PERIOD );
														$$			 = pNode;
														pNode->left  = new_node( K_IDENT );
														pNode->left->set_string_data( "TSelResult" );
														pNode->right = $1;
														#else
														$$ = $1;
														#endif
													}
			; 

ordering_specification	: K_ASC 
						| K_DESC
						| /* nothing */				{ $$ = NULL; }
						;

value_expression	: numeric_value_expression
					| string_value_expression
					| datetime_value_expression
				/*	| interval_value_expression
				*/
					;

	/* datetime_value_expression - it is a very complex rule => extremely simplified */
datetime_value_expression	: datetime_term
						/*	| <interval value expression> <plus sign> <datetime term>
							| <datetime value expression> <plus sign> <interval term>
							| <datetime value expression> <minus sign> <interval term>
						*/

datetime_term	: datetime_factor
datetime_factor	: datetime_primary /*[ <time zone> ]*/
datetime_primary	: value_expression_primary
					| datetime_value_function
					| K_DATE_VALUE

datetime_value_function	: K_DATE K_LPAREN datetime_value_expression K_RPAREN {
														$$ = $1;
														$1->left = $3;
													}
						| date_conversion 
						| K_CURRENT_DATE
						;

	/* to_date('1998,091,00:00:00' , 'YYYY,DDD,HH24:MI:SS') */
date_conversion : K_TO_DATE K_LPAREN string_value_expression K_COMMA K_STRING K_RPAREN	{
														$1->left	= $3;
														$1->right	= $5;
														$$			= $1;
													}
				| K_TO_DATE K_LPAREN string_value_expression K_RPAREN	{
														$1->left	= $3;
														$$			= $1;
													}
				;
				
char_conversion : K_TO_CHAR K_LPAREN numeric_value_expression K_RPAREN	{
														$1->left	= $3;
														$$			= $1;
													}
				| K_TO_CHAR K_LPAREN numeric_value_expression 
				  K_COMMA K_STRING K_RPAREN			{
														$1->left	= $3;
														$1->right	= $5;
														$$			= $1;
													}
				| K_TO_DATE K_LPAREN datetime_value_expression K_RPAREN	{
														$1->left	= $3;
														$$			= $1;
													}
				;


numeric_value_expression	: term
							| numeric_value_expression K_PLUS term	{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
							| numeric_value_expression K_MINUS term	{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
							;

term	: factor
		| term K_MUL factor							{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
		| term K_DIV factor							{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
		;

factor	: numeric_primary
		| sign numeric_primary						{	
														$1->setTag(K_UMINUS);
														$1->left	= $2;
														$$			= $1;
													}
		;

sign : K_MINUS
	 ;

numeric_primary	: value_expression_primary
				| numeric_value_function
				| K_INTEGER
				| K_REAL
				;
				
numeric_value_function	: square_root
						| absolute_value_expression
						| year_expresssion
						| month_expresssion
						| day_expresssion
						| aq_function
						;

value_expression_primary	: column_reference
						/*
							| unsigned_value_specification
						*/
							| set_function_specification
						/*
							| scalar_subquery
						*/
							| case_expression
							| K_LPAREN value_expression K_RPAREN	{
														$$ = $2;
													}
							| cast_specification
							| window_function
							| nvl_specification
							| decode_specification
							;

column_reference	: qualifier K_PERIOD column_name	{
														$2->left	= $1;
														$2->right	= $3;
														$3->setTag(K_COLUMN);
														$$			= $2;
													}
					| column_name					{
														$1->setTag(K_COLUMN);
														$$			= $1;
													}
					;

qualifier	: table_name
		/*	| correlation_name
		*/
			;

set_function_specification	: count_all
							| general_set_function
							;

count_all : K_COUNT K_LPAREN K_MUL K_RPAREN			{
														$1->left	= $3;
														$3->setTag(K_STAR);
														$$			= $1;
													}
			| K_COUNT K_LPAREN set_quantifier value_expression K_RPAREN	{
														if ( $3 != NULL ) {
															$1->left	= $3;
															$1->right	= $4;
														} else {
															$1->left	= $4;
														}
														$$ = $1;
													}
			;

general_set_function	: set_function_type 
							K_LPAREN 
							set_quantifier 
							value_expression 
							K_RPAREN				{
														if ( $3 != NULL ) {
															$1->left	= $3;
															$1->right	= $4;
														} else {
															$1->left	= $4;
														}
														$$ = $1;
													}
						;

set_function_type	: K_AVG 
					| K_MAX 
					| K_MIN 
					| K_SUM 
				/*
					| K_COUNT
				*/
					;

case_expression	: case_specification
			/*
				| case_abbreviation
			*/
				;

case_specification	: simple_case
					| searched_case
					;
 
	/* Tree : 
		CASE->left		= case_operand;
		CASE->right		= WHEN[1];
		WHEN[1]->next	= WHEN[2];
		WHEN[1]->left	= when_op;
		WHEN[1]->right	= THEN[1];
		THEN[1]->left	= result;
		WHEN[n]->next	= ELSE;
		ELSE->left		= result(else);
	*/
simple_case	: K_CASE case_operand simple_when_clause_list else_clause K_END	{
														$1->left		= $2;
														$1->right	= $3;
														if ( $4 != NULL ) {
															aq::tnode *pNode;
															pNode = $3;
															while ( pNode->next != NULL )
																pNode = pNode->next;
															pNode->next = $4;
														}
														delete $5;
														$$			= $1;
													}
			;

	/* Tree : 
		CASE->left		= WHEN[1];
		//CASE->right		= ELSE;
		WHEN[1]->next	= WHEN[2];
		WHEN[1]->left	= when_op;
		WHEN[1]->right	= THEN[1];
		THEN[1]->left	= result;
		WHEN[n]->next	= ELSE;
		ELSE->left		= result(else);
	*/
searched_case	: K_CASE searched_when_clause_list else_clause K_END {
														$1->left	= $2;
														/* $1->right	= $3; */
														if ( $3 != NULL ) {
															aq::tnode *pNode;
															pNode = $2;
															while ( pNode->next != NULL )
																pNode = pNode->next;
															pNode->next = $3;
														}
														delete $4;
														$$			= $1;
													}
				;

simple_when_clause_list : simple_when_clause
						| simple_when_clause simple_when_clause_list {
														$1->next	= $2;
														$$			= $1;
													}
						;

simple_when_clause	: K_WHEN when_operand K_THEN result	{
														$1->left	= $2;
														$1->right	= $4;
														delete $3;
														$$			= $1;
													}
					;

searched_when_clause_list	: searched_when_clause
							| searched_when_clause_list searched_when_clause {
														$1->next	= $2;
														$$			= $1;
													}
							;

searched_when_clause	: K_WHEN search_condition K_THEN result {
														$1->left	= $2;
														$1->right	= $3;
														$3->left	= $4;
														$$			= $1;
													}
						;

else_clause	: K_ELSE result							{
														$1->left	= $2;
														$$			= $1;
													}
			| /* nothing */							{	$$ = NULL; }
			;

case_operand	: value_expression
				;

when_operand	: value_expression
				;

result	: result_expression 
		| K_NULL
		;

result_expression	: value_expression
					;


string_value_expression : character_value_expression
					/*
						| bit_value_expression
					*/
						;

character_value_expression	: concatenation
							| character_factor
							;

concatenation	: character_value_expression concatenation_operator character_factor {
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
				;

concatenation_operator	: K_CONCAT
						| K_PLUS
						;

character_factor	: character_primary /* [ <collate clause> ] */
					;

character_primary	: K_STRING		/* Added by Zoli */
					| value_expression_primary
					| string_value_function
					;
					
string_value_function	: binary_substring_function
						| char_conversion
						;

search_condition	: boolean_term
					| search_condition K_OR boolean_term	{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
					;

boolean_term	: boolean_factor
				| boolean_term K_AND boolean_factor	{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
				;

boolean_factor	: K_NOT boolean_test				{
														$1->left	= $2;
														$$			= $1;
													}
				| boolean_test
				;

boolean_test	: boolean_primary K_IS K_NOT truth_value	{
														$2->left	= $1;
														$2->right	= $3;
														$3->left	= $4;
														$$			= $2;
													}
				| boolean_primary K_IS truth_value	{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
				| boolean_primary 
				;

truth_value	: K_TRUE
			| K_FALSE
			| K_UNKNOWN
			;

boolean_primary	: predicate
				| K_LPAREN search_condition K_RPAREN	{ $$ = $2; }
				;

predicate	: comparison_predicate
			| between_predicate
			| in_predicate
			| like_predicate
			| null_predicate
		/*
			| quantified_comparison_predicate
		*/
			| exists_predicate
		/*
			| unique_predicate
			| match_predicate
			| overlaps_predicate
		*/
			;

//---------------
comparison_predicate	: row_value_constructor comp_op row_value_constructor	{
// K_PERIOD
														//#if 0 
														/* Enforce K_JEQ instead of K_EQ ! */
														/* Moved to convert_syntax_tree_to_prefix_form() */
														
														if ( $1->getTag() == K_IDENT || 
															 $1->getTag() == K_COLUMN || 
															( $1->getTag() == K_PERIOD && 
															  $1->left != NULL &&
															  $1->left->getTag() == K_IDENT &&
															  $1->right != NULL &&
															  ( $1->right->getTag() == K_IDENT || 
															    $1->right->getTag() == K_COLUMN ) ) )
															if ( $3->getTag() == K_IDENT || 
																 $3->getTag() == K_COLUMN || 
																( $3->getTag() == K_PERIOD && 
																  $3->left != NULL &&
																  $3->left->getTag() == K_IDENT &&
																  $3->right != NULL &&
																  ( $3->right->getTag() == K_IDENT ||
																    $3->right->getTag() == K_COLUMN ) ) )
																    switch( $2->getTag() )
																    {
																    case K_EQ: $2->setTag(K_JEQ); break;
																    case K_LT: $2->setTag(K_JINF); break;
																    case K_LEQ: $2->setTag(K_JIEQ); break;
																    case K_GT: $2->setTag(K_JSUP) ; break;
																    case K_GEQ: $2->setTag(K_JSEQ); break;
																    case K_NEQ: $2->setTag(K_JNEQ); break;
																    default:;
																    };
														
														//#endif
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
						| row_value_constructor comp_op K_ALL row_subquery		{
														$2->left	= $1;
														$2->right	= $3;
														$3->left	= $4;
														$$ = $2;
													}
						| row_value_constructor comp_op K_ANY row_subquery		{
														$2->left	= $1;
														$2->right	= $3;
														$3->left	= $4;
														$$ = $2;
													}
						| row_value_constructor comp_op K_SOME row_subquery		{
														$2->left	= $1;
														$2->right	= $3;
														$3->left	= $4;
														$$ = $2;
													}
						;

comp_op	: K_EQ
		| K_NEQ
		| K_LT
		| K_GT
		| K_LEQ
		| K_GEQ
		;

//----------------
between_predicate	: row_value_constructor K_NOT K_BETWEEN
						row_value_constructor K_AND row_value_constructor {
														$3->left	 = $1;
														$3->tag = K_NOT_BETWEEN;
														$3->right = $5;
														$5->left	= $4;
														$5->right = $6;
														$$ = $3;
													}
					|  row_value_constructor K_BETWEEN
						row_value_constructor K_AND row_value_constructor	{
														$2->left	= $1;
														$2->right	= $4;
														$4->left	= $3;
														$4->right	= $5;
														$$			= $2;
													}
					; 

//----------------
in_predicate	: row_value_constructor K_NOT K_IN in_predicate_value	{
														$2->left	= $3;
														$3->left	= $1;
														$3->right	= $4;
														$$			= $2;
													}
				| row_value_constructor K_IN in_predicate_value	{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
				;

in_predicate_value	: K_LPAREN in_value_list K_RPAREN	{ $$ = $2; }
					| table_subquery
					;

in_value_list	: value_expression
				| value_expression K_COMMA in_value_list	{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
				;

//----------------
like_predicate	: row_value_constructor K_NOT K_LIKE pattern
					/* [ K_ESCAPE <escape character> ] */	{
														$3->left	 = $1;
														$3->tag = K_NOT_LIKE;
														$3->right = $4;
														$$ = $3;
													}
				| row_value_constructor K_NOT K_LIKE pattern K_ESCAPE K_STRING {
														$3->left	= $1;
														$3->tag = K_NOT_LIKE;
														$3->right = $5;
														$5->left	 = $4;
														$5->right = $6;
														$$ = $3;
													}
				| match_value K_NOT K_LIKE pattern
					/* [ K_ESCAPE <escape character> ] */	{
														$3->left	= $1;
														$3->tag = K_NOT_LIKE;
														$3->right = $4;
														$$ = $3;
													}
				| match_value K_NOT K_LIKE pattern K_ESCAPE K_STRING {
														$3->left	 = $1;
														$3->tag = K_NOT_LIKE;
														$3->right = $5;
														$5->left	 = $4;
														$5->right = $6;
														$$ = $3;
													}
				| match_value K_LIKE pattern
					/* [ K_ESCAPE escape_character ] */		{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
				| match_value K_LIKE pattern K_ESCAPE K_STRING {
														$2->left	= $1;
														$2->right	= $4;
														$4->left	= $3;
														$4->right	= $5;
														$$			= $2;
													}
				;

match_value	: character_value_expression
			;

pattern	: character_value_expression
		;

	/*
escape_character	:  character_value_expression
					;
	*/

//-----------------
null_predicate	: row_value_constructor K_IS K_NOT K_NULL {
														$2->left	= $1;
														$2->right	= $3;
														$3->left	= $4;
														$$			= $2;
													}
				| row_value_constructor K_IS K_NULL		{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
				; 

exists_predicate	: K_EXISTS table_subquery		{
														$1->left = $2;
														$$ = $1; 
													}
					;

row_subquery	: subquery
				;

row_value_constructor	: row_value_constructor_element
						| K_LPAREN row_value_constructor_list K_RPAREN	{
														$$ = $2;
													}
						| row_subquery
						;

row_value_constructor_list	: row_value_constructor_element
							| row_value_constructor_list K_COMMA row_value_constructor_element {
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
							;

row_value_constructor_element	: value_expression
								| K_NULL
								| K_DEFAULT
								;

//---------------------------------------------------
aggregate_function	: set_function_specification //[ <filter clause> ]
					//| binary_set_function [ <filter clause> ]
					//| ordered_set_function [ <filter clause> ]
					;
								
//---------------------------------------------------
window_function	: window_function_type K_OVER window_name_or_specification {
														delete $2;
														$1->right	= $3;
														$$			= $1;
													}
				;

window_function_type	: /*rank_function_type K_LPAREN K_RPAREN
						|*/ K_ROW_NUMBER K_LPAREN K_RPAREN
						| aggregate_function		{ 
														$$	= $1;
													}
						/*| ntile_function*/
						| lead_or_lag_function		{ 
														$$	= $1; 
													}
						| first_or_last_value_function	{ 
														$$	= $1; 
													}
						/*| nth_value_function*/
						;

/*						
rank_function_type	: K_RANK
					| K_DENSE_RANK
					| K_PERCENT_RANK
					| K_CUME_DIST
					;
*/

lead_or_lag_function	: lead_or_lag K_LPAREN lead_or_lag_extent offset_optional K_RPAREN {
														if( $3 != NULL )
														{
															aq::tnode *pNode;
															pNode			= new tnode( K_COMMA );
															pNode->left		= $3;
															pNode->right	= $4;
															$1->left		= pNode;
														}
														else
														{
															$1->left	= $3;
														}
														$$			= $1;
													}
						//[ <null treatment> ]
						;

lead_or_lag	: K_LEAD | K_LAG
			;

lead_or_lag_extent	: value_expression
					;

offset_optional	: K_COMMA offset default_expression_optional {
														if( $3 != NULL )
														{
															aq::tnode *pNode;
															pNode			= new tnode( K_COMMA );
															pNode->left		= $2;
															pNode->right	= $3;
															$$				= pNode;
														}
														else
														{
															$$				= $2;
														}
													}
				| /* nothing */						{	$$ = NULL; }
				;

offset	: K_INTEGER
		;

default_expression_optional	: K_COMMA default_expression {
														$$	= $2;
													}
							| /* nothing */			{	$$ = NULL; }
							;

default_expression	: value_expression

/*
null_treatment	: K_RESPECT_NULLS 
				| K_IGNORE_NULLS
*/
				
first_or_last_value_function	: first_or_last_value
								  K_LPAREN value_expression K_RPAREN {
														$1->left	= $3;
														$$			= $1;
													}
								  //[ <null treatment>]
								  ;

first_or_last_value	: K_FIRST_VALUE | K_LAST_VALUE
					;

//---------------------------------------------------
window_name_or_specification	: window_name		{	$$	= $1;	}
								| inline_window_specification
								;

inline_window_specification	: window_specification;

window_name	: identifier
			;

window_specification	: K_LPAREN window_specification_details K_RPAREN { $$ = $2; }
						;

window_specification_details	: /* existing_window_name */
								  window_partition_clause
								  window_order_clause
								  window_frame_clause {
														if( $2 != NULL )
														{
															if( $1 != NULL )
															{
																$1->right	= $3;
																$2->right	= $1;
															}
															else
																$2->right	= $3;
															$$				= $2;
														}
														else
														{
															if( $1 != NULL )
															{
																$1->right	= $3;
																$$			= $1;
															}
															else
																$$			= $3;
														}
													}
								;

/*
existing_window_name	: window_name
						| /* nothing */ /*			{	$$	= NULL; }
*/

window_partition_clause	: K_PARTITION K_BY window_partition_column_reference_list {
														$1->left	= $2;
														$2->left	= $3;
														$$			= $1;
													}
						| /* nothing */				{	$$	= NULL; }
						;

window_partition_column_reference_list	: window_partition_column_reference
										| window_partition_column_reference_list
										K_COMMA window_partition_column_reference {
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
										;

window_partition_column_reference	: value_expression //the SQL:2008 standard
//uses column_reference, not value_expression //[ <collate clause> ]
									;

window_order_clause	: order_by_clause
					;


window_frame_clause	: window_frame_units window_frame_extent /*[ <window frame exclusion> ]*/ {
														aq::tnode *pNode;
														pNode			= new tnode( K_FRAME );
														pNode->left		= $1;
														pNode->right	= $2;
														$$				= pNode;
													}
					| /* nothing */ 				{	$$ = NULL; }

window_frame_units	: K_ROWS
					| K_RANGE

window_frame_extent	: window_frame_start			
					| window_frame_between			

window_frame_start	: K_UNBOUNDED K_PRECEDING		{	
														$2->left	= $1;
														$$			= $2;
													}
					| window_frame_preceding		
					| K_CURRENT K_ROW				{	$$ = $1;	}

/* window_frame_preceding	: unsigned_value_specification K_PRECEDING */
window_frame_preceding	: K_INTEGER K_PRECEDING		{
														$2->left	= $1;
														$$			= $2;
													}

window_frame_between	: K_BETWEEN window_frame_bound_1 K_AND window_frame_bound_2 {
														$3->left	= $2;
														$3->right	= $4;
														$$			= $3;
													}
window_frame_bound_1	: window_frame_bound
window_frame_bound_2	: window_frame_bound
window_frame_bound		: window_frame_start
						| K_UNBOUNDED K_FOLLOWING	{
														$2->left	= $1;
														$$			= $2;
													}
						| window_frame_following
						
/*window_frame_following	: unsigned_value_specification FOLLOWING*/
window_frame_following	: K_INTEGER K_FOLLOWING		{
														$2->left	= $1;
														$$			= $2;
													}

/*
window_frame_exclusion	: EXCLUDE CURRENT ROW
						| EXCLUDE GROUP
						| EXCLUDE TIES
						| EXCLUDE NO OTHERS
*/

//---------------------------------------------------
cast_specification	: K_CAST K_LPAREN cast_operand K_AS cast_target K_RPAREN {
														$1->left	= $3;
														$1->right	= $5;
														$$			= $1;
													}
cast_operand	: value_expression
				/*| implicitly_typed_value_specification*/

cast_target	: /*domain_name
			| */data_type							
			
data_type	: K_STRING_TYPE
			| K_REAL_TYPE
			| K_INTEGER_TYPE
			
nvl_specification	: K_NVL K_LPAREN value_expression K_COMMA 
					  value_expression K_RPAREN		{
														$1->left	= $3;
														$1->right	= $5;
														$$			= $1;
													}

//---------------------------------------------------
square_root	: K_SQRT K_LPAREN numeric_value_expression K_RPAREN {
														$1->left	= $3;
														$$			= $1;
													}
			;

absolute_value_expression	: K_ABS K_LPAREN numeric_value_expression K_RPAREN {
														$1->left	= $3;
														$$			= $1;
													}
							;

year_expresssion	: K_YEAR K_LPAREN datetime_value_expression K_RPAREN {
														$1->left	= $3;
														$$			= $1;
													}
					;

month_expresssion	: K_MONTH K_LPAREN datetime_value_expression K_RPAREN {
														$1->left	= $3;
														$$			= $1;
													}
					;

day_expresssion	: K_DAY K_LPAREN datetime_value_expression K_RPAREN {
														$1->left	= $3;
														$$			= $1;
													}
				;			
				
aq_function		: K_FUNC K_LPAREN aq_arg_list K_RPAREN				{
														$1->left	= $3;
														$$			= $1;
													}
				;

aq_arg_list		: value_expression
				| value_expression K_COMMA aq_arg_list				{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
				;

//---------------------------------------------------
binary_substring_function	: K_SUBSTRING K_LPAREN binary_primary 
							  K_COMMA start_position K_RPAREN {
														$4->left		= $3;
														$4->right	= $5;
														$1->left		= $4;
														$$			= $1;
													}
							| K_SUBSTRING K_LPAREN binary_primary 
							  K_COMMA start_position K_COMMA string_length K_RPAREN {
														$4->left			= $3;
														$1->left			= $4;
														$6->left			= $5;
														$6->right		= $7;
														$1->left->left	= $3;
														$1->left->right	= $6;
														$$				= $1;
													}

start_position	: numeric_value_expression
				;

string_length	: numeric_value_expression
				;

binary_primary	: value_expression_primary
				| string_value_function
				| K_STRING
				;

identifier	: K_IDENT
			;
			
//---------------------------------------------------
create_table_stmt	: K_CREATE K_TABLE table_name K_AS select_stmt {
														$1->left	= $3;
														$1->right	= $5;
														$$			= $1;
													}
													
//---------------------------------------------------
insert_stmt	: K_INSERT K_INTO table_name K_LPAREN column_name_list K_RPAREN 
			  K_VALUES K_LPAREN value_list K_RPAREN	{
														$1->left		= $3;
														aq::tnode *pNode;
														pNode			= new tnode( K_INSERT_ARGS );
														$1->right		= pNode;
														pNode->left		= $5;
														pNode->right	= $9;
														$$				= $1;
													}
			| K_INSERT K_INTO table_name K_LPAREN column_name_list K_RPAREN 
			  select_stmt							{
														$1->left		= $3;
														aq::tnode *pNode;
														pNode			= new tnode( K_INSERT_ARGS );
														$1->right		= pNode;
														pNode->left		= $5;
														pNode->right	= $7;
														$$				= $1;
													}
			;

value_list	: value_scalar
			| value_list K_COMMA value_scalar		{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
			;
																	
value_scalar	: K_INTEGER
				| K_REAL
				| K_STRING
				| K_DATE_VALUE
				;
				
//---------------------------------------------------
update_stmt	: K_UPDATE table_name 
			  K_SET column_value_list
			  where_clause							{
														$1->left = $2;
														aq::tnode *pNode;
														pNode = new tnode( K_SET );
														$1->next	 = pNode;
														pNode->left = $4;
														pNode->next = $5;
													}
			| K_UPDATE table_name 
			  K_SET column_value_list
			  K_WHERE K_LPAREN column_name_list K_RPAREN K_IN subquery	{
														$1->left		= $2;
														aq::tnode *pNode;
														pNode			= new tnode( K_SET );
														$1->right		= pNode;
														pNode->left		= $4;
														pNode->right	= $9;
														pNode->right->left	= $7;
														pNode->right->right	= $10;
														$$				= $1;
													}
			;

column_value_list	: column_value_pair
					| column_value_list K_COMMA column_value_pair	{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
					;
																	
column_value_pair	: column_name K_EQ value_scalar	{
														$$			= $2;
														$2->left	= $1;
														$2->right	= $3;
													}
					;
					
//---------------------------------------------------
delete_stmt	: K_DELETE K_FROM table_name
			  K_WHERE column_value_list				{
														$1->left		= $3;
														$1->right		= $5;
														$$				= $1;
													}
			| K_DELETE K_FROM table_name
			  K_WHERE K_LPAREN column_name_list K_RPAREN K_IN subquery	{
														$1->left		= $3;
														$1->right		= $8;
														$1->right->left	= $6;
														$1->right->right= $9;
														$$				= $1;
													}
			;

//---------------------------------------------------
truncate_stmt	: K_TRUNCATE K_TABLE table_name		{
														$1->left		= $3;
														$$				= $1;
													}
				;

//---------------------------------------------------
decode_specification	: K_DECODE K_LPAREN value_expression 
						  K_COMMA search_result_list K_RPAREN {
														$1->left	= $3;
														$1->right	= $5;
														$$			= $1;
													}
						;

search_result_list	: search_result_list_with_default
					| search_result_list_without_default
					;

search_result_list_with_default	: search_result_list_without_default K_COMMA default_result {
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
								;

search_result_list_without_default	: search_result_pair
									| search_result_list_without_default K_COMMA search_result_pair	{
																		$2->left	= $1;
																		$2->right	= $3;
																		$$			= $2;
																	}
									;

search_result_pair	: value_scalar K_COMMA value_scalar {
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
					;

default_result	: value_scalar
				;

//---------------------------------------------------	
union_minus_stmt	: union_minus_list

union_minus_list	: query_exp K_UNION query_exp	{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
					| query_exp K_SEL_MINUS query_exp	{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
					| query_exp K_UNION K_ALL query_exp		{
														$2->left	= $1;
														$2->right	= $3;
														$3->left	= $4;
														$$			= $2;
													}
					| union_minus_list K_UNION query_exp	{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
					| union_minus_list K_SEL_MINUS query_exp	{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
					| union_minus_list K_UNION K_ALL query_exp	{
														$2->left	= $1;
														$2->right	= $3;
														$3->left	= $4;
														$$			= $2;
													}
			;
			
//---------------------------------------------------	
merge_stmt	: K_MERGE optional_into table_reference K_USING table_reference
			  K_ON search_condition
			  when_matched when_not_matched when_not_matched_by_source	{
														aq::tnode *pNode1;
														pNode1			= new tnode( K_MERGE_ARGS1 );
														$1->left		= pNode1;
														pNode1->left	= $3;
														pNode1->right	= $5;
														$1->right		= $7;
														aq::tnode *pNode2;
														pNode2			= new tnode( K_MERGE_ARGS2 );
														$1->next		= pNode2;
														pNode2->left	= $8;
														pNode2->right	= $9;
														pNode2->next	= $10;
														$$				= $1;
													}
			;
			
optional_into	: K_INTO
				| /* nothing */ 					{	$$	= NULL; }
				;

when_matched	: K_WHEN K_MATCHED K_THEN merge_matched	{
														$$	= $4;
													}
				| /* nothing */ 					{	$$	= NULL; }
				;

when_not_matched	: K_WHEN K_NOT K_MATCHED optional_by_target
					  K_THEN merge_not_matched		{
														$$	= $5;
													}
					| /* nothing */ 				{	$$	= NULL; }
					;
					
optional_by_target	: K_BY K_TARGET					{	$$	= NULL; }
					| /* nothing */ 				{	$$	= NULL; }
					;
					
when_not_matched_by_source	: K_WHEN K_NOT K_MATCHED 
							  K_BY K_SOURCE K_THEN merge_matched	{
														$$	= $7;
													}
							| /* nothing */ 		{	$$	= NULL; }
							;
							
merge_matched	: K_UPDATE K_SET column_column_list	{
														$1->left		= $3;
														$$				= $1;
													}
				| K_DELETE
				;
				
merge_not_matched	: K_INSERT K_LPAREN column_name_list K_RPAREN 
					  K_VALUES K_LPAREN column_name_list K_RPAREN	{
														$1->left		= $3;
														$1->right		= $7;
														$$				= $1;
													}
					;
					
column_column_list	: column_column_pair
					| column_column_list K_COMMA column_column_pair	{
														$2->left	= $1;
														$2->right	= $3;
														$$			= $2;
													}
					;
																	
column_column_pair	: column_reference K_EQ column_reference	{
														$$			= $2;
														$2->left	= $1;
														$2->right	= $3;
													}
					;

%%

//------------------------------------------------------------------------------
#include "lex.yy.cpp"

//------------------------------------------------------------------------------
/* Returns 0 on success, 1 on error */
int SQLParse( const char *pszStr, aq::tnode** ppNode ) {
	int rc = 0;
	yy_scan_string( pszStr );
	rc = yyparse( (void*)ppNode );

  // FIXME
#ifndef __FreeBSD__ 
  yylex_destroy();
#endif

	return rc;
}

//------------------------------------------------------------------------------
int yywrap( void ) {
	// return 0;		// To allow multi file ( ^Z - EOF )
	return 1;		// To allow only one file ( EOF -> Exit )
}

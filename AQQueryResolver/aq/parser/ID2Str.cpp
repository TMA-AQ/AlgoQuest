#include "ID2Str.h"
#include <aq/Exceptions.h>
#include <cstdio>
#include <sstream>
#include <cassert>

namespace aq
{

struct TID2String 
{
	unsigned int  m_nID;
	const char   *pszStr;
};

//------------------------------------------------------------------------------
TID2String g_id2str[] = {
	{ K_ALL,			"ALL"			},
	{ K_AND,			"AND"			},
	{ K_ANY,			"ANY"			},
	{ K_AS,				"AS"			},
	{ K_ASC,			"ASC"			},
	{ K_AVG,			"AVG"			},
	{ K_BETWEEN,		"BETWEEN"		},
  { K_BY,				"" }, // "BY"			},
	{ K_CASE,			"CASE"			},
	{ K_COMMIT,			"COMMIT"		},
	{ K_COUNT,			"COUNT"			},
	{ K_DATE,			"DATE"			},
	{ K_DAY,			"DAY"			},
	{ K_DEFAULT,		"DEFAULT"		},
	{ K_DEFERRED,		"DEFERRED"		},
	{ K_DELETE,			"DELETE"		},
	{ K_DELETED,			"DELETED"		},
	{ K_DESC,			"DESC"			},
	{ K_DISTINCT,		"DISTINCT"		},
	{ K_ELSE,			"ELSE"			},
	{ K_END,			"END"			},
	{ K_EXISTS,			"EXISTS"		},
	{ K_EXTRACT,		"EXTRACT"		},
	{ K_IMMEDIATE,		"IMMEDIATE"		},
	{ K_FOR,			"FOR"			},
	{ K_FROM,			"FROM"			},
	{ K_FULL,			"FULL"			},
	{ K_GROUP,			"GROUP"			},
	{ K_HAVING,			"HAVING"		},
	{ K_IN,				"IN"			},
	{ K_INNER,			"K_INNER"		},
	{ K_INSERT,			"INSERT"		},
	{ K_INTERVAL,		"INTERVAL"		},
	{ K_INTO,			"INTO"			},
	{ K_IS,				"IS"			},
	{ K_JOIN,			"JOIN"			},
	{ K_LEFT,			"LEFT"			},
	{ K_LIKE,			"LIKE"			},
	{ K_MAX,			"MAX"			},
	{ K_MIN,			"MIN"			},
	{ K_MONTH,			"MONTH"			},
	{ K_NATURAL,		"NATURAL"		},
	{ K_OR,				"OR"			},
	{ K_NULL,			"NULL"			},
	{ K_ON,				"ON"			},
	{ K_ORDER,			"ORDER"			},
	{ K_OUTER,			"K_OUTER"		},
	{ K_RIGHT,			"RIGHT"			},
	{ K_ROLLBACK,		"ROLLBACK"		},
	{ K_SELECT,			"SELECT"		},
	{ K_SET,			"SET"			},
	{ K_SOME,			"SOME"			},
	{ K_SUBSTRING,		"SUBSTRING"		},
	{ K_SUM,			"SUM"			},
	{ K_TABLE,			"TABLE"			},
	{ K_THEN,			"THEN"			},
	{ K_TRANSACTION,	"TRANSACTION"	},
	{ K_UNION,			"UNION"			},
	{ K_UPDATE,			"UPDATE"		},
	{ K_VALUES,			"VALUES"		},
	{ K_WHEN,			"WHEN"			},
	{ K_WHERE,			"WHERE"			},
	{ K_WORK,			"WORK"			},
	{ K_YEAR,			"YEAR"			},
	{ K_PERIOD,			"."				},
	{ K_PLUS,			"+"				},
	{ K_MINUS,			"-"				},
	{ K_MUL,			"*"				},
	{ K_DIV,			"/"				},
	{ K_EQ,				"="				},
	{ K_LT,				"<"		},
	{ K_GT,				">"		},
	{ K_COMMA,			","				},
	{ K_SEMICOLON,		";"				},
	{ K_NEQ,			"K_NEQ"/*"<>"*/	},
	{ K_LEQ,			"<="/*"<="*/	},
	{ K_GEQ,			">="/*">="*/	},
	{ K_CONCAT,			"K_CONCAT"/*"||"*/	},
	{ K_NOT,			"NOT"			},
	{ K_UMINUS,			"-"				},
	{ K_INTEGER,		""				},
	{ K_IDENT,			""				},
	{ K_REAL,			""				},
	{ K_STRING,			""				},
	{ K_CALL  ,			"CALL"			},
	{ K_COLUMNS,		"COLUMNS"		},
	{ K_JEQ,			"K_JEQ"			},
	{ K_JAUTO,			"K_JAUTO"			},
/*
	{ K_JTHETA,			"JTHETA"		},
*/
	{ K_LIST,			"LIST"			},
	{ K_OUTREF,			"OUTREF"		},
	{ K_SOURCE,			"SOURCE"		},
	{ K_STAR,			"K_STAR"		},
	{ K_START,			"START"			},
	{ K_TO,				"TO"			},
/*
	{ K_PARPAIR,		"PARPAIR"		},
	{ K_BLOCK ,			"BLOCK"			},
	{ K_NULL_POINTER,	"NULL_POINTER"	},
*/
	{ K_TO_DATE,		"TO_DATE"		},
	{ K_TRUE,			"K_TRUE"		},
	{ K_FALSE,			"K_FALSE"		},

	{ K_COLUMN,			"COLUMN"		},
	{ K_LPAREN,			"("				},
	{ K_RPAREN,			")"				},
	{ K_LBRACE,			"{"				},
	{ K_RBRACE,			"}"				},
	{ K_LBRACKETS,		"["				},
	{ K_RBRACKETS,		"]"				},
	{ K_REPLACE,		"REPLACE"		},
	{ K_FIRST_VALUE,	"FIRST_VALUE"	},
	{ K_LEAD,			"LEAD"			},
	{ K_LAG,			"LAG"			},
	{ K_PARTITION,		"PARTITION"		},
	{ K_SQRT,			"SQRT"			},
	{ K_ABS,			"ABS"			},
	{ K_CURRENT,		"CURRENT"		},
	{ K_FOLLOWING,		"FOLLOWING"		},
	{ K_FRAME,			"FRAME"			},
	{ K_PRECEDING,		"PRECEDING"		},
	{ K_RANGE,			"RANGE"			},
	{ K_ROWS,			"ROWS"			},
	{ K_UNBOUNDED,		"UNBOUNDED"		},
	{ K_STRING_TYPE,	"K_STRING_TYPE"	},
	{ K_REAL_TYPE,		"K_REAL_TYPE"	},
	{ K_INTEGER_TYPE,	"K_INTEGER_TYPE"},
	{ K_CAST,			"CAST"			},
	{ K_CREATE,			"CREATE"		},
	{ K_INSERT_ARGS,	"INSERT_ARGS"	},
	{ K_CURRENT_DATE,	"CURRENT_DATE"	},
	{ K_NVL,			"NVL"			},
	{ K_TO_CHAR,		"TO_CHAR"		},
	{ K_ROW_NUMBER,		"ROW_NUMBER"	},
	{ K_TRUNCATE,		"TRUNCATE"		},
	{ K_DECODE,			"DECODE"		},
	{ K_JNO,			"K_JNO"			},
	{ K_JINF,			"K_JINF"		},
	{ K_JIEQ,			"K_JIEQ"		},
	{ K_JSUP,			"K_JSUP"		},
	{ K_JSEQ,			"K_JSEQ"		},
	{ K_JNEQ,			"K_JNEQ"		},
	{ K_SEL_MINUS,		"K_MINUS"		},
	{ K_MERGE,			"K_MERGE"		},
	{ K_MATCHED,		"K_MATCHED"		},
	{ K_USING,			"K_USING"		},
	{ K_TARGET,			"K_TARGET"		}//,
  // { K_IN_VALUES,  "K_IN" }
};

//------------------------------------------------------------------------------
TID2String g_id2kstr[] = {
  { K_ABS, "K_ABS" },
  { K_ALL, "K_ALL" },
  { K_AND, "K_AND" },
  { K_ANY, "K_ANY" },
  { K_AS, "K_AS" },
  { K_ASC, "K_ASC" },
  { K_AVG, "K_AVG" },
  { K_BETWEEN, "K_BETWEEN" },
  { K_CALL  , "K_CALL  " },
  { K_CASE, "K_CASE" },
  { K_CAST, "K_CAST" },
  { K_COLUMN, "K_COLUMN" },
  { K_COLUMNS, "K_COLUMNS" },
  { K_COMMA, "K_COMMA" },
  { K_COMMIT, "K_COMMIT" },
  { K_CONCAT, "K_CONCAT" },
  { K_COUNT, "K_COUNT" },
  { K_CREATE, "K_CREATE" },
  { K_CURRENT, "K_CURRENT" },
  { K_CURRENT_DATE, "K_CURRENT_DATE" },
  { K_DATE, "K_DATE" },
  { K_DAY, "K_DAY" },
  { K_DECODE, "K_DECODE" },
  { K_DEFAULT, "K_DEFAULT" },
  { K_DEFERRED, "K_DEFERRED" },
  { K_DELETE, "K_DELETE" },
  { K_DELETED, "K_DELETED" },
  { K_DESC, "K_DESC" },
  { K_DISTINCT, "K_DISTINCT" },
  { K_DIV, "K_DIV" },
  { K_ELSE, "K_ELSE" },
  { K_END, "K_END" },
  { K_EQ, "K_EQ" },
  { K_EXISTS, "K_EXISTS" },
  { K_EXTRACT, "K_EXTRACT" },
  { K_FALSE, "K_FALSE" },
  { K_FIRST_VALUE, "K_FIRST_VALUE" },
  { K_FOLLOWING, "K_FOLLOWING" },
  { K_FOR, "K_FOR" },
  { K_FRAME, "K_FRAME" },
  { K_FROM, "K_FROM" },
  { K_FULL, "K_FULL" },
  { K_GEQ, "K_GEQ" },
  { K_GROUP, "K_GROUP" },
  { K_GT, "K_GT" },
  { K_HAVING, "K_HAVING" },
  { K_IDENT, "K_IDENT" },
  { K_IMMEDIATE, "K_IMMEDIATE" },
  { K_IN, "K_IN" },
  { K_IN_VALUES, "K_IN_VALUES" },
  { K_INNER, "K_INNER" },
  { K_INSERT, "K_INSERT" },
  { K_INSERT_ARGS, "K_INSERT_ARGS" },
  { K_INTEGER, "K_INTEGER" },
  { K_INTEGER_TYPE, "K_INTEGER_TYPE" },
  { K_INTERVAL, "K_INTERVAL" },
  { K_INTO, "K_INTO" },
  { K_IS, "K_IS" },
  { K_JAUTO, "K_JAUTO" },
  { K_JEQ, "K_JEQ" },
  { K_JIEQ, "K_JIEQ" },
  { K_JINF, "K_JINF" },
  { K_JNEQ, "K_JNEQ" },
  { K_JNO, "K_JNO" },
  { K_JOIN, "K_JOIN" },
  { K_JSEQ, "K_JSEQ" },
  { K_JSUP, "K_JSUP" },
  { K_LAG, "K_LAG" },
  { K_LBRACE, "K_LBRACE" },
  { K_LBRACKETS, "K_LBRACKETS" },
  { K_LEAD, "K_LEAD" },
  { K_LEFT, "K_LEFT" },
  { K_LEQ, "K_LEQ" },
  { K_LIKE, "K_LIKE" },
  { K_LIST, "K_LIST" },
  { K_LPAREN, "K_LPAREN" },
  { K_LT, "K_LT" },
  { K_MATCHED, "K_MATCHED" },
  { K_MAX, "K_MAX" },
  { K_MERGE, "K_MERGE" },
  { K_MIN, "K_MIN" },
  { K_MINUS, "K_MINUS" },
  { K_MONTH, "K_MONTH" },
  { K_MUL, "K_MUL" },
  { K_NATURAL, "K_NATURAL" },
  { K_NEQ, "K_NEQ" },
  { K_NOT, "K_NOT" },
  { K_NULL, "K_NULL" },
  { K_NVL, "K_NVL" },
  { K_ON, "K_ON" },
  { K_OR, "K_OR" },
  { K_ORDER, "K_ORDER" },
  { K_OUTER, "K_OUTER" },
  { K_OUTREF, "K_OUTREF" },
  { K_PARTITION, "K_PARTITION" },
  { K_PERIOD, "K_PERIOD" },
  { K_PLUS, "K_PLUS" },
  { K_PRECEDING, "K_PRECEDING" },
  { K_RANGE, "K_RANGE" },
  { K_RBRACE, "K_RBRACE" },
  { K_RBRACKETS, "K_RBRACKETS" },
  { K_REAL, "K_REAL" },
  { K_REAL_TYPE, "K_REAL_TYPE" },
  { K_REPLACE, "K_REPLACE" },
  { K_RIGHT, "K_RIGHT" },
  { K_ROLLBACK, "K_ROLLBACK" },
  { K_ROWS, "K_ROWS" },
  { K_ROW_NUMBER, "K_ROW_NUMBER" },
  { K_RPAREN, "K_RPAREN" },
  { K_SELECT, "K_SELECT" },
  { K_SEL_MINUS, "K_SEL_MINUS" },
  { K_SEMICOLON, "K_SEMICOLON" },
  { K_SET, "K_SET" },
  { K_SOME, "K_SOME" },
  { K_SOURCE, "K_SOURCE" },
  { K_SQRT, "K_SQRT" },
  { K_STAR, "K_STAR" },
  { K_START, "K_START" },
  { K_STRING, "K_STRING" },
  { K_STRING_TYPE, "K_STRING_TYPE" },
  { K_SUBSTRING, "K_SUBSTRING" },
  { K_SUM, "K_SUM" },
  { K_TABLE, "K_TABLE" },
  { K_TARGET, "K_TARGET" },
  { K_THEN, "K_THEN" },
  { K_TO, "K_TO" },
  { K_TO_CHAR, "K_TO_CHAR" },
  { K_TO_DATE, "K_TO_DATE" },
  { K_TRANSACTION, "K_TRANSACTION" },
  { K_TRUE, "K_TRUE" },
  { K_TRUNCATE, "K_TRUNCATE" },
  { K_UMINUS, "K_UMINUS" },
  { K_UNBOUNDED, "K_UNBOUNDED" },
  { K_UNION, "K_UNION" },
  { K_UPDATE, "K_UPDATE" },
  { K_USING, "K_USING" },
  { K_VALUES, "K_VALUES" },
  { K_WHEN, "K_WHEN" },
  { K_WHERE, "K_WHERE" },
  { K_WORK, "K_WORK" },
  { K_YEAR, "K_YEAR" },
  { K_BY, "K_BY" }
};

#define SIZE_OF( tab ) ( sizeof( tab ) / sizeof( tab[ 0 ] ) )

//------------------------------------------------------------------------------
const char* id_to_string( unsigned int nID ) {
	unsigned int i;
	for ( i = 0; i < SIZE_OF( g_id2str ); i++ ) {
		if ( g_id2str[ i ].m_nID == nID ) {
			return g_id2str[ i ].pszStr;
		}
	}
  //throw aq::generic_error(aq::generic_error::VERB_TYPE_MISMATCH, "cannot find type %u", nID);
	//return "K_UNKNOW";
  return NULL;
}

//------------------------------------------------------------------------------
const char* id_to_kstring( unsigned int nID ) {
	unsigned int i;
	for ( i = 0; i < SIZE_OF( g_id2kstr ); i++ ) {
		if ( g_id2kstr[ i ].m_nID == nID ) {
			return g_id2kstr[ i ].pszStr;
		}
	}
  assert(false);
  return NULL;
}

//------------------------------------------------------------------------------
const char* id_to_sql_string( unsigned int nID )
{

	switch (nID)
	{
	case K_JEQ: nID = K_EQ ; break;
	case K_JAUTO: nID = K_EQ ; break;
	case K_JNO: nID = K_LT ; break;
	case K_JINF: nID = K_LT ; break;
	case K_JIEQ: nID = K_LEQ ; break;
	case K_JSUP: nID = K_GT ; break;
	case K_JSEQ: nID = K_GEQ ; break;
	}

	unsigned int i;
	for ( i = 0; i < SIZE_OF( g_id2str ); i++ ) {
		if ( g_id2str[ i ].m_nID == nID ) {
			return g_id2str[ i ].pszStr;
		}
	}

	return "K_UNKNOW";
}

}

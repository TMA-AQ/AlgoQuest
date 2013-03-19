#include "ID2Str.h"
#include <stdio.h>

//------------------------------------------------------------------------------
TID2String g_id2str[] = {
	{ K_ALL,			"ALL"			},
	{ K_AND,			"AND"			},
	{ K_ANY,			"ANY"			},
	{ K_AS,				"AS"			},
	{ K_ASC,			"ASC"			},
	{ K_AVG,			"AVG"			},
	{ K_BETWEEN,		"BETWEEN"		},
	{ K_BY,				"BY"			},
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
	{ K_TARGET,			"K_TARGET"		}
};

#define SIZE_OF( tab ) ( sizeof( tab ) / sizeof( tab[ 0 ] ) )

//------------------------------------------------------------------------------
char* id_to_string( unsigned int nID ) {
	unsigned int i;
	for ( i = 0; i < SIZE_OF( g_id2str ); i++ ) {
		if ( g_id2str[ i ].m_nID == nID ) {
			return g_id2str[ i ].pszStr;
		}
	}
	return NULL;
}


char* id_to_sql_string( unsigned int nID )
{

	switch (nID)
	{
	case K_JEQ: nID = K_EQ ; break;
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
	return NULL;
}
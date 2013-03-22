#ifndef __GBZ_LIKE_PATTERNMATCHING_H__
#define __GBZ_LIKE_PATTERNMATCHING_H__

//------------------------------------------------------------------------------
typedef enum tagMatchOperation {
	OP_MATCH_PATTERN_END = 0,	/* No more Match Operation. Matches spaces and the terminating NULL character ! */
	OP_MATCH_EXACT,				/* SubString match (with given length, case sensitive ! ) */
	OP_MATCH_ANY_CHAR,			/* Pattern Char : '_'. */
	OP_MATCH_ANY_SUBSTRING,		/* Pattern Char : '%'. Zero or more ANY characters */
	OP_MATCH_SET				/* Pattern format : '['...']'. Inlcuding char SET and RANGE, even NEGATED */
} TMatchOperation;

//------------------------------------------------------------------------------
typedef struct tagMatchOp {
	TMatchOperation		m_eMatchOp;
	unsigned int		m_nRequiredMinStrLen;		/* Required Min size of the rest of string to match ! */
	union TOpParams {
		struct TMatchExactData {
			unsigned int	m_nExactStrLen;
			char			*m_pszExactStr;
		} m_MatchExactData;
		unsigned char m_arrMatchSetData[ 256 ];		/* ASCII char table ! */
	} m_Params;
} TMatchOp;

//------------------------------------------------------------------------------
typedef struct tagMatchOpArray {
	TMatchOp*		m_arrMatchOp;
	unsigned int	m_nrOperations;
} TMatchOpArray;

//------------------------------------------------------------------------------
typedef TMatchOpArray*	TPatternDescription;

//------------------------------------------------------------------------------
#define NO_ESCAPE_CHAR		256

//------------------------------------------------------------------------------
/* Returns -1 on error, 0 on success */
int PatternMatchingCreate( char* pszPattern, int cEscape, TPatternDescription *pPatternDesc );

/* Returns -1 on error, 0 not matched, 1 on match */
int MatchPattern( const char* pszToMatch, TPatternDescription patternDesc );

/* Returns -1 on error, 0 on success */
void PatternMatchingDestroy( TPatternDescription *pPatternDesc );

#endif /* __GBZ_LIKE_PATTERNMATCHING_H__ */
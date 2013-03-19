#include "LIKE_PatternMatching.h"
#include <stdio.h>
#include <malloc.h>
#include <string.h>

//------------------------------------------------------------------------------
TPatternDescription create_pattern_description( void ) {
	TPatternDescription patternDesc;

	patternDesc = (TPatternDescription)malloc( sizeof( TMatchOpArray ) );
	if ( patternDesc != NULL )
		memset( patternDesc, 0, sizeof( TMatchOpArray ) );
	return patternDesc;
}

//------------------------------------------------------------------------------
void delete_pattern_description( TPatternDescription *pPatternDesc ) {
	if ( pPatternDesc == NULL )
		return;
	if ( *pPatternDesc == NULL )
		return;
	if ( (*pPatternDesc)->m_arrMatchOp != NULL ) {
		TMatchOp *pMOp;
		unsigned int i;

		pMOp = (*pPatternDesc)->m_arrMatchOp;

		/* Free the allocated substrings for OP_MATCH_EXACT */
		for ( i = 0; i < (*pPatternDesc)->m_nrOperations; i++ ) {
			if ( pMOp[ i ].m_eMatchOp == OP_MATCH_EXACT ) {
				if ( pMOp[ i ].m_Params.m_MatchExactData.m_pszExactStr != NULL )
					free( pMOp[ i ].m_Params.m_MatchExactData.m_pszExactStr );
			}
		}

		free( (*pPatternDesc)->m_arrMatchOp );
	}
	free( *pPatternDesc );
	*pPatternDesc = NULL;
}

//------------------------------------------------------------------------------
/* Return -1 on error, 0 on success ! */
int realloc_match_op_array( TPatternDescription patternDesc, unsigned int nrNewElements ) {
	TMatchOp* parrMOpNew;

	if ( nrNewElements == 0 ) {
		/* Free the array */
		if ( patternDesc != NULL ) {
			if ( patternDesc->m_arrMatchOp != NULL ) {
				free( patternDesc->m_arrMatchOp );
				patternDesc->m_arrMatchOp = NULL;
				patternDesc->m_nrOperations = 0;
			}
		}
		return 0;
	}

	if ( patternDesc->m_nrOperations > nrNewElements ) {
		/* No need to resize ! */

		unsigned int i;

		/* Free the allocated substrings for OP_MATCH_EXACT for the discarded Operations */
		for ( i = nrNewElements; i < patternDesc->m_nrOperations; i++ ) {
			if ( patternDesc->m_arrMatchOp[ i ].m_eMatchOp == OP_MATCH_EXACT ) {
				if ( patternDesc->m_arrMatchOp[ i ].m_Params.m_MatchExactData.m_pszExactStr != NULL )
					free( patternDesc->m_arrMatchOp[ i ].m_Params.m_MatchExactData.m_pszExactStr );
			}
		}

		patternDesc->m_nrOperations = nrNewElements;
		return 0;
	}

	/* Allocate new Operations Array */
	parrMOpNew = (TMatchOp*)malloc( nrNewElements * sizeof( TMatchOp ) );
	if ( parrMOpNew == NULL )
		return -1;

	/* Because OP_MATCH_PATTERN_END value is 0, we can use only "memset" here ! */
	memset( parrMOpNew, 0, nrNewElements * sizeof( TMatchOp ) );
	if ( patternDesc->m_arrMatchOp != NULL ) {
		/* Copy old elements */
		memcpy( parrMOpNew, patternDesc->m_arrMatchOp, patternDesc->m_nrOperations * sizeof( TMatchOp ) );
		free( patternDesc->m_arrMatchOp );
	}

	patternDesc->m_arrMatchOp	= parrMOpNew;
	patternDesc->m_nrOperations	= nrNewElements;

	return 0;
}

//------------------------------------------------------------------------------
TMatchOp* add_an_operation_destroy_on_error( TPatternDescription *pPatternDesc, TMatchOperation eOp ) {
	TMatchOp* pMOp;

	if ( pPatternDesc == NULL )
		return NULL;
	if ( realloc_match_op_array( *pPatternDesc, (*pPatternDesc)->m_nrOperations + 1 ) == -1 ) {
		delete_pattern_description( pPatternDesc );
		return NULL;
	}

	pMOp = (*pPatternDesc)->m_arrMatchOp + (*pPatternDesc)->m_nrOperations - 1;
	pMOp->m_eMatchOp = eOp;
	return pMOp;
}

//------------------------------------------------------------------------------
/* Returns -1 on error, 0 on success */
int add_char_to_match_exact_substring( TMatchOp	*pMatchOp, char cVal ) {
	char *pszNewStr;

	pszNewStr = (char*)malloc( pMatchOp->m_Params.m_MatchExactData.m_nExactStrLen + 1 );
	if ( pszNewStr == NULL )
		return -1;
	
	/* This substring is not a null terminated string ! Do not use strcpy ! */
	if ( pMatchOp->m_Params.m_MatchExactData.m_nExactStrLen > 0 ) {
		memcpy( pszNewStr, pMatchOp->m_Params.m_MatchExactData.m_pszExactStr, pMatchOp->m_Params.m_MatchExactData.m_nExactStrLen );
		/* Free the old substring */
		free( pMatchOp->m_Params.m_MatchExactData.m_pszExactStr );
	}

	/* Set the new character */
	pszNewStr[ pMatchOp->m_Params.m_MatchExactData.m_nExactStrLen ] = cVal;

	/* Update m_MatchExactData */
	pMatchOp->m_Params.m_MatchExactData.m_pszExactStr = pszNewStr;
	pMatchOp->m_Params.m_MatchExactData.m_nExactStrLen++;

	return 0;
}

//------------------------------------------------------------------------------
/* Returns -1 on error, 0 on success */
int PatternMatchingCreate( char* pszPattern, int cEscape, TPatternDescription *pPatternDesc ) {
	char		*pszPtr;
	TMatchOp	*pCurrentMatchOp;
	int			bAllocNewOp;

	/* Translate the Pattern String in operation driven states */ 

	*pPatternDesc = NULL;
	if ( pszPattern == NULL )
		return -1;
	if ( pszPattern[ 0 ] == '\0' )
		return -1;

	*pPatternDesc = create_pattern_description();
	if ( *pPatternDesc == NULL )
		return -1;

	pszPtr			= pszPattern;
	pCurrentMatchOp = NULL;
	do {
		if ( *pszPtr == '\0' ) {
			/* 
			 * End of Pattern 
			 */
			if ( pCurrentMatchOp == NULL ) {
				delete_pattern_description( pPatternDesc );
				return -1;
			}
			pCurrentMatchOp = add_an_operation_destroy_on_error( pPatternDesc, OP_MATCH_PATTERN_END );
			if ( pCurrentMatchOp == NULL )
				return -1;	/* Memory is freed in "add_an_operation_destroy_on_error" */

			return 0;	/* Done */
		} else if ( *pszPtr != cEscape && *pszPtr == '%' ) {
			/* 
			 * Match 0 or more characters -> OP_MATCH_ANY_SUBSTRING 
			 */
			
			bAllocNewOp = 0;
			if ( pCurrentMatchOp == NULL )
				bAllocNewOp = 1;
			else if ( pCurrentMatchOp->m_eMatchOp != OP_MATCH_ANY_SUBSTRING )	/* Don't duplicate op. */
				bAllocNewOp = 1;

			if ( bAllocNewOp == 1 ) {
				/* Add new Operation */
				pCurrentMatchOp = add_an_operation_destroy_on_error( pPatternDesc, OP_MATCH_ANY_SUBSTRING );
				if ( pCurrentMatchOp == NULL )
					return -1;	/* Memory is freed in "add_an_operation_destroy_on_error" */
			}

			/* Nothing else to do for this operation */

		} else if ( *pszPtr != cEscape && *pszPtr == '_' ) {
			/* 
			 * Match ANY single character -> OP_MATCH_ANY_CHAR 
			 */

			/* Add new Operation */
			pCurrentMatchOp = add_an_operation_destroy_on_error( pPatternDesc, OP_MATCH_ANY_CHAR );
			if ( pCurrentMatchOp == NULL )
				return -1;	/* Memory is freed in "add_an_operation_destroy_on_error" */

			/* Nothing else to do for this operation */

		} else if ( *pszPtr != cEscape && *pszPtr == '[' ) {
			/* 
			 * Match one character from SET ( [Negated] Range and/or Set ) -> OP_MATCH_SET 
			 */

			int		bNegatedSet = 0;
			char	cPrevChar;
			int		bAllowRange;
			unsigned int i;
			
			pszPtr++;	/* Skip '[' */
			if ( *pszPtr == '\0' ) {
				delete_pattern_description( pPatternDesc );
				return -1;
			}
			
			/* Add new Operation */
			pCurrentMatchOp = add_an_operation_destroy_on_error( pPatternDesc, OP_MATCH_SET );
			if ( pCurrentMatchOp == NULL )
				return -1;	/* Memory is freed in "add_an_operation_destroy_on_error" */


			/* Check if Negated Set */
			if ( *pszPtr == '^' ) {
				bNegatedSet = 1;
				pszPtr++;	/* Skip '^' */
				if ( *pszPtr == '\0' || *pszPtr == ']' ) {	/* [^] not supported ! */
					delete_pattern_description( pPatternDesc );
					return -1;
				}
			}

			/* Check if first char is '-' (Range specifier) */
			if ( *pszPtr == '-' ) {
				pCurrentMatchOp->m_Params.m_arrMatchSetData[ '-' ] = 1;
				pszPtr++;	/* Skip '-' */
				if ( *pszPtr == '\0' || *pszPtr == '-' ) {	/* [--...] not allowed */
					delete_pattern_description( pPatternDesc );
					return -1;
				}
			}

			/* NOTE : Inside [] only the ']' can be escaped ! */
			/* => for Escape the ']' character is NOT permitted ! */
			if ( cEscape == ']' ) {
				if ( *pszPtr == '\0' ) {
					delete_pattern_description( pPatternDesc );
					return -1;
				}
			}

			bAllowRange	= 0;
			cPrevChar	= *pszPtr;
			while ( *pszPtr != '\0' && *pszPtr != ']' ) {
				if ( *pszPtr != '-' ) {
					pCurrentMatchOp->m_Params.m_arrMatchSetData[ *pszPtr ] = 1;
					bAllowRange = 1;
				} else {
					/* '-' detected -> Range Specification */

					if ( bAllowRange == 0 ) {
						/* ...--... or ...a-b-c... not allowed ! */
						delete_pattern_description( pPatternDesc );
						return -1;
					}

					if ( cPrevChar == '-' ) {	/* -- not supported ! */
						delete_pattern_description( pPatternDesc );
						return -1;
					}

					pszPtr++;	/* Skip '-' */
					if ( *pszPtr == '\0' || *pszPtr == ']' ) { /* ...-] not supported ! */
						delete_pattern_description( pPatternDesc );
						return -1;
					}

					for ( ; cPrevChar <= *pszPtr; cPrevChar++ )
						pCurrentMatchOp->m_Params.m_arrMatchSetData[ cPrevChar ] = 1;

					bAllowRange = 0;	/* Expect non '-' char */
				}
				cPrevChar = *pszPtr;
				pszPtr++;	/* Next char */
			}

			if ( *pszPtr == '\0' ) { /* Missing terminating ']' */
				delete_pattern_description( pPatternDesc );
				return -1;
			}

			if ( bNegatedSet == 1 ) {
				/* Need to negate ( invert ) the SET */
				for ( i = 1; i < 256; i++ ) {	/* Start from 1, because '\0' is not a valid char for [] ! */
					if ( pCurrentMatchOp->m_Params.m_arrMatchSetData[ i ] == 0 )
						pCurrentMatchOp->m_Params.m_arrMatchSetData[ i ] = 1;
					else
						pCurrentMatchOp->m_Params.m_arrMatchSetData[ i ] = 0;
				}
			}

			/* Nothing else to do for OP_MATCH_SET operation */

		} else { 
			/* 
			 * Any character or Escaped character 
			 */

			/* Check for the Escape character */
			if ( *pszPtr == cEscape ) {
				pszPtr++;	/* Skip Escape character -> next character */
				if ( *pszPtr == '\0' ) {
					delete_pattern_description( pPatternDesc );
					return -1;
				}
			}

			bAllocNewOp = 0;
			if ( pCurrentMatchOp == NULL )
				bAllocNewOp = 1;
			else if ( pCurrentMatchOp->m_eMatchOp != OP_MATCH_EXACT )
				bAllocNewOp = 1;

			if ( bAllocNewOp == 1 ) {
				/* Add new Operation */
				pCurrentMatchOp = add_an_operation_destroy_on_error( pPatternDesc, OP_MATCH_EXACT );
				if ( pCurrentMatchOp == NULL )
					return -1;	/* Memory is freed in "add_an_operation_destroy_on_error" */
			}

			/* Add the character to the "need to match" substring */
			if ( add_char_to_match_exact_substring( pCurrentMatchOp, *pszPtr ) == -1 ) {
				delete_pattern_description( pPatternDesc );
				return -1;
			}
		}

		pszPtr++;
	} while ( 1 );

	return 0;
}

//------------------------------------------------------------------------------
/* Returns -1 on error, 0 not matched, 1 on match */
int MatchSubStringWithPattern( const char* pszToMatch, unsigned int nToMatchStrLen, TMatchOp *pMatchOp ) {
	unsigned int i;
	int nRet;

	if ( pMatchOp == NULL )
		return -1;
	
	do { 
		switch ( pMatchOp->m_eMatchOp ) {
			case OP_MATCH_PATTERN_END:
				/* Check if end of the string or if ends with spaces */
				while ( *pszToMatch != '\0' && *pszToMatch == ' ' )
					pszToMatch++;
				if ( *pszToMatch == '\0' )
					return 1; /* MATCH ! */
				return 0; /* NO Match ! */
				/* break; */
			case OP_MATCH_EXACT:
				/* If '\0' is reached the != will be false and for exits with the return 0 ! */
				for ( i = 0; i < pMatchOp->m_Params.m_MatchExactData.m_nExactStrLen; i++ ) {
					if ( *pszToMatch != pMatchOp->m_Params.m_MatchExactData.m_pszExactStr[ i ] )
						return 0; /* NO Match ! */
					pszToMatch++;
				}
				break;
			case OP_MATCH_ANY_CHAR:
				pszToMatch++;
				break;
			case OP_MATCH_ANY_SUBSTRING:
				/* Match 0 or more characters */
				pMatchOp++; /* Need to Match the rest of the string with the rest of the pattern ! */
				while ( 1 ) {
					nRet = MatchSubStringWithPattern( pszToMatch, nToMatchStrLen, pMatchOp );
					if ( nRet == -1 )
						return -1;
					else if ( nRet == 1 )
						return 1; /* MATCH ! */
					else if ( *pszToMatch == '\0' )
						return 0; /* NO Match ! */
					/* Skip One Character and retry */
					pszToMatch++;
					nToMatchStrLen--;
				}
				return -1; /* This point should never be reached ! */
				/* break; */
			case OP_MATCH_SET:
				/* Check if it is in the SET */
				if ( *pszToMatch == '\0' )
					return 0; /* NO Match ! */
				if ( pMatchOp->m_Params.m_arrMatchSetData[ *pszToMatch ] == 0 )
					return 0; /* NO Match ! */
				pszToMatch++;
				break;
			default:
				return -1;
		}

		pMatchOp++;	/* Next Operation */
	} while ( 1 );

	return -1;
}

//------------------------------------------------------------------------------
/* Returns -1 on error, 0 not matched, 1 on match */
int MatchPattern( const char* pszToMatch, TPatternDescription patternDesc ) {
	if ( pszToMatch == NULL )
		return -1;
	if ( patternDesc == NULL )
		return -1;
	if ( patternDesc->m_arrMatchOp == NULL )
		return -1;
	return MatchSubStringWithPattern( pszToMatch, strlen( pszToMatch ), patternDesc->m_arrMatchOp );
}

//------------------------------------------------------------------------------
void PatternMatchingDestroy( TPatternDescription *pPatternDesc ) {
	delete_pattern_description( pPatternDesc );
}

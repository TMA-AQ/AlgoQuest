#include "stdafx.h"
#include "SQLTypes.h"
#include <aq/Logger.h>

#ifdef __cplusplus
extern "C" {
#endif

SQLRETURN  SQL_API SQLNumResultCols(SQLHSTMT StatementHandle,
																		_Out_ SQLSMALLINT * ColumnCount)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	*ColumnCount = ((AqHandleStmt*)StatementHandle)->result->getNbCol();
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLMoreResults(SQLHSTMT StatementHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
  if (((AqHandleStmt*)StatementHandle)->result->moreResults())
    return SQL_SUCCESS;
  return SQL_NO_DATA; // FIXME
}

SQLRETURN  SQL_API SQLRowCount(_In_ SQLHSTMT StatementHandle,
															 _Out_ SQLLEN* RowCount)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
  *RowCount = 1; // FIXME
	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLGetData(SQLHSTMT StatementHandle,
															SQLUSMALLINT ColumnNumber, SQLSMALLINT TargetType,
															_Out_writes_opt_(_Inexpressible_(BufferLength)) SQLPOINTER TargetValue, SQLLEN BufferLength,
															_Out_opt_ SQLLEN *StrLen_or_IndPtr)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
  if (((AqHandleStmt*)StatementHandle)->result->get(ColumnNumber, TargetValue, BufferLength, StrLen_or_IndPtr, TargetType))
    return SQL_SUCCESS;
	return SQL_ERROR;
}

#ifdef _WIN64
SQLRETURN  SQL_API SQLColAttribute (SQLHSTMT StatementHandle,
																		 SQLUSMALLINT ColumnNumber, 
																		 SQLUSMALLINT FieldIdentifier,
																		 _Out_writes_bytes_opt_(BufferLength) SQLPOINTER CharacterAttribute, 
																		 SQLSMALLINT BufferLength,
																		 _Out_opt_ SQLSMALLINT *StringLength, 
																		 _Out_opt_ SQLLEN *NumericAttribute)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	const aq::ResultSet::col_attr_t * const attr = ((AqHandleStmt*)StatementHandle)->result->getColAttr(ColumnNumber);
	switch (FieldIdentifier)
	{
  case SQL_DESC_NAME:
    if (StringLength != NULL)
    {
      *StringLength = attr->name.size();
    }
		if (CharacterAttribute != NULL)
		{
			memset(CharacterAttribute, 0, BufferLength);
			memcpy(CharacterAttribute, attr->name.c_str(), attr->name.size());
		}
		break;
	case SQL_DESC_DISPLAY_SIZE:
		*NumericAttribute = std::max(attr->size, attr->name.size());
		break;
	case SQL_DESC_CONCISE_TYPE:
		*NumericAttribute = SQL_VARCHAR; // FIXME
		break;
	default:
		AQ_ODBC_LOG("NOT SUPPORTED\n");
	}
	return SQL_SUCCESS;
}
#else
SQLRETURN  SQL_API SQLColAttribute (SQLHSTMT StatementHandle,
																		SQLUSMALLINT ColumnNumber, 
																		SQLUSMALLINT FieldIdentifier,
																		_Out_writes_bytes_opt_(BufferLength) SQLPOINTER CharacterAttribute, 
																		SQLSMALLINT BufferLength,
																		_Out_opt_ SQLSMALLINT *StringLength, 
																		_Out_opt_ SQLPOINTER NumericAttribute)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	const aq::ResultSet::col_attr_t * const attr = ((AqHandleStmt*)StatementHandle)->result->getColAttr(ColumnNumber);

	if (attr == 0)
	{
		aq::Logger::getInstance().log(AQ_ERROR, "cannot get attributes of column %u\n", ColumnNumber);
		return SQL_ERROR;
	}

	switch (FieldIdentifier)
	{
	case SQL_DESC_NAME:
		if (CharacterAttribute == NULL)
		{
			*StringLength = attr->name.size();
		}
		else
		{
			memset(CharacterAttribute, 0, BufferLength);
			memcpy(CharacterAttribute, attr->name.c_str(), attr->name.size());
		}
		break;
	case SQL_DESC_DISPLAY_SIZE:
		*((size_t*)NumericAttribute) = std::max(attr->size, attr->name.size());
		break;
	case SQL_DESC_CONCISE_TYPE:
		*((size_t*)NumericAttribute) = SQL_VARCHAR; // FIXME
		break;
	default:
		AQ_ODBC_LOG("NOT SUPPORTED\n");
	}
	return SQL_SUCCESS;
}
#endif

SQLRETURN  SQL_API SQLDescribeCol(SQLHSTMT StatementHandle,
																	SQLUSMALLINT ColumnNumber, _Out_writes_opt_(BufferLength) SQLCHAR *ColumnName,
																	SQLSMALLINT BufferLength, _Out_opt_ SQLSMALLINT *NameLength,
																	_Out_opt_ SQLSMALLINT *DataType, _Out_opt_ SQLULEN *ColumnSize,
																	_Out_opt_ SQLSMALLINT *DecimalDigits, _Out_opt_ SQLSMALLINT *Nullable)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	const aq::ResultSet::col_attr_t * const attr = ((AqHandleStmt*)StatementHandle)->result->getColAttr(ColumnNumber);

  strcpy((char*)ColumnName, attr->name.c_str());
  *NameLength = attr->name.size();
  *DataType = attr->type;
  *ColumnSize = 32; // FIXME

	return SQL_SUCCESS;
}

SQLRETURN  SQL_API SQLBindCol(SQLHSTMT StatementHandle,
															SQLUSMALLINT ColumnNumber, SQLSMALLINT TargetType,
															_Inout_updates_opt_(_Inexpressible_(BufferLength)) SQLPOINTER TargetValue, 
															SQLLEN BufferLength, _Inout_opt_ SQLLEN *StrLen_or_Ind)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	
	if (((AqHandleStmt*)StatementHandle)->result->bindCol(ColumnNumber, TargetValue, StrLen_or_Ind, TargetType))
		return SQL_SUCCESS;

	return SQL_ERROR;
}

SQLRETURN  SQL_API SQLFetch(SQLHSTMT StatementHandle)
{
	AQ_ODBC_LOG("%s called\n", __FUNCTION__);
	
	if (((AqHandleStmt*)StatementHandle)->result->fetch())
		return SQL_SUCCESS;
	else
		return SQL_NO_DATA;

	return SQL_SUCCESS;
}

#ifdef __cplusplus
}
#endif
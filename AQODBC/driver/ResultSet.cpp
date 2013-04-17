#include "stdafx.h"
#include "ResultSet.h"
#include <iostream>
#include <aq/Logger.h>
#include <aq/Utilities.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/filesystem.hpp>

using namespace aq;

ResultSet::ResultSet()
	:
	_nRows(0),
	_size(0), 
	_eos(false),
	headerFilled(false),
	lineCount1Read(false),
	lineCount2Read(false),
	firstDelimiterFind(false)
{
	aq::Logger::getInstance().log(AQ_DEBUG, "");
	_buffer = (char*)malloc(4097);
}

ResultSet::~ResultSet()
{
}

void ResultSet::setQuery(const char * _query)
{
  this->query = _query;
}

bool ResultSet::get(unsigned short c, void * ptr, size_t len, void * len_ptr, SQLSMALLINT type)
{
	if ((c > 0) && (c <= headers.size()))
	{
		--c;
    if (headers[c].type != type)
      aq::Logger::getInstance().log(AQ_WARNING, "target type differs from internal type: %d != %d", type, headers[c].type);
		memcpy(ptr, currentRow[c].valueBind, std::min((size_t)128, len)); // FIXME
		memcpy(len_ptr, currentRow[c].sizeBind, sizeof(size_t));
		return true;
	}
	return false;
}

bool ResultSet::bindCol(unsigned short c, void * ptr, void * len_ptr, SQLSMALLINT type)
{
	if ((c > 0) && (c <= headers.size()))
	{
		--c;
    if (headers[c].type != type)
      aq::Logger::getInstance().log(AQ_WARNING, "target type differs from internal type: %d != %d", type, headers[c].type);
		headers[c].valueBind = ptr;
		headers[c].sizeBind = len_ptr;
		return true;
	}
	return false;
}

bool ResultSet::moreResults() const
{
	if ((resultIt == results.end()) || ((*resultIt).size() != headers.size()))
  {
    if ((this->fd == NULL) || feof(this->fd))
    {
      return false;
    }
  }
  return true;
}

bool ResultSet::fetch()
{
	if ((resultIt == results.end()) || ((*resultIt).size() != headers.size()))
  {
    if ((this->fd == NULL) || feof(this->fd))
    {
      return false;
    }
    else
    {

      // read rows
      char buf[1024];
      int size = 1024;
      int trimEnd = 1;
      ReadValidLine(fd, buf, size, trimEnd);
      if (!feof(this->fd))
      {
        char * tokens = strtok(buf, ";");
        size_t pos = 0;
        while (tokens != NULL)
        {
          if (pos >= headers.size())
            return false;
          void * value_ptr = headers[pos].valueBind;
          void * size_ptr = headers[pos].sizeBind;
          if ((value_ptr == NULL) && (size_ptr == NULL))
          {
            value_ptr = currentRow[pos].valueBind;
            size_ptr = currentRow[pos].sizeBind;
          }
          std::string token_str(tokens);
          boost::algorithm::trim(token_str);
          size_t size = token_str.size();
          memcpy(value_ptr, token_str.c_str(), size);
          if (headers[pos].type == SQL_C_CHAR)
            ((char*)(value_ptr))[size] = 0;
          memcpy(size_ptr, &size, sizeof(size_t));
          tokens = strtok(NULL, ";");
          ++pos;
        }
      }
      else
      {
        return false;
      }
    }
  }
  else
  {
    assert(headers.size() == (*resultIt).size());
    for (size_t i = 0; i < headers.size(); ++i)
    {
      if ((headers[i].valueBind == 0) || (headers[i].sizeBind == 0))
      {
        aq::Logger::getInstance().log(AQ_DEBUG, "column %u not binded\n", i);
        continue;
      }
      size_t size = (*resultIt)[i].len;
      void * ptr_from = (*resultIt)[i].buf;
      void * ptr_dest = headers[i].valueBind;
      memcpy(ptr_dest, ptr_from, size);
      if (headers[i].type == SQL_C_CHAR)
        ((char*)(ptr_dest))[size] = 0;
      memcpy(headers[i].sizeBind, &size, sizeof(size_t));
    }
    ++resultIt;
  }
	return true;
}

void ResultSet::pushResult(const char * buf, size_t size)
{
	// append to buffer
	assert((_size + size) < 4096);
	memcpy(_buffer + _size, buf, size);
	_size +=size;
	_buffer[_size] = '\0';

	char * cur = _buffer;
	char * val = _buffer;
	while ((*cur != '\0') && (cur < (_buffer + _size)))

	{
		if (!firstDelimiterFind)
		{
			if (*cur == ';')
			{
				firstDelimiterFind = true;
				val = cur + 1;
			}
		}
		else if (!headerFilled)
		{
			if ((*cur == ';') || (*cur == '\n'))
			{
				if (val < cur)
				{
					col_attr_t attr(std::string(val, cur - val).c_str(), 32, SQL_C_CHAR);
					aq::Logger::getInstance().log(AQ_INFO, "add column: %s\n", attr.name.c_str());
					headers.push_back(attr);
				}
				if (*cur == '\n')
				{
					results.push_back(std::vector<col_t>());
          resultIt = results.begin();

          size_t numCol = 0;
          for (results_header_t::const_iterator it = headers.begin(); it != headers.end(); ++it)
          {
            currentRow.push_back(col_attr_t("", 32, SQL_C_CHAR)); // FIXME
            currentRow.rbegin()->valueBind = static_cast<void*>(::malloc(128)); // FIXME
            currentRow.rbegin()->sizeBind = static_cast<void*>(::malloc(sizeof(size_t)));
            this->bindCol(++numCol, currentRow.rbegin()->valueBind, currentRow.rbegin()->sizeBind, SQL_C_CHAR);
          }

					headerFilled = true;
				}
        val = cur + 1;
			}
		}
		else if ((*cur == ';') || (*cur == '\n'))
		{
			if (val < cur)
			{
				// store value
				std::string v(val, cur - val);
				val = cur + 1;

        // check eos
        if (v == "EOS")
        {
          _eos = true;
          val = cur;
          break;
        }

				col_t c(v);

				(*resultIt).push_back(c);
				if ((*resultIt).size() == headers.size()) 
				{
					assert (*cur == '\n');
					++_nRows;
					results.push_back(std::vector<col_t>());
					resultIt = results.end();
					--resultIt;
				}

				val = cur + 1;
			}
			else
			{
				val = cur + 1;
			}

		}
		++cur;
	}

	if (val < cur)
	{
		memcpy(_buffer, val, cur - val);
		_size = cur - val;
	}
	else
	{
		_size = 0;
	}

	if (_eos)
	{
		resultIt = results.begin();
		aq::Logger::getInstance().log(AQ_ERROR, "EOS\n");
	}

}

void ResultSet::loadCatalg(const char * db)
{
  std::string db_base_desc(db);
  db_base_desc += "/base_struct/base";
  build_base_from_raw(db_base_desc.c_str(), baseDesc);
  // dump_base(std::cout, baseDesc);
}

void ResultSet::loadCatalg(std::istream& data)
{
  try
  {
    build_base_from_xml(data, baseDesc);
  }
  catch (const std::exception& ex)
  {
  }
}

void ResultSet::clearBases()
{
	// clean
  headers.clear();
	results.clear();
}

void ResultSet::addBases(const char * db)
{
  // fill headers if needed
  if (headers.empty())
  {
    headers.push_back(col_attr_t("TABLE_SCHEM", 32, SQL_C_CHAR));
  }

  // fill rows
  std::vector<col_t> line;
  col_t c1(db);
  line.push_back(c1);
  results.push_back(line);

	// set cursor
	resultIt = results.begin();
}

void ResultSet::fillBases(const char * path)
{
	// clean
  headers.clear();
	results.clear();

  // fill headers
  headers.push_back(col_attr_t("TABLE_SCHEM", 32, SQL_C_CHAR));

  // fill rows
  boost::filesystem::path p(path);
  if (boost::filesystem::exists(p) && boost::filesystem::is_directory(p))
  {
    std::vector<boost::filesystem::path> dbs;
    std::copy(boost::filesystem::directory_iterator(p), boost::filesystem::directory_iterator(), std::back_inserter(dbs));
    for (std::vector<boost::filesystem::path>::const_iterator it = dbs.begin(); it != dbs.end(); ++it)
    {
      const boost::filesystem::path& db = *it;
      if (boost::filesystem::is_directory(db))
      {
        std::vector<col_t> line;
        col_t c1(db.filename().generic_string());
        line.push_back(c1);
        results.push_back(line);
      }
    }
  }

	// set cursor
	resultIt = results.begin();
}

void ResultSet::fillTables()
{
	// clean
  headers.clear();
	results.clear();
  
  // fill headers
  headers.push_back(col_attr_t("TABLE_CAT", 32, SQL_C_CHAR));
  headers.push_back(col_attr_t("TABLE_SCHEM", 32, SQL_C_CHAR));
  headers.push_back(col_attr_t("TABLE_NAME", 32, SQL_C_CHAR));
  headers.push_back(col_attr_t("TABLE_TYPE", 32, SQL_C_CHAR));
  headers.push_back(col_attr_t("REMARKS", 32, SQL_C_CHAR));

  // fill rows
  unsigned int numCol = 0;
  for (aq::base_t::tables_t::const_iterator it = baseDesc.table.begin(); it != baseDesc.table.end(); ++it)
  {
    const base_t::table_t& table = *it;
    std::vector<col_t> row;
    row.push_back(col_t("NULL"));
    row.push_back(col_t(baseDesc.nom));
    row.push_back(col_t(table.nom));
    row.push_back(col_t("TABLE"));
    row.push_back(col_t(""));
    results.push_back(row);

    // bind
    currentRow.push_back(col_attr_t("", 32, SQL_C_CHAR)); // FIXME
    currentRow.rbegin()->valueBind = static_cast<void*>(::malloc(128)); // FIXME
    currentRow.rbegin()->sizeBind = static_cast<void*>(::malloc(sizeof(size_t)));
    this->bindCol(++numCol, currentRow.rbegin()->valueBind, currentRow.rbegin()->sizeBind, SQL_C_CHAR);
  }


	// set cursor
	resultIt = results.begin();
}

void ResultSet::fillColumns(const char * tableName)
{
	// clean
  headers.clear();
	results.clear();
  
  // fill headers
	headers.push_back(col_attr_t("TABLE_CAT", 32, SQL_C_CHAR));
	headers.push_back(col_attr_t("TABLE_SCHEM", 32, SQL_C_CHAR));
	headers.push_back(col_attr_t("TABLE_NAME", 32, SQL_C_CHAR));
	headers.push_back(col_attr_t("COLUMN_NAME", 32, SQL_C_CHAR));
	headers.push_back(col_attr_t("DATA_TYPE", 32, SQL_C_SSHORT));
	headers.push_back(col_attr_t("TYPE_NAME ", 32, SQL_C_CHAR));
	headers.push_back(col_attr_t("COLUMN_SIZE", 32, SQL_C_SLONG));
	headers.push_back(col_attr_t("BUFFER_LENGTH", 32, SQL_C_SLONG));
	headers.push_back(col_attr_t("DECIMAL_DIGITS", 32, SQL_C_SLONG));
	headers.push_back(col_attr_t("NUM_PREC_RADIX", 32, SQL_C_CHAR));
	headers.push_back(col_attr_t("NULLABLE", 32, SQL_C_CHAR));
	headers.push_back(col_attr_t("REMARKS", 32, SQL_C_CHAR));
	headers.push_back(col_attr_t("COLUMN_DEF", 32, SQL_C_CHAR));
	headers.push_back(col_attr_t("SQL_DATA_TYPE", 32, SQL_C_CHAR));
	// headers.push_back(col_attr_t("SQL_DATETIME_SUB", 32, SQL_C_CHAR));
	headers.push_back(col_attr_t("CHAR_OCTET_LENGTH", 32, SQL_C_CHAR));
	headers.push_back(col_attr_t("ORDINAL_POSITION", 32, SQL_C_CHAR));
	headers.push_back(col_attr_t("IS_NULLABLE", 32, SQL_C_CHAR));
	// headers.push_back(col_attr_t("REMARKS", 32, SQL_C_CHAR));
	
  // fill rows
  for (aq::base_t::tables_t::const_iterator it_table = baseDesc.table.begin(); it_table != baseDesc.table.end(); ++it_table)
  {
    const base_t::table_t& table = *it_table;
    if ((table.nom == tableName) || (tableName == "%"))
    {
      for (aq::base_t::table_t::cols_t::const_iterator it_col = table.colonne.begin(); it_col != table.colonne.end(); ++it_col)
      {
        const base_t::table_t::col_t& column = *it_col;
        std::vector<col_t> row;
        row.push_back(col_t("NULL"));
        row.push_back(col_t(baseDesc.nom));
        row.push_back(col_t(table.nom));
        row.push_back(col_t(column.nom));
        switch(column.type)
        {
        case t_int:
          row.push_back(col_t(SQL_INTEGER));
          row.push_back(col_t("INTEGER"));
          break;
        case t_double:
          row.push_back(col_t(SQL_DOUBLE));
          row.push_back(col_t("DOUBLE"));
          break;
        case t_date1: 
        case t_date2: 
        case t_date3:
          row.push_back(col_t(SQL_DATE));
          row.push_back(col_t("DATE"));
          break;
        case t_char: 
          row.push_back(col_t(SQL_VARCHAR));
          row.push_back(col_t("VARCHAR"));
          break;
        case t_long_long: 
          row.push_back(col_t(SQL_INTEGER));
          row.push_back(col_t("LONG"));
          break;
        case t_raw:
          row.push_back(col_t(SQL_VARBINARY));
          row.push_back(col_t("RAW"));
          break;
        default:
          aq::Logger::getInstance().log(AQ_ERROR, "invalid type %d for column", column.type);
          row.push_back(col_t(0));
          row.push_back(col_t(""));
        }
        for (unsigned int i = 6; i < headers.size(); ++i) // TODO
        {
          row.push_back(col_t(""));
        }
        results.push_back(row);
      }
    }
  }

	// set cursor
	resultIt = results.begin();
}

void ResultSet::openResultCursor(const char * resultFilename)
{
  // clean
  currentRow.clear(); // fixme : delete binding if not null
  headers.clear();
  results.clear();

  // open cursor
  this->fd = fopen(resultFilename, "r");
  if (this->fd != NULL)
  {
    // read rows description
    char buf[1024];
    int size = 1024;
    int trimEnd = 1;
    ReadValidLine(fd, buf, size, trimEnd);
    char * tokens = strtok(buf, ";");
    while (tokens != NULL)
    {
      aq::Logger::getInstance().log(AQ_DEBUG, "read column '%s'\n", tokens);
      std::string token_str(tokens);
      boost::algorithm::trim(token_str);
      headers.push_back(col_attr_t(token_str.c_str(), 32, SQL_C_CHAR)); // FIXME
      currentRow.push_back(col_attr_t(token_str.c_str(), 32, SQL_C_CHAR)); // FIXME
      currentRow.rbegin()->valueBind = static_cast<void*>(::malloc(128)); // FIXME
      currentRow.rbegin()->sizeBind = static_cast<void*>(::malloc(sizeof(size_t)));
      this->bindCol(headers.size(), currentRow.rbegin()->valueBind, currentRow.rbegin()->sizeBind, SQL_C_CHAR);
      tokens = strtok(NULL, ";");
    }
  }

  // set cursor
  resultIt = results.begin();
}

void ResultSet::fillTypeInfos()
{
	// clean
	results.clear();

	// fill
	std::list<std::string> l;
	l.push_back("TYPE_NAME");
	l.push_back("DATA_TYPE");
	l.push_back("COLUMN_SIZE");
	l.push_back("LITERAL_PREFIX");
	l.push_back("LITERAL_SUFFIX");
	l.push_back("CREATE_PARAMS");
	l.push_back("NULLABLE");
	l.push_back("CASE_SENSITIVE");
	l.push_back("SEARCHABLE");
	l.push_back("UNSIGNED_ATTRIBUTE");
	l.push_back("FIXED_PREC_SCALE");
	l.push_back("AUTO_UNIQUE_VALUE");
	l.push_back("LOCAL_TYPE_NAME");
	l.push_back("MINIMUM_SCALE");
	l.push_back("MAXIMUM_SCALE");
	l.push_back("SQL_DATA_TYPE");
	l.push_back("SQL_DATETIME_SUB");
	l.push_back("NUM_PREC_RADIX");
	l.push_back("INTERVAL_PRECISION");
  
	std::vector<col_t> l1;
	for (std::list<std::string>::const_iterator it = l.begin(); it != l.end(); ++it)
  {
		col_attr_t c1((*it).c_str(), 32, SQL_C_CHAR);
		headers.push_back(c1);
		l1.push_back(col_t(""));
	}
	results.push_back(l1);

	// set cursor
	resultIt = results.begin();
}

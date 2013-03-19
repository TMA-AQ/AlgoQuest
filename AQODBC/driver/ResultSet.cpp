#include "stdafx.h"
#include "ResultSet.h"
#include <aq/Logger.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

using namespace aq;

ResultSet::ResultSet()
	:
	_n(0),
	_nlines(0),
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

bool ResultSet::bindCol(unsigned short c, void * ptr, void * len_ptr)
{
	if ((c > 0) && (c <= headers.size()))
	{
		--c;
		headers[c].valueBind = ptr;
		headers[c].sizeBind = len_ptr;
		return true;
	}
	return false;
}

bool ResultSet::fetch()
{
	if ((resultIt == results.end()) || ((*resultIt).size() != headers.size()))
		return false;

	assert(headers.size() == (*resultIt).size());
	for (size_t i = 0; i < headers.size(); ++i)
	{
		if ((headers[i].valueBind == 0) || (headers[i].sizeBind == 0))
		{
			aq::Logger::getInstance().log(AQ_ERROR, "column %u not binded\n", i);
			continue;
		}
		size_t size = (*resultIt)[i].value.size();
		memcpy(headers[i].valueBind, (*resultIt)[i].value.c_str(), size);
		((char*)(headers[i].valueBind))[size] = 0;
		memcpy(headers[i].sizeBind, &size, sizeof(size_t));
	}

	++resultIt;

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
					col_attr_t attr;
					attr.name = std::string(val, cur - val);
					val = cur + 1;
					attr.size = 32; // fixme
					attr.valueBind = (void*)0;
					attr.sizeBind = (void*)0;
					aq::Logger::getInstance().log(AQ_INFO, "add column: %s\n", attr.name.c_str());
					headers.push_back(attr);
				}
				if (*cur == '\n')
				{
					results.push_back(std::vector<col_t>());
					resultIt = results.begin();
					headerFilled = true;
				}
			}
		}
		else if (!lineCount1Read)
		{
			if (*cur == '\n')
			{
				std::string s(val, cur - val);
				boost::algorithm::trim(s);
				_nlines = boost::lexical_cast<unsigned long>(s);
				val = cur + 1;
				lineCount1Read = true;
				aq::Logger::getInstance().log(AQ_INFO, "read %u lines\n", _nlines);
			}
		}
		else if (!lineCount2Read)
		{
			if (*cur == '\n')
			{
				std::string s(val, cur - val);
				val = cur + 1;
				lineCount2Read = true;
			}
		}
		else if ((*cur == ';') || (*cur == '\n'))
		{
			if (val < cur)
			{
				// store value
				std::string v(val, cur - val);
				val = cur + 1;
				col_t c(v);

				(*resultIt).push_back(c);
				if ((*resultIt).size() == headers.size()) 
				{
					assert (*cur == '\n');
					++_n;
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

	_eos = firstDelimiterFind && headerFilled && lineCount1Read && lineCount2Read && (_n == _nlines);

	if (_eos)
	{
		resultIt = results.begin();
		aq::Logger::getInstance().log(AQ_ERROR, "EOS\n");
	}

}

// -----------------------------------------------------------------------------------------------
// TESTS PURPOSE

void ResultSet::fillSimulateResultTables1()
{
	// clean
	results.clear();

	// fill
	{
		col_attr_t c1_attr;
		c1_attr.name = "TABLE_SCHEM";
		c1_attr.size = 32;
		headers.push_back(c1_attr);
		std::vector<col_t> line;
		col_t c1("test");
		line.push_back(c1);
		results.push_back(line);
		results.push_back(line);
		results.push_back(line);
	}

	// set cursor
	resultIt = results.begin();
}

void ResultSet::fillSimulateResultTables2()
{
	// clean
	results.clear();

	{
		col_attr_t c1;
		c1.name = "TABLE_CAT";
		c1.size = 32;
		headers.push_back(c1);
	}

	{
		col_attr_t c1;
		c1.name = "TABLE_SCHEM";
		c1.size = 32;
		headers.push_back(c1);
	}

	{
		col_attr_t c1;
		c1.name = "TABLE_NAME";
		c1.size = 32;
		headers.push_back(c1);
	}

	{
		col_attr_t c1;
		c1.name = "TABLE_TYPE";
		c1.size = 32;
		headers.push_back(c1);
	}

	{
		col_attr_t c1;
		c1.name = "REMARKS";
		c1.size = 32;
		headers.push_back(c1);
	}

	std::vector<col_t> l1;
	l1.push_back(col_t("NULL"));
	l1.push_back(col_t("test"));
	l1.push_back(col_t("table1"));
	l1.push_back(col_t("TABLE"));
	l1.push_back(col_t(""));

	std::vector<col_t> l2;
	l2.push_back(col_t("NULL"));
	l2.push_back(col_t("test"));
	l2.push_back(col_t("table2"));
	l2.push_back(col_t("TABLE"));
	l2.push_back(col_t(""));

	std::vector<col_t> l3;
	l3.push_back(col_t("NULL"));
	l3.push_back(col_t("test"));
	l3.push_back(col_t("table3"));
	l3.push_back(col_t("TABLE"));
	l3.push_back(col_t(""));

	results.push_back(l1);
	results.push_back(l2);
	results.push_back(l3);

	// set cursor
	resultIt = results.begin();
}

void ResultSet::fillSimulateResultColumns(const char * TableName, size_t NameLength)
{
	// clean
	results.clear();
	std::string tableName(TableName, TableName + NameLength);

	{
		col_attr_t c1;
		c1.name = "TABLE_CAT";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}

	{
		col_attr_t c1;
		c1.name = "TABLE_SCHEM";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}

	{
		col_attr_t c1;
		c1.name = "TABLE_NAME";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}

	{
		col_attr_t c1;
		c1.name = "COLUMN_NAME";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}

	{
		col_attr_t c1;
		c1.name = "DATA_TYPE";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}
	
	{
		col_attr_t c1;
		c1.name = "TYPE_NAME";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}
	
	{
		col_attr_t c1;
		c1.name = "COLUMN_SIZE";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}
	
	{
		col_attr_t c1;
		c1.name = "BUFFER_LENGTH";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}
	
	{
		col_attr_t c1;
		c1.name = "DECIMAL_DIGITS";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}
	
	{
		col_attr_t c1;
		c1.name = "NUM_PREC_RADIX";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}
	
	{
		col_attr_t c1;
		c1.name = "NULLABLE";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}
	
	{
		col_attr_t c1;
		c1.name = "REMARKS";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}
	
	{
		col_attr_t c1;
		c1.name = "COLUMN_DEF";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}
	
	{
		col_attr_t c1;
		c1.name = "SQL_DATA_TYPE";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}
	
	{
		col_attr_t c1;
		c1.name = "SQL_DATETIME_SUB";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}
	
	{
		col_attr_t c1;
		c1.name = "CHAR_OCTET_LENGTH";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}
	
	{
		col_attr_t c1;
		c1.name = "ORDINAL_POSITION";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}
	
	{
		col_attr_t c1;
		c1.name = "IS_NULLABLE";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	}
	
	std::list<std::string> c;
	c.push_back("id");
	c.push_back("val_1");
	c.push_back("val_2");

	if (strncmp(TableName, "table2", NameLength) == 0)
	{
		c.push_back("id_t1");
	}
	else if (strncmp(TableName, "table3", NameLength) == 0)
	{
		c.push_back("id_t2");
	}

	std::for_each (c.begin(), c.end(), [&] (std::string colName)
	{
		std::vector<col_t> l1;
		l1.push_back(col_t("NULL"));
		l1.push_back(col_t("test"));
		l1.push_back(col_t(TableName));
		l1.push_back(col_t(colName));
		l1.push_back(col_t("4"));
		l1.push_back(col_t("INTEGER"));
		for (unsigned int i = 6; i <= 19; ++i)
		{
			l1.push_back(col_t(""));
		}
		results.push_back(l1);
	});

	// set cursor
	resultIt = results.begin();
}

void ResultSet::fillSimulateResultTypeInfos()
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

	std::for_each(l.begin(), l.end(), [&] (std::string colname)
	{
		col_attr_t c1;
		c1.name = "type";
		c1.size = 32;
		c1.valueBind = (void*)0;
		c1.sizeBind = (void*)0;
		headers.push_back(c1);
	});
	
	std::vector<col_t> l1;
	std::for_each(l.begin(), l.end(), [&] (std::string colname)
	{
		l1.push_back(col_t(""));
	});
	results.push_back(l1);

	// set cursor
	resultIt = results.begin();
}

void ResultSet::fillSimulateResultQuery()
{
	// clean
	results.clear();

	// fill

	// set cursor
	resultIt = results.begin();
}

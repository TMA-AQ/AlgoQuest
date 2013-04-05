#ifndef __RESULT_SET_HH__
#define __RESULT_SET_HH__

#include <aq/BaseDesc.h>
#include <vector>
#include <list>
#include <boost/variant.hpp>
// #include <boost/tuple/tuple.hpp>

namespace aq
{

class ResultSet
{
public:

	//typedef struct 
	//{
	//	std::string name;
	//	size_t size;
	//} col_attr_t;

	//typedef std::vector<boost::tuple<col_attr_t, std::list<std::string>, std::list<std::string>::const_iterator, void *, void * > > results_t; 
	
	struct col_attr_t
	{
		std::string name;
		size_t size;
    SQLSMALLINT type;
		void * valueBind;
		void * sizeBind;
    col_attr_t(const char * _name, size_t _size, SQLSMALLINT _type) : name(_name), size(_size), type(_type), valueBind(NULL), sizeBind(NULL) {}
	};
	typedef std::vector<col_attr_t> results_header_t; 

	struct col_t
	{
		col_t(std::string s)
    {
      len = s.size() + 1;
      buf = ::malloc(len);
      ::memcpy(buf, s.c_str(), len);
    }
		col_t(int v)
    {
      len = sizeof(int);
      buf = ::malloc(sizeof(int));
      ::memcpy(buf, &v, len);
    }
    void * buf;
    size_t len;
		// std::string value;
		// boost::variant<std::string, size_t> value;
	};
  typedef std::vector<col_t> row_t;
	typedef std::list<row_t> results_t; 

	struct writer_t : public boost::static_visitor<>
	{
		writer_t(col_attr_t _c) : c(_c)
		{
		}

		template <typename T>
		void operator()(const T& t)
		{
			::memcpy(c.valueBind, t, c.size);
			::memcpy(c.sizeBind, c.size, sizeof(size_t));
		}

		private:
			col_attr_t c;
	};

public:
	
	ResultSet();
	~ResultSet();

	void pushResult(const char * buf, size_t size);
	bool eos() const { return _eos; }

	size_t getNbCol() const { 
    return headers.size(); 
  }
	const col_attr_t * const getColAttr(unsigned short c) const { 
		if ((c == 0) || (c > headers.size())) return NULL ; else return &headers[c - 1]; 
	}
  
	bool get(unsigned short c, void * ptr, size_t len, void * len_ptr, SQLSMALLINT type);
	bool bindCol(unsigned short c, void * ptr, void * len_ptr, SQLSMALLINT type);
	bool fetch();
  bool moreResults() const;

  void loadCatalg(const char * db);

  void fillBases(const char * path);
  void fillTables();
  void fillColumns(const char *);
  void fillTypeInfos();
  
  void openResultCursor(const char * resultFilename);

  void setQuery(const char * _query);
  const std::string& getQuery() { return this->query; }

private:
  base_t baseDesc;
	results_t results;
  results_header_t currentRow;
	results_header_t headers;
	results_t::iterator resultIt;
  std::string query;
  FILE * fd;
	unsigned long _n;
	unsigned long _nlines;
	bool _eos;
	bool headerFilled;
	bool lineCount1Read;
	bool lineCount2Read;
	bool firstDelimiterFind;
	size_t _size;
	char * _buffer;

};

}

#endif

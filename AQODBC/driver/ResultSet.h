#ifndef __RESULT_SET_HH__
#define __RESULT_SET_HH__

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
		void * valueBind;
		void * sizeBind;
	};
	typedef std::vector<col_attr_t> results_header_t; 

	struct col_t
	{
		col_t(std::string s) : value(s) {}
		std::string value;
		// boost::variant<std::string, size_t> value;
	};
	typedef std::list<std::vector<col_t> > results_t; 

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

	size_t getNbCol() const { return headers.size(); }
	const col_attr_t const * getColAttr(unsigned short c) const { 
		if ((c == 0) || (c > headers.size())) return NULL ; else return &headers[c - 1]; 
	}

	bool bindCol(unsigned short c, void * ptr, void * len_ptr);
	bool fetch();

	// testing purpose
	void fillSimulateResultTables1();
	void fillSimulateResultTables2();
	void fillSimulateResultColumns(const char * TableName, size_t NameLength);
	void fillSimulateResultTypeInfos();
	void fillSimulateResultQuery();

private:

	results_t results;
	results_header_t headers;
	results_t::iterator resultIt;
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

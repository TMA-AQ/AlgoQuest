#include "Base.h"
#include <aq/Logger.h>
#include <aq/Exceptions.h>
#include <fstream>
#include <boost/bind.hpp>
#include <boost/scoped_array.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/case_conv.hpp>

namespace aq
{
  
// -------------------------------------------------------------------------------------------------
Base::Base()
{
}

// -------------------------------------------------------------------------------------------------
Base::Base(const Base& source)
{
  for (auto& t : source.Tables)
  {
    this->Tables.push_back(new Table(*t));
  }
  this->Name = source.Name;
}

// -------------------------------------------------------------------------------------------------
Base::Base(const std::string& filename)
{
  Base::load(filename, *this);
}

// -------------------------------------------------------------------------------------------------
Base::~Base()
{
}

// -------------------------------------------------------------------------------------------------
Base& Base::operator=(const Base& source)
{
  if (this != &source)
  {
    this->Name = source.Name;
    for (auto& t : source.Tables)
    {
      this->Tables.push_back(new Table(*t));
    }
  }
  return *this;
}

// -------------------------------------------------------------------------------------------------
int Base::load(const std::string& filename, aq::Base& bd)
{
  if (filename == "")
  {
    aq::Logger::getInstance().log(AQ_WARNING, "no database specify");
    return -1;
  }
  aq::Logger::getInstance().log(AQ_INFO, "load base %s\n", filename.c_str());
  std::fstream bdFile(filename.c_str());
  aq::base_t baseDescHolder;
  if (filename.substr(filename.size() - 4) == ".xml")
  {
    aq::build_base_from_xml(bdFile, baseDescHolder);
  }
  else
  {
    aq::build_base_from_raw(bdFile, baseDescHolder);
  }
  bd.loadFromBaseDesc(baseDescHolder);
  return 0;
}

//------------------------------------------------------------------------------
Table::Ptr Base::getTable(size_t id)
{
	for( size_t i = 0; i < this->Tables.size(); ++i )
		if( id == this->Tables[i]->ID )
			return this->Tables[i];
	throw generic_error(generic_error::INVALID_TABLE, "cannot find table %u", id);
}

//------------------------------------------------------------------------------
const Table::Ptr Base::getTable(size_t id) const
{
	return const_cast<Base*>(this)->getTable(id);
}

//------------------------------------------------------------------------------
Table::Ptr Base::getTable( const std::string& name )
{
	std::string auxName = name;
	boost::to_upper(auxName);
	boost::trim(auxName);
	for( size_t idx = 0; idx < this->Tables.size(); ++idx )
  {
		if( auxName == this->Tables[idx]->getName() )
    {
      //if (this->Tables[idx]->getReferenceTable() != "")
      //{
      //  return this->getTable(this->Tables[idx]->getReferenceTable());
      //}
      //else
      //{
        return this->Tables[idx];
      //}
    }
  }
	throw generic_error(generic_error::INVALID_TABLE, "cannot find table %s", name.c_str());
}

//------------------------------------------------------------------------------
const Table::Ptr Base::getTable( const std::string& name ) const
{
  return const_cast<Base*>(this)->getTable(name);
}


//------------------------------------------------------------------------------
void Base::clear()
{
  this->Name = "";
  this->Tables.clear();
}

//------------------------------------------------------------------------------
void Base::loadFromBaseDesc(const aq::base_t& base) 
{
  this->Name = base.name;
  std::for_each(base.table.begin(), base.table.end(), [&] (const base_t::table_t& table) {
		Table::Ptr pTD(new Table(table.name, table.id));
		pTD->TotalCount = table.nb_record;
    std::for_each(table.colonne.begin(), table.colonne.end(), [&] (const base_t::table_t::col_t& column) {
      aq::ColumnType type = aq::symbole_to_column_type(column.type);
      unsigned int size = 0;
      switch (type)
      {
      case COL_TYPE_VARCHAR: 
        size = column.size; 
        break;
      case COL_TYPE_INT: 
      case COL_TYPE_BIG_INT:
      case COL_TYPE_DOUBLE:
      case COL_TYPE_DATE: 
        size = 1; 
        break;
      }
      pTD->Columns.push_back(new Column(column.name, column.id, size, type));
		});
		this->Tables.push_back(pTD);
  });
}

//------------------------------------------------------------------------------
void Base::dumpRaw( std::ostream& os ) const
{
  os << this->Name << std::endl;
  os << this->Tables.size() << std::endl << std::endl;
  for (auto& table : this->Tables)
  {
    table->dumpRaw(os);
  }
 }
 
//------------------------------------------------------------------------------
void Base::dumpXml( std::ostream& os ) const
{
  os << "<Database Name=\"" << this->Name << "\">" << std::endl;
  os << "<Tables>" << std::endl;
  std::for_each(this->Tables.begin(), this->Tables.end(), boost::bind(&Table::dumpXml, _1, boost::ref(os)));
  os << "</Tables>" << std::endl;
  os << "</Database>" << std::endl;
 }
 
}
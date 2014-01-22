#ifndef __ROW_H__
#define __ROW_H__

#include "ColumnItem.h"
#include <vector>
#include <boost/variant.hpp>

namespace aq
{

  /// \brief structure holding informations about item on the current row being processeed
  ///
  /// an item is represented in the SELECT clause of a sql query. this can be a column, but also a computed value
  /// \todo this structure should be more complex and hold a complete description of the sql verb to be apply
  struct row_item_t
  {
    typedef boost::variant<ColumnItem<int32_t>, ColumnItem<int64_t>, ColumnItem<double>, ColumnItem<char*> > item_t;
    item_t item; ///< value of the item
    aq::ColumnType type; ///< type of the item (needed to retrieve value)
    unsigned int size; ///< size of the item (used for array)
    std::string tableName; ///< table's name in base description of the item (can be temporary table)
    std::string columnName; ///< column's name in table
    aq::aggregate_function_t aggFunc; ///< the type of aggregate function to be applied if there is
    
    /// \name flags
    /// item flags
    /// \todo use masks system
    /// \{
    bool computed;
    bool grouped;
    bool displayed;
    bool null;
    
    /// \brief build a row_item
    row_item_t();

    /// \brief build a row_item
    /// \param _item
    /// \param _type
    /// \param _size
    /// \param _tableName
    /// \param _columnName
    /// \param _computed
    row_item_t(const item_t& _item, aq::ColumnType _type, unsigned int _size, std::string _tableName, std::string _columnName, bool _computed = false);

    /// \brief copy constructor
    row_item_t(const row_item_t& source);

    /// \brief desctructor
    ~row_item_t();

    /// \brief assignement operator
    row_item_t& operator=(const row_item_t& source);

    /// \brief compare item to a couple of (tableName,columnName)
    /// \param _tableName
    /// \param _columnName
    /// \return true is item match, false otherwise
    bool match(const std::string& _tableName, const std::string& _columnName);
  };

  /// \brief hold all the item in a SELECT clause
  ///
  /// - Store column item value needed before SELECT processing in a std::vector. 
  ///   This value are the result of the FROM, WHERE, GROUP and ORDER clause.
  /// - Then after processed (by the verb tree) the item computed are store in another std::vector
  /// \note For the ORDER clause there is strong limitation. See AQ_Engine functionalities for more explanations.
  class Row
  {
  public:

    typedef std::vector<size_t> index_t;
    typedef std::vector<row_item_t> row_t;

    Row();
    Row(const Row& source);
    ~Row();
    Row& operator=(const Row& source);

    index_t indexes; ///< store the table indexes needed to fill initial row
    row_t initialRow; ///< store the initial row values needed to processed the SELECT clause
    row_t computedRow; ///< store the computed row after processed the SELECT clause
    unsigned int count; ///< used by AQ_Engine on indexes

    /// \name flags
    /// row flags
    /// \todo use masks system
    /// \{
    bool completed;
    bool reinit;
    bool flush;
    /// \}
  };

}

#endif
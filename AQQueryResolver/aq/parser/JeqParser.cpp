#include "JeqParser.h"
#include "ID2Str.h"
#include <vector>
#include <map>
#include <set>
#include <cassert>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>

extern const int nrJoinTypes;
extern const int joinTypes[];
extern const int inverseTypes[];

namespace aq
{

//------------------------------------------------------------------------------
struct connectionLine
{
	int lineId;
	std::string table1;
	std::string table2;
  std::string table1JoinType; ///< k_inner / k_outer
  std::string table2JoinType; ///< k_inner / k_outer
	std::vector<std::string> tableAndCol1;
	std::vector<std::string> tableAndCol2;
	bool isUsed;
	std::vector<std::string> connectionType; ///< operator (k_jeq, k_jneq, k_jinf, k_jieq, k_jseq, k_jsup)

	connectionLine(): isUsed(false){}
};

//------------------------------------------------------------------------------
struct kjeqConnection
{
	int lineId;
	std::string tableSource;
	std::string tableDestination;
};

///
void ReadJeq(	std::string& inputString, 
				std::vector<connectionLine>& connectionArray, 
				std::map<std::string, int>& tablesListAndOccurrence );


/// AssembleJeq will construct a table with the path to follow.
/// it uses the connectionArray
/// and stores the result as 'kjeqConnection' (lineId, tableSource, tableDestination) in a list : orederedConnections
/// 
/// ======================================================
/// |    lineId  |   tableSource |   tableDestination    |
/// ======================================================
/// |    2       |   person      |   car                 |
/// ------------------------------------------------------
/// |    3       |   car         |   brand               |
/// ------------------------------------------------------
/// |    0       |   brand       |   car                 |
/// ------------------------------------------------------
/// |    1       |   car         |   motor               |
/// ======================================================
/// 
/// the lineId represents the Id of the connectionLine in connectionsArray where tableSource and tableDestination are both presented
void AssembleJeq(	std::vector<connectionLine>& connectionArray,
					std::map<std::string, int>& tablesListAndOccurrence,
					std::vector<kjeqConnection>& orderedC );

///
void WriteJeq(	std::string& inputString,
				const std::vector<kjeqConnection>& orderedConnections,
				const std::vector<connectionLine>& connectionArray,
        bool add_active_neutral_filter );

//------------------------------------------------------------------------------
void ParseJeq( std::string& inputString, bool add_active_neutral_filter )
{
	std::vector<connectionLine> connectionArray;
	std::map<std::string, int> tablesListAndOccurrence;
	ReadJeq( inputString, connectionArray, tablesListAndOccurrence );
	if( connectionArray.size() == 0 )
		return;
	std::vector<kjeqConnection> orderedConnections;
	AssembleJeq( connectionArray, tablesListAndOccurrence, orderedConnections );
	WriteJeq( inputString, orderedConnections, connectionArray, add_active_neutral_filter );
}

//------------------------------------------------------------------------------
void findJoin( const std::string& inputString, std::string& joinType, std::string::size_type& position )
{
	position = std::string::npos;
	for( int idx = 0; idx < nrJoinTypes; ++idx )
	{
		std::string::size_type pos = inputString.find( id_to_string( joinTypes[idx] ) );
		if( pos != std::string::npos && (pos < position || position == std::string::npos) )
		{
			position = pos;
			joinType = id_to_string( joinTypes[idx] );
		}
	}
}

//------------------------------------------------------------------------------
std::string invertJoin( const std::string& join )
{
	int idx = 0;
	for( ; idx < nrJoinTypes; ++idx )
		if( id_to_string(joinTypes[idx]) == join )
			break;
	if( idx == nrJoinTypes )
		return join;
	return id_to_string( inverseTypes[idx] );
}

//------------------------------------------------------------------------------
connectionLine nextConnectionLine( std::string& inputString )
{
	connectionLine CL;
  std::string::size_type pos = std::string::npos;
	std::string::size_type nextPosition = std::string::npos;
	std::string connectionType;
	if( inputString.length() > 0 )
		findJoin( inputString, connectionType, nextPosition );

	if( nextPosition != std::string::npos )
  {
		inputString = inputString.substr( nextPosition );
  }
	else
  {
		return CL;
  }

	CL.connectionType.push_back( connectionType );

	inputString = inputString.substr( inputString.find(' ') + 1 );
	boost::algorithm::trim( inputString );

	nextPosition = inputString.find(' ') + 1;
	nextPosition = inputString.find(' ', nextPosition) + 1;
	nextPosition = inputString.find(' ', nextPosition);
  CL.table1 = inputString.substr(0, nextPosition);

  pos = CL.table1.find('.');
  if (pos == std::string::npos)
    throw std::exception(); // TODO
  CL.table1JoinType = CL.table1.substr(0, pos);
  CL.table1 = CL.table1.substr(pos);

	nextPosition = inputString.find(' ', nextPosition + 1);
	CL.tableAndCol1.push_back( inputString.substr(0, nextPosition) );
	boost::algorithm::trim( CL.tableAndCol1[0] );

	inputString = inputString.substr(nextPosition + 1);
	boost::algorithm::trim( inputString );

	nextPosition = inputString.find(' ') + 1;
	nextPosition = inputString.find(' ', nextPosition) + 1;
	nextPosition = inputString.find(' ', nextPosition);
	CL.table2 = inputString.substr(0, nextPosition);
  
  pos = CL.table2.find('.');
  if (pos == std::string::npos)
    throw std::exception(); // TODO
  CL.table2JoinType = CL.table2.substr(0, pos);
  CL.table2 = CL.table2.substr(pos);

	nextPosition = inputString.find(' ', nextPosition + 1);
	if (nextPosition == std::string::npos)  
  {
		CL.tableAndCol2.push_back( inputString );
  }
	else
	{
		CL.tableAndCol2.push_back( inputString.substr(0, nextPosition) );
		boost::algorithm::trim( CL.tableAndCol2[0] );
	}

	boost::algorithm::to_upper(CL.table1);
	boost::algorithm::to_upper(CL.table2);
	std::for_each(CL.tableAndCol1.begin(), CL.tableAndCol1.end(), boost::bind(boost::algorithm::to_upper<std::string>, _1, std::locale()));
	std::for_each(CL.tableAndCol2.begin(), CL.tableAndCol2.end(), boost::bind(boost::algorithm::to_upper<std::string>, _1, std::locale()));

	return CL;
}

//------------------------------------------------------------------------------
void getConnectionArray(std::string inputString, 
                        std::vector<connectionLine>& CA )
{                     
	connectionLine CL;
	CL = nextConnectionLine( inputString );
	while( CL.table1 != "" )
	{
		assert( CL.tableAndCol1.size() == 1 );
		assert( CL.tableAndCol2.size() == 1 );

		bool found = false;
		for( size_t idx = 0; idx < CA.size(); ++idx )
    {
      if ( (CA[idx].table1 == CL.table1) && (CA[idx].table2 == CL.table2) )
			{
				CA[idx].tableAndCol1.push_back( CL.tableAndCol1[0] );
				CA[idx].tableAndCol2.push_back( CL.tableAndCol2[0] );
				CA[idx].connectionType.push_back( CL.connectionType[0] );
				found = true;
				break;
			}
      else if ( (CA[idx].table1 == CL.table2) && (CA[idx].table2 == CL.table1) )
			{
				CA[idx].tableAndCol1.push_back( CL.tableAndCol2[0] );
				CA[idx].tableAndCol2.push_back( CL.tableAndCol1[0] );
				CA[idx].connectionType.push_back( invertJoin(CL.connectionType[0]) );
				found = true;
				break;
			}
    }
		if( !found )
		{
			CL.lineId = static_cast<int>(CA.size());
			CA.push_back(CL);
		}
		CL = nextConnectionLine( inputString );
	}
}

//------------------------------------------------------------------------------
void getTablesListAndOccurence(std::vector<connectionLine>& connectionArray, 
                               std::map<std::string, int>& tablesDictionary )
{
	bool t1EQt2;

	for( size_t idx = 0; idx < connectionArray.size(); ++idx )
	{
		connectionLine& CL = connectionArray[idx];
		t1EQt2 = (CL.table1 == CL.table2);

		if(tablesDictionary.find(CL.table1) == tablesDictionary.end() )
			tablesDictionary[CL.table1] = 0;

		if(tablesDictionary.find(CL.table2) == tablesDictionary.end() )
			tablesDictionary[CL.table2] = 0;

		if (!t1EQt2)
		{
			tablesDictionary[CL.table1]++;
			tablesDictionary[CL.table2]++;
		}
	}
}

//------------------------------------------------------------------------------
void ReadJeq(std::string& inputString, 
             std::vector<connectionLine>& connectionArray, 
             std::map<std::string, int>& tablesListAndOccurrence)
{
	//recognize the different tables and conditions 'WHERE / K_JEQ / K_JINF ..' 
	getConnectionArray( inputString, connectionArray );

	//with the help of the connectionArray, fill a dictionary key:table / value:occurrence
	getTablesListAndOccurence( connectionArray, tablesListAndOccurrence );
}

//------------------------------------------------------------------------------
bool ContainsUnused( const std::vector<connectionLine>& connectionArray )
{
	for( size_t idx = 0; idx < connectionArray.size(); ++idx )
		if( !connectionArray[idx].isUsed )
			return true;
	return false;
}

//------------------------------------------------------------------------------
// getMinConnectionTable return a string with the name of the table which have the least connections,
// it helps to take the shorter way
std::string getMinConnectionTable(const std::vector<connectionLine>& possibleConnections, 
                                  std::map<std::string, int>& dictionary,
                                  std::string source)
{
	std::string minTable = "";
	int minValue = 0;
	bool first = true;

	// for each connectionLine in the list of possible connections,
	// we will find the destination table with the least connections but
	// if the source and the destination tables are the same, we take this connection
	for( size_t idx = 0; idx < possibleConnections.size(); ++idx )
	{
		const connectionLine& CL = possibleConnections[idx];
		if (CL.table1 == CL.table2)
		{
			minTable = CL.table1;
			break;
		}

		if (CL.table1 != source)
		{
			if( dictionary[CL.table1] < minValue || first )
			{
				minTable = CL.table1;
				minValue = dictionary[CL.table1];
				first = false;
			}
		}
		else if( dictionary[CL.table2] < minValue || first )
		{
			minTable = CL.table2;
			minValue = dictionary[CL.table2];
			first = false;
		}
	}

	return minTable;
}

//------------------------------------------------------------------------------
// getFirstTableName is used to take the first Source
std::string getFirstTableName(std::map<std::string, int>& dictionary)
{
	std::string firstTable = (*dictionary.begin()).first;
	int minValue = (*dictionary.begin()).second;

	for(std::map<std::string, int>::iterator idx = dictionary.begin(); idx != dictionary.end(); ++idx )
  {
		if ((*idx).second < minValue)
		{
			minValue = (*idx).second;
			firstTable = (*idx).first;
		}
  }

	return firstTable;
}

//------------------------------------------------------------------------------
void AssembleJeq(std::vector<connectionLine>& connectionArray,
                 std::map<std::string, int>& tablesListAndOccurrence,
                 std::vector<kjeqConnection>& orderedC )
{
  std::string source = "";
  std::string destination = "";
  int lineId = -1;
  kjeqConnection kC;

  // get the first 'Source' table
  source = getFirstTableName(tablesListAndOccurrence);

  // while some connections between tables haven't been used
  // we look if the source can reach a new destination
  // or we take the way back
  while( ContainsUnused( connectionArray ) )
  {
    std::vector<connectionLine> possibleConnections;
    for( size_t idx = 0; idx < connectionArray.size(); ++idx )
    {
      const connectionLine& line = connectionArray[idx];
      if( !line.isUsed && (line.table1 == source || line.table2 == source) ) 
        possibleConnections.push_back( line );
    }
    if( possibleConnections.size() > 0 )
    {
      //go forward
      destination = getMinConnectionTable(possibleConnections, tablesListAndOccurrence, source);
      for( size_t idx = 0; idx < possibleConnections.size(); ++idx )
      {
        connectionLine& line = possibleConnections[idx];
        if( (line.table1 == source && line.table2 == destination) || 
          (line.table2 == source && line.table1 == destination) )
        {
          lineId = line.lineId;
          for( size_t idx2 = 0; idx2 < connectionArray.size(); ++idx2 )
            if( connectionArray[idx2].lineId == lineId )
            {
              connectionArray[idx2].isUsed = true;
              break;
            }
            break;
        }
      }
    }
    else
    {
      //get back -> unpile first time source was a destination
      for( size_t idx = 0; idx < orderedC.size(); ++idx )
        if( orderedC[idx].tableDestination == source )
        {
          lineId = orderedC[idx].lineId;
          destination = orderedC[idx].tableSource;
          break;
        }
    }

    kC.lineId = lineId;
    kC.tableSource = source;
    kC.tableDestination = destination;

    orderedC.push_back(kC);

    source = destination;
  }
}

//------------------------------------------------------------------------------
connectionLine findConnection(const std::vector<connectionLine>& connectionArray, 
                              const kjeqConnection& kC ) 
{
	for( size_t idx = 0; idx < connectionArray.size(); ++idx )
		if( connectionArray[idx].lineId == kC.lineId )
			return connectionArray[idx];
	assert( 0 );
	return connectionLine();
}

//------------------------------------------------------------------------------
connectionLine invertConnection(const connectionLine& cL)
{
  connectionLine cli;
  for (auto& ct : cL.connectionType)
  {
    cli.connectionType.push_back(invertJoin(ct));
  }
  cli.isUsed = cL.isUsed;
  cli.lineId = cL.lineId;
  cli.table1 = cL.table2;
  cli.table1JoinType = cL.table2JoinType;
  cli.table2 = cL.table1;
  cli.table2JoinType = cL.table1JoinType;
  cli.tableAndCol1 = cL.tableAndCol2;
  cli.tableAndCol2 = cL.tableAndCol1;
  return cli;
}

//------------------------------------------------------------------------------
void WriteJeq(std::string& inputString,
              const std::vector<kjeqConnection>& orderedConnections,
              const std::vector<connectionLine>& connectionArray,
              bool add_active_neutral_filter)
{
	std::string andClause = "AND ";
	connectionLine cL;
	std::string output;

	// Step 1 : Add as many 'AND' as needed

	size_t currentNrCon = 0;
	for( size_t idx = 0; idx < connectionArray.size(); ++idx )
		currentNrCon += connectionArray[idx].tableAndCol1.size();
	size_t totalNrCon = 0;
	for( size_t idx = 0; idx < orderedConnections.size(); ++idx )
	{
		const kjeqConnection& kC = orderedConnections[idx];
		cL = findConnection( connectionArray, kC );
		totalNrCon += cL.tableAndCol1.size();
	}
	assert( currentNrCon <= totalNrCon );
	for( size_t idx = currentNrCon; idx < totalNrCon; ++idx )
		output += andClause;
  
	// Step 2 : add K_JEQ conditions in the right order

  output += '\n';
  bool table_prc_is_neutral = false;
  std::string table_src_prec;
  std::set<std::string> tables;
	for( size_t idx = 0; idx < orderedConnections.size(); ++idx )
	{
		const kjeqConnection& kC = orderedConnections[idx];
		cL = findConnection( connectionArray, kC );
    
    assert(((kC.tableSource == cL.table1) && (kC.tableDestination == cL.table2)) || ((kC.tableSource == cL.table2) && (kC.tableDestination == cL.table1)));
    bool invert = (kC.tableSource != cL.table2) && ((connectionArray.size() > 1) || (cL.connectionType.size() > 1));
    if (invert)
    {
      cL = invertConnection(cL);
    }

		for( size_t idx2 = 0; idx2 < cL.tableAndCol1.size(); ++idx2 )
		{
      std::string source, destination;
      
      if (add_active_neutral_filter)
        source = table_prc_is_neutral ? "K_NEUTRAL" : "K_ACTIVE";
      source += " " + cL.tableAndCol2[idx2];
      
      if (add_active_neutral_filter)
      {
        if (tables.find(cL.table1) != tables.end())
        {
          if (cL.table1 == table_src_prec)
          {
            destination = "K_NEUTRAL";
            table_prc_is_neutral = true;
          }
          else
          {
            destination = "K_FILTER ";
            table_prc_is_neutral = false;
          }
        }
        else
        {
          destination = "K_ACTIVE";
          table_prc_is_neutral = false;
        }
      }
      destination += " ";
      destination += cL.tableAndCol1[idx2];

      output += cL.connectionType[idx2] + " " + destination + " " + source + "\n";
		}

    table_src_prec = cL.table2;
    tables.insert(cL.table1);
    tables.insert(cL.table2);
	}

	// Step 3 : localize the conditions from the input request and delete them
	// each one is caught between the 'firstIndex' and the 'lastindex'
	
	std::string::size_type firstIndex = std::string::npos, lastIndex;
	for(;;) 
	{
		std::string aux;
		findJoin( inputString, aux, firstIndex );
		if( firstIndex == std::string::npos )
			break;

		lastIndex = firstIndex;
		lastIndex = inputString.find('.', lastIndex)+1;
		lastIndex = inputString.find('.', lastIndex);

		lastIndex = inputString.find(' ', lastIndex) + 1;
		lastIndex = inputString.find(' ', lastIndex) + 1;
		lastIndex = inputString.find(' ', lastIndex) + 1;
		if( lastIndex == std::string::npos )
			lastIndex = inputString.length();
		inputString = inputString.replace(firstIndex, lastIndex-firstIndex, "");
	};
	firstIndex = inputString.rfind( andClause );
	std::string whereClause = "WHERE ";
	if( firstIndex == std::string::npos )
		firstIndex = inputString.find( whereClause ) + whereClause.length();
	else
		firstIndex += andClause.length();

	inputString.insert( firstIndex, output );
}


void AddActiveNeutralFilter(std::string&)
{
}

}
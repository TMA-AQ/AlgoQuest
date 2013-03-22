#include "JeqParser.h"
#include "ID2Str.h"
#include <vector>
#include <map>
#include <cassert>
#include <boost/bind.hpp>
#include <boost/algorithm/string.hpp>
using namespace std;

extern const int nrJoinTypes;
extern const int joinTypes[];
extern const int inverseTypes[];

//------------------------------------------------------------------------------
struct connectionLine
{
	int lineId;
	string table1;
	string table2;
	vector<string> tableAndCol1;
	vector<string> tableAndCol2;
	bool isUsed;
	vector<string> connectionType;

	connectionLine(): isUsed(false){}
};

//------------------------------------------------------------------------------
struct kjeqConnection
{
	int lineId;
	string tableSource;
	string tableDestination;
};

//------------------------------------------------------------------------------
void ReadJeq(	std::string& inputString, 
				vector<connectionLine>& connectionArray, 
				map<string, int>& tablesListAndOccurrence );

void AssembleJeq(	vector<connectionLine>& connectionArray,
					map<string, int>& tablesListAndOccurrence,
					vector<kjeqConnection>& orderedC );
void WriteJeq(	string& inputString,
				const vector<kjeqConnection>& orderedConnections,
				const vector<connectionLine>& connectionArray );

void ParseJeq( std::string& inputString )
{
	vector<connectionLine> connectionArray;
	map<string, int> tablesListAndOccurrence;
	ReadJeq( inputString, connectionArray, tablesListAndOccurrence );
	if( connectionArray.size() == 0 )
		return;
	vector<kjeqConnection> orderedConnections;
	AssembleJeq( connectionArray, tablesListAndOccurrence, orderedConnections );
	WriteJeq( inputString, orderedConnections, connectionArray );
}

//------------------------------------------------------------------------------
void findJoin( const string& inputString, string& joinType, std::string::size_type& position )
{
	position = string::npos;
	for( int idx = 0; idx < nrJoinTypes; ++idx )
	{
		std::string::size_type pos = inputString.find( id_to_string( joinTypes[idx] ) );
		if( pos != string::npos && (pos < position || position == string::npos) )
		{
			position = pos;
			joinType = id_to_string( joinTypes[idx] );
		}
	}
}

//------------------------------------------------------------------------------
string invertJoin( string& join )
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
connectionLine nextConnectionLine( string& inputString )
{
	connectionLine CL;
	std::string::size_type nextPosition = std::string::npos;
	string connectionType;
	if( inputString.length() > 0 )
		findJoin( inputString, connectionType, nextPosition );

	if( nextPosition != std::string::npos )
		inputString = inputString.substr( nextPosition );
	else
		return CL;

	CL.connectionType.push_back( connectionType );

	inputString = inputString.substr( inputString.find(' ') + 1 );
	boost::algorithm::trim( inputString );

	nextPosition = inputString.find(' ') + 1;
	nextPosition = inputString.find(' ', nextPosition) + 1;
	nextPosition = inputString.find(' ', nextPosition);
	CL.table1 = inputString.substr(0, nextPosition);

	nextPosition = inputString.find(' ', nextPosition + 1);
	CL.tableAndCol1.push_back( inputString.substr(0, nextPosition) );
	boost::algorithm::trim( CL.tableAndCol1[0] );

	inputString = inputString.substr(nextPosition + 1);
	boost::algorithm::trim( inputString );

	nextPosition = inputString.find(' ') + 1;
	nextPosition = inputString.find(' ', nextPosition) + 1;
	nextPosition = inputString.find(' ', nextPosition);
	CL.table2 = inputString.substr(0, nextPosition);

	nextPosition = inputString.find(' ', nextPosition + 1);
	if (nextPosition == std::string::npos)         
		CL.tableAndCol2.push_back( inputString );
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
void getConnectionArray(	std::string inputString, 
							vector<connectionLine>& CA )
{                     
	connectionLine CL;
	CL = nextConnectionLine( inputString );
	while( CL.table1 != "" )
	{
		assert( CL.tableAndCol1.size() == 1 );
		assert( CL.tableAndCol2.size() == 1 );
		if( CL.table1 > CL.table2 )
		{
			swap( CL.table1, CL.table2 );
			swap( CL.tableAndCol1, CL.tableAndCol2 );
		}
		bool found = false;
		for( size_t idx = 0; idx < CA.size(); ++idx )
			if( CA[idx].table1 == CL.table1 && CA[idx].table2 == CL.table2 )
			{
				CA[idx].tableAndCol1.push_back( CL.tableAndCol1[0] );
				CA[idx].tableAndCol2.push_back( CL.tableAndCol2[0] );
				CA[idx].connectionType.push_back( CL.connectionType[0] );
				found = true;
				break;
			}
		if( !found )
		{
			CL.lineId = CA.size();
			CA.push_back(CL);
		}
		CL = nextConnectionLine( inputString );
	}
}

//------------------------------------------------------------------------------
void getTablesListAndOccurence(	vector<connectionLine>& connectionArray, 
								map<string, int>& tablesDictionary )
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
void ReadJeq(	std::string& inputString, 
				vector<connectionLine>& connectionArray, 
				map<string, int>& tablesListAndOccurrence )
{
	//recognize the different tables and conditions 'WHERE / K_JEQ / K_JINF ..' 
	getConnectionArray( inputString, connectionArray );

	//with the help of the connectionArray, fill a dictionary key:table / value:occurrence
	getTablesListAndOccurence( connectionArray, tablesListAndOccurrence );
}

/*
         * AssembleJeq will construct a table with the path to follow.
         * it uses the connectionArray
         * and stores the result as 'kjeqConnection' (lineId, tableSource, tableDestination) in a list : orederedConnections
         * 
         * ======================================================
         * |    lineId  |   tableSource |   tableDestination    |
         * ======================================================
         * |    2       |   person      |   car                 |
         * ------------------------------------------------------
         * |    3       |   car         |   brand               |
         * ------------------------------------------------------
         * |    0       |   brand       |   car                 |
         * ------------------------------------------------------
         * |    1       |   car         |   motor               |
         * ======================================================
         * 
         * the lineId represents the Id of the connectionLine in connectionsArray where tableSource and tableDestination are both presented
        */

//------------------------------------------------------------------------------
bool ContainsUnused( const vector<connectionLine>& connectionArray )
{
	for( size_t idx = 0; idx < connectionArray.size(); ++idx )
		if( !connectionArray[idx].isUsed )
			return true;
	return false;
}

//------------------------------------------------------------------------------
// getMinConnectionTable return a string with the name of the table which have the least connections,
// it helps to take the shorter way
string getMinConnectionTable(	const vector<connectionLine>& possibleConnections, 
								map<string, int>& dictionary,
								string source )
{
	string minTable = "";
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
string getFirstTableName(map<string, int>& dictionary)
{
	string firstTable = (*dictionary.begin()).first;
	int minValue = (*dictionary.begin()).second;

	for(map<string, int>::iterator idx = dictionary.begin(); idx != dictionary.end(); ++idx )
		if ((*idx).second < minValue)
		{
			minValue = (*idx).second;
			firstTable = (*idx).first;
		}

	return firstTable;
}

//------------------------------------------------------------------------------
void AssembleJeq(	vector<connectionLine>& connectionArray,
					map<string, int>& tablesListAndOccurrence,
					vector<kjeqConnection>& orderedC )
{
    string source = "";
    string destination = "";
    int lineId = -1;
    kjeqConnection kC;

    // get the first 'Source' table
    source = getFirstTableName(tablesListAndOccurrence);

    // while some connections between tables haven't been used
    // we look if the source can reach a new destination
    // or we take the way back
    while( ContainsUnused( connectionArray ) )
    {
		vector<connectionLine> possibleConnections;
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
connectionLine findConnection(	const vector<connectionLine>& connectionArray, 
								const kjeqConnection& kC ) 
{
	for( size_t idx = 0; idx < connectionArray.size(); ++idx )
		if( connectionArray[idx].lineId == kC.lineId )
			return connectionArray[idx];
	assert( 0 );
	return connectionLine();
}

//------------------------------------------------------------------------------
void WriteJeq(	string& inputString,
				const vector<kjeqConnection>& orderedConnections,
				const vector<connectionLine>& connectionArray )
{
	string andClause = "AND ";
	connectionLine cL;
	string output;

	// Step 1 : add K_JEQ conditions in the right order

	//Edit : Add as many 'AND' as needed
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

	for( size_t idx = 0; idx < orderedConnections.size(); ++idx )
	{
		const kjeqConnection& kC = orderedConnections[idx];
		cL = findConnection( connectionArray, kC );
		for( size_t idx2 = 0; idx2 < cL.tableAndCol1.size(); ++idx2 )
		{
			if (kC.tableSource != cL.table1)
			{
				output += cL.connectionType[idx2] + " ";
				output += cL.tableAndCol1[idx2];
				output += " ";
				output += cL.tableAndCol2[idx2];
			}
			else
			{
				output += invertJoin( cL.connectionType[idx2] ) + " ";
				output += cL.tableAndCol2[idx2];
				output += " ";
				output += cL.tableAndCol1[idx2];
			}
			output += ' ';
		}
	}

	// Step 2 : localize the conditions from the input request and delete them
	// each one is caught between the 'firstIndex' and the 'lastindex'
	
	std::string::size_type firstIndex = string::npos, lastIndex;
	for(;;) 
	{
		string aux;
		findJoin( inputString, aux, firstIndex );
		if( firstIndex == string::npos )
			break;

		lastIndex = firstIndex;
		lastIndex = inputString.find('.', lastIndex)+1;
		lastIndex = inputString.find('.', lastIndex);

		lastIndex = inputString.find(' ', lastIndex) + 1;
		lastIndex = inputString.find(' ', lastIndex) + 1;
		lastIndex = inputString.find(' ', lastIndex) + 1;
		if( lastIndex == string::npos )
			lastIndex = inputString.length();
		inputString = inputString.replace(firstIndex, lastIndex-firstIndex, "");
	};
	firstIndex = inputString.rfind( andClause );
	string whereClause = "WHERE ";
	if( firstIndex == string::npos )
		firstIndex = inputString.find( whereClause ) + whereClause.length();
	else
		firstIndex += andClause.length();

	inputString.insert( firstIndex, output );
}
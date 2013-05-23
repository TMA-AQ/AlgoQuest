//#include "statement_resolver.h"
//#include "TreeUtilities.h"
//#include "QueryResolver.h"
//#include <aq/DBTypes.h>
//#include <aq/Utilities.h>
//#include <aq/Exceptions.h>
//#include <boost/scoped_array.hpp>
//
//namespace aq
//{
//
////------------------------------------------------------------------------------
//Table::Ptr solve( aq::tnode* sqlStatement )
//{
//	if( !sqlStatement )
//		throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
//
//	switch( sqlStatement->tag )
//	{
//	case K_SELECT:
//		{
//#ifdef _DEBUG
//			std::string query;
//			// syntax_tree_to_sql_form(this->sqlStatement, query);
//			std::cerr << query << std::endl;
//#endif
//   //   QueryResolver queryResolver;
//			//return queryResolver.SolveSelect();
//		}
//		break;
//
// // case K_CREATE:
//	//	{
//	//		SolveCreate( pNode );
//	//	}
//	//	break;
//	//case K_INSERT:
//	//	{
//	//		SolveInsert( pNode );
//	//	}
//	//	break;
//	//case K_DELETE:
//	//case K_UPDATE:
//	//	{
//	//		SolveUpdateDelete( pNode );
//	//	}
//	//	break;
//	//case K_TRUNCATE:
//	//	{
//	//		SolveTruncate( pNode );
//	//	}
//	//	break;
//	//case K_SEL_MINUS:
//	//case K_UNION:
//	//	{
//	//		SolveUnionMinus( pNode );
//	//	}
//	//	break;
//
//	default:
//		throw aq::generic_error(aq::generic_error::NOT_IMPLEMENED, "");
//	}
//	return 0; //debug13 should throw instead of returning
//}
//
////------------------------------------------------------------------------------
//void addUnionMinusNode(int tag, std::vector<aq::tnode*>& queries, std::vector<int>& operation, aq::tnode* pNode,
//                       TProjectSettings * pSettings, AQEngine_Intf * aq_engine, Base& BaseDesc)
//{
//	assert( pNode );
//	if( tag == K_SEL_MINUS )
//	{
//		operation.push_back( 2 );
//		assert( pNode->tag == K_SELECT );
//		queries.push_back( pNode );
//	}
//	else
//	{
//		assert( tag == K_UNION );
//		if( pNode->tag == K_ALL )
//		{
//			operation.push_back( 1 );
//			assert( pNode->left );
//			assert( pNode->left->tag == K_SELECT );
//			queries.push_back( pNode->left );
//		}
//		else
//		{
//			operation.push_back( 0 );
//			assert( pNode->tag == K_SELECT );
//			queries.push_back( pNode );
//		}
//	}
//}
//
////------------------------------------------------------------------------------
//void SolveInsertAux(Table& table, size_t tableIdx, size_t colIdx, size_t packNumber,
//                    std::vector<size_t>& reverseValuePos,
//                    Column& nullColumn, Table& valuesToInsert, size_t startIdx, 
//                    size_t endIdx, bool append,
//                    TProjectSettings * pSettings, AQEngine_Intf * aq_engine, Base& BaseDesc)
//{
//  char szBuffer[256];
//	sprintf( szBuffer, "%s%sB001T%.4uC%.4uP%.12u", pSettings->szRootPath.c_str(), 
//		"data_orga\\columns\\", table.ID, table.Columns[colIdx]->ID, packNumber );
//	if( reverseValuePos[colIdx] < 0 )
//		nullColumn.saveToFile( szBuffer, startIdx, endIdx, append );
//	else
//	{
//		Column& col = *valuesToInsert.Columns[reverseValuePos[colIdx]];
//		col.saveToFile( szBuffer, startIdx, endIdx, append );
//	}
//
//	sprintf( szBuffer, "\"%s %s %u %u %u\"\n", 
//		pSettings->szLoaderPath,pSettings->iniFile.c_str(), tableIdx + 1, colIdx + 1, packNumber );
//	system ( szBuffer );
//}
//
////------------------------------------------------------------------------------
//void SolveInsert(aq::tnode* pNode,
//                 TProjectSettings * pSettings, AQEngine_Intf * aq_engine, Base& BaseDesc)
//{
//	if( !pNode || pNode->tag != K_INSERT )
//		return;
//	size_t tableIdx = BaseDesc.getTableIdx( pNode->left->getData().val_str );
//	std::vector<aq::tnode*> columns;
//	commaListToNodeArray( pNode->right->left, columns );
//	std::reverse( columns.begin(), columns.end() );
//	if( columns.size() == 0 )
//		throw aq::generic_error(aq::generic_error::GENERIC, "");
//
//	std::vector<int> valuePos;
//	Table& table = *BaseDesc.Tables[tableIdx];
//	aq::getColumnsIds( table, columns, valuePos );
//	
//	Table::Ptr valuesToInsert;
//	if( pNode->right->right->tag == K_SELECT )
//	{
//    unsigned int id_generator = 1;
//		QueryResolver queryResolver(pNode->right->right, pSettings, aq_engine, BaseDesc, id_generator);
//		queryResolver.solve();
//		valuesToInsert = queryResolver.getResult();
//		//check that the result is compatible with the insert columns
//		if( (valuesToInsert->Columns.size() - 1) != valuePos.size() )
//			throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
//		for( size_t idx = 0; idx < valuesToInsert->Columns.size()-1; ++idx )
//			if( !compatibleTypes(	valuesToInsert->Columns[idx]->Type,
//									table.Columns[valuePos[idx]]->Type) )
//				throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
//	}
//	else
//	{
//		std::vector<aq::tnode*> values;
//		commaListToNodeArray( pNode->right->right, values );
//		std::reverse( values.begin(), values.end() );
//		if( columns.size() != values.size() )
//			throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
//		//create table from values
//		valuesToInsert = new Table();
//		for( size_t idx = 0; idx < values.size(); ++idx )
//		{
//			Column::Ptr column = new Column( *table.Columns[valuePos[idx]] );
//			switch( column->Type )
//			{
//			case aq::COL_TYPE_INT:
//			case aq::COL_TYPE_BIG_INT:
//			case aq::COL_TYPE_DATE1:
//			case aq::COL_TYPE_DATE2:
//			case aq::COL_TYPE_DATE3:
//      case aq::COL_TYPE_DOUBLE:
//				column->Items.push_back( new ColumnItem(static_cast<double>(pNode->getData().val_int)) );
//				break;
//      case aq::COL_TYPE_VARCHAR:
//				column->Items.push_back( new ColumnItem(pNode->getData().val_str) );
//				break;
//			}
//			valuesToInsert->Columns.push_back( column );
//		}
//	}
//
//	int newRowsNr = (int) valuesToInsert->TotalCount;
//	int nrColumns = (int) table.Columns.size();
//	//write new base struct
//	table.TotalCount += newRowsNr;
//	BaseDesc.saveToRawFile( pSettings->szDBDescFN );
//
//	//write rows to table file
//	std::vector<size_t> reverseValuePos;
//	reverseValuePos.resize( nrColumns, -1 );
//	for( size_t idx = 0; idx < valuePos.size(); ++idx )
//		reverseValuePos[valuePos[idx]] = idx;
//
//	std::string tablePath = pSettings->szRootPath;
//	tablePath += "data_orga\\tables\\" + table.getOriginalName() + ".txt";
//	FILE* file = fopen(tablePath.c_str(), "a");
//	if( !file )
//		throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "");
//	
//  char szBuffer[256];
//	for( int idx = 0; idx < (int) valuesToInsert->Columns[0]->Items.size(); ++idx )
//	{
//		int nrRepetitions = 1;
//		if( valuesToInsert->HasCount )
//			nrRepetitions = (int) valuesToInsert->Columns[nrColumns - 1]->Items[idx]->numval;
//		for( int countIdx = 0; countIdx < nrRepetitions; ++countIdx )
//		{
//			std::string row;
//			for( int idx2 = 0; idx2 < nrColumns; ++idx2 )
//			{
//				if( pSettings->csvFormat )
//					row += "\"";
//				//if( reverseValuePos[idx2] < 0  ) // this is not possible
//				//	row += "NULL";
//				else
//				{
//					Column& column = *valuesToInsert->Columns[reverseValuePos[idx2]];
//					column.Items[idx]->toString( szBuffer, column.Type );
//					row += szBuffer;
//				}
//				if( pSettings->csvFormat )
//					row += "\"";
//				if( idx2 < nrColumns - 1)
//					if( pSettings->csvFormat )
//						row += ",";
//					else
//						row += ";";
//			}
//			row += "\n";
//			fputs( row.c_str(), file );
//		}
//	}
//	fclose( file );
//
//	//write values to column values and call cut_in_col
//	int lastOldPackSize = (table.TotalCount - newRowsNr) % pSettings->packSize;
//	int firstPackSize = pSettings->packSize - lastOldPackSize;
//	if( newRowsNr < firstPackSize )
//		firstPackSize = newRowsNr;
//	int packNumber = (int) (table.TotalCount - newRowsNr) / pSettings->packSize;
//	if( firstPackSize > 0 )
//	{
//		Column nullColumn;
//		nullColumn.Items.resize(firstPackSize, NULL);
//		for( int idx = 0; idx < nrColumns; ++idx )
//			SolveInsertAux( table, tableIdx, idx, packNumber, reverseValuePos, nullColumn, 
//				*valuesToInsert, 0, firstPackSize, true, pSettings, aq_engine,BaseDesc );
//		++packNumber;
//	}
//	
//	int leftOverRows = (int) newRowsNr - firstPackSize;
//	int nrPacks = leftOverRows / pSettings->packSize;
//	if( leftOverRows % pSettings->packSize > 0 )
//		++nrPacks;
//	Column nullColumn;
//	nullColumn.Items.resize(pSettings->packSize, NULL);
//	for( int idx = 0; idx < nrPacks; ++idx )
//	{
//		int firstIdx = firstPackSize + idx * pSettings->packSize;
//		int endIdx = firstIdx + pSettings->packSize;
//		if( idx == nrPacks - 1 )
//		{
//			int lastPackSize = table.TotalCount % pSettings->packSize;
//			nullColumn.Items.resize(lastPackSize, NULL);
//			endIdx = firstIdx + lastPackSize;
//		}
//		for( int idx2 = 0; idx2 < nrColumns; ++idx2 )
//			SolveInsertAux( table, tableIdx, idx2, packNumber + idx, reverseValuePos, 
//				nullColumn, *valuesToInsert, firstIdx, endIdx, false, pSettings, aq_engine,BaseDesc );
//	}
//}
//
////------------------------------------------------------------------------------
//void SolveUpdateDelete(aq::tnode* pNode,
//                       TProjectSettings * pSettings, AQEngine_Intf * aq_engine, Base& BaseDesc)
//{
//	size_t tableIdx = BaseDesc.getTableIdx( pNode->left->getData().val_str );
//	std::vector<Column::Ptr>& columns = BaseDesc.Tables[tableIdx]->Columns;
//	Table& table = *BaseDesc.Tables[tableIdx];
//
//	std::string tablePath = pSettings->szRootPath;
//	tablePath += "data_orga\\tables\\" + table.getOriginalName() + ".txt";
//	FILE* fOldTable = aq::fopenUTF8( tablePath.c_str(), "rt" );
//	if( fOldTable == NULL )
//		throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "");
//
//	std::string newTablePath = tablePath + "a"; //form unique table name
//	FILE* fNewTable = fopen( newTablePath.c_str(), "wt" );
//	if( fNewTable == NULL )
//		throw aq::generic_error(aq::generic_error::COULD_NOT_OPEN_FILE, "");
//
//	std::vector<aq::tnode*> updates;
//	std::vector<size_t> updateToTableMap;
//	std::vector<size_t> tableToUpdateMap;
//	if( pNode->tag == K_UPDATE )
//	{
//		commaListToNodeArray( pNode->right->left, updates );
//		updateToTableMap.resize( updates.size(), -1 );
//		tableToUpdateMap.resize( columns.size(), -1 );
//
//		for( size_t idx = 0; idx < updates.size(); ++idx )
//		{
//			std::string str(updates[idx]->left->getData().val_str);
//			aq::strtoupr( str );
//			for( size_t idx2 = 0; idx2 < columns.size(); ++idx2 )
//				if( str == columns[idx2]->getName() )
//				{
//					updateToTableMap[idx] = idx2;
//					tableToUpdateMap[idx2] = idx;
//
//					break;
//				}
//				if( updateToTableMap[idx] < 0 )
//					throw aq::generic_error( aq::generic_error::INVALID_QUERY, "" );
//		}
//	}
//	else
//	{	
//		for( size_t idx = 0; idx < columns.size(); ++idx )
//		{
//			updates.push_back( NULL );
//			updateToTableMap.push_back( idx );
//			tableToUpdateMap.push_back( idx );
//		}
//	}
//
//	char* myRecord = new char[pSettings->maxRecordSize];
//	boost::scoped_array<char> myRecordDel( myRecord );
//
//	Table::Ptr condTable = new Table();
//	aq::tnode* conditionsRoot;
//	if( pNode->tag == K_UPDATE )
//		conditionsRoot = pNode->right->right;
//	else
//		conditionsRoot = pNode->right;
//	if( conditionsRoot->tag == K_IN )
//	{
//		std::vector<int> valuePos;
//		std::vector<aq::tnode*> conditions;
//		commaListToNodeArray( conditionsRoot->left, conditions );
//		std::reverse( conditions.begin(), conditions.end() );
//		aq::getColumnsIds( table, conditions, valuePos );
//
//		assert( conditionsRoot->right->tag == K_SELECT );
//		
//    unsigned int id_generator = 1;
//		QueryResolver queryResolver(pNode->right->right, pSettings, aq_engine, BaseDesc, id_generator);
//		queryResolver.solve();
//		condTable = queryResolver.getResult();
//		if( !condTable || condTable->Columns.size() == 0 )
//			return; //no conditions
//		//check that the result is compatible with the columns
//		if( condTable->HasCount )
//			condTable->Columns.pop_back();
//		if( (condTable->Columns.size()) != valuePos.size() )
//			throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
//		for( size_t idx = 0; idx < condTable->Columns.size(); ++idx )
//		{
//			Column& col = *table.Columns[valuePos[idx]];
//			if( !compatibleTypes(	condTable->Columns[idx]->Type,
//									col.Type) )
//				throw aq::generic_error(aq::generic_error::INVALID_QUERY, "");
//			condTable->Columns[idx]->Type = col.Type;
//			condTable->Columns[idx]->ID = col.ID;
//		}
//	}
//	else
//	{
//		std::vector<aq::tnode*> conditions;
//		commaListToNodeArray( conditionsRoot, conditions );
//		for( size_t idx = 0; idx < conditions.size(); ++idx )
//		{
//			std::string str(conditions[idx]->left->getData().val_str);
//			aq::strtoupr( str );
//			bool found = false;
//			for( size_t idx2 = 0; idx2 < columns.size(); ++idx2 )
//				if( str == columns[idx2]->getName() )
//				{
//					Column::Ptr newCol = new Column(*columns[idx2]);
//
//					switch( newCol->Type )
//					{
//					case aq::COL_TYPE_INT:
//					case aq::COL_TYPE_BIG_INT:
//					case aq::COL_TYPE_DATE1:
//					case aq::COL_TYPE_DATE2:
//					case aq::COL_TYPE_DATE3:
//          case aq::COL_TYPE_DOUBLE:
//						newCol->Items.push_back( new ColumnItem(static_cast<double>(conditions[idx]->right->getData().val_int)) );
//						break;
//          case aq::COL_TYPE_VARCHAR:
//						newCol->Items.push_back( new ColumnItem(conditions[idx]->right->getData().val_str) );
//						break;
//					}
//
//					condTable->Columns.push_back( newCol );
//					found = true;
//					break;
//				}
//			if( !found )
//				throw aq::generic_error( aq::generic_error::INVALID_QUERY, "" );
//		}
//	}
//  
//  char szBuffer[256];
//	llong nrOfPacks = table.TotalCount / pSettings->packSize + 1;
//	if( table.TotalCount % pSettings->packSize == 0 )
//		++nrOfPacks;
//	for( size_t packNr = 0; packNr < (size_t) nrOfPacks; ++packNr )
//	{
//		std::vector<bool> updatedRows;
//		llong nrItems = pSettings->packSize;
//		if( packNr == nrOfPacks - 1 )
//		{
//			nrItems = BaseDesc.Tables[tableIdx]->TotalCount % pSettings->packSize;
//			if( nrItems == 0 )
//				nrItems = pSettings->packSize;
//		}
//		updatedRows.resize( (size_t) nrItems, true );
//		
//		for( size_t idx = 0; idx < condTable->Columns.size(); ++idx )
//		{
//			Column::Ptr condCol = condTable->Columns[idx];
//			Column::Ptr column( new Column(*condCol) );
//			std::string columnPath = pSettings->szRootPath;
//			sprintf( szBuffer, "B001T%.4uC%.4uP%.12u", table.ID, 
//				column->ID, packNr );
//			columnPath += "data_orga\\columns\\";
//			columnPath += szBuffer;
//			column->loadFromFile( columnPath );
//			for( size_t idx2 = 0; idx2 < column->Items.size(); ++idx2 )
//			{
//				bool found = false;
//				for( size_t idx3 = 0; idx3 < condCol->Items.size(); ++idx3 )
//					if( ColumnItem::equal(	column->Items[idx2].get(), 
//								condCol->Items[idx3].get(), 
//								column->Type) )
//					{
//						found = true;
//						break;
//					}
//				if( !found )
//					updatedRows[idx2] = false;
//			}
//		}
//		bool atLeastOneUpdate = false;
//		for( size_t idx = 0; idx < updatedRows.size(); ++idx )
//			if( updatedRows[idx] )
//			{
//				atLeastOneUpdate = true;
//				break;
//			}
//
//		if( atLeastOneUpdate )
//		{
//			for( size_t idx = 0; idx < updates.size(); ++idx )
//			{
//				Column::Ptr tColumn = table.Columns[updateToTableMap[idx]];
//				Column::Ptr column( new Column(*tColumn) );
//				std::string columnPath = pSettings->szRootPath;
//				sprintf( szBuffer, "B001T%.4uC%.4uP%.12u", table.ID, 
//					tColumn->ID, packNr );
//				columnPath += "data_orga\\columns\\";
//				columnPath += szBuffer;
//				aq::tnode* pValNode = NULL;
//				if( pNode->tag == K_UPDATE )
//					pValNode = updates[idx]->right;
//				column->loadFromFile( columnPath );
//				for( size_t idx2 = 0; idx2 < updatedRows.size(); ++idx2 )
//					if( updatedRows[idx2] )
//						if( pValNode )
//							switch( pValNode->getDataType() )
//						{
//              case aq::NODE_DATA_INT:
//								column->Items[idx2]->numval = (double) pValNode->getData().val_int;
//								break;
//							case aq::NODE_DATA_NUMBER:
//								column->Items[idx2]->numval = pValNode->getData().val_number;
//								break;
//							case aq::NODE_DATA_STRING:
//								column->Items[idx2]->strval = pValNode->getData().val_str;
//								break;
//							default:
//								assert( 0 );
//						}
//						else
//							column->Items[idx2] = NULL;
//				column->saveToFile( columnPath );
//				sprintf( szBuffer, "\"%s %s %d %d %u\"\n", 
//					pSettings->szLoaderPath, pSettings->iniFile.c_str(), tableIdx + 1, 
//					updateToTableMap[idx] + 1, packNr );
//				system ( szBuffer );
//			}
//		}
//
//		for( size_t idx = 0; idx < updatedRows.size(); ++idx )
//			if( updatedRows[idx] )
//			{
//				fgets( myRecord, pSettings->maxRecordSize, fOldTable );
//				std::vector<char*> fields;
//				aq::splitLine( myRecord, pSettings->fieldSeparator, fields, true );
//				std::string strval;
//				for( size_t idx2 = 0; idx2 < fields.size(); ++idx2 )
//				{
//					if( pNode->tag == K_DELETE )
//						fwrite( "NULL", sizeof(char), strlen("NULL"), fNewTable );
//					else if( tableToUpdateMap[idx2] < 0 )
//						fwrite( fields[idx2], sizeof(char), strlen(fields[idx2]), fNewTable );
//					else
//					{
//						std::string str = updates[tableToUpdateMap[idx2]]->right->to_string();
//						fwrite( str.c_str(), sizeof(char), str.size(), fNewTable );
//					}
//					if( idx2 + 1 < fields.size() )
//						fputc( ';', fNewTable );
//				}
//				fputc( '\n', fNewTable );
//			}
//			else
//			{
//				fgets( myRecord, pSettings->maxRecordSize, fOldTable );
//				fputs( myRecord, fNewTable );				
//			}
//	}
//	fclose( fOldTable );
//	fclose( fNewTable );
//
//	remove( tablePath.c_str() );
//	rename( newTablePath.c_str(), tablePath.c_str() );
//}
//
////------------------------------------------------------------------------------
//void SolveUnionMinus(aq::tnode* pNode,
//                     TProjectSettings * pSettings, AQEngine_Intf * aq_engine, Base& BaseDesc)
//{
//	std::vector<aq::tnode*> queries;
//	std::vector<int> operation;
//	int lastTag = -1;
//	while( pNode->tag == K_UNION || pNode->tag == K_SEL_MINUS )
//	{
//		aq::addUnionMinusNode( pNode->tag, queries, operation, pNode->right, pSettings, aq_engine,BaseDesc );
//		lastTag = pNode->tag;
//		pNode = pNode->left;
//	}
//	aq::addUnionMinusNode( lastTag, queries, operation, pNode, pSettings, aq_engine,BaseDesc );
//	reverse( queries.begin(), queries.end() );
//	reverse( operation.begin(), operation.end() );
//	Table::Ptr totalTable = NULL;
//	std::vector<size_t> deletedRows;
//	for( size_t idx = 0; idx < queries.size(); ++idx )
//	{
//    unsigned int id_generator = 1;
//		QueryResolver queryResolver(queries[idx], pSettings, aq_engine, BaseDesc, id_generator);
//		queryResolver.solve();
//		Table::Ptr table = queryResolver.getResult();
//		if( table->Columns.size() == 0 )
//			throw aq::generic_error(aq::generic_error::INVALID_TABLE, "");
//		if( totalTable )
//		{
//			//check for compatibility
//			if( totalTable->Columns.size() != table->Columns.size() )
//				throw aq::generic_error(aq::generic_error::INVALID_TABLE, "");
//			for( size_t idx2 = 0; idx2 < table->Columns.size(); ++idx2 )
//				if( !compatibleTypes( totalTable->Columns[idx2]->Type,
//					table->Columns[idx2]->Type) )
//					throw aq::generic_error(aq::generic_error::INVALID_TABLE, "");
//			size_t totalTableSize = totalTable->Columns[0]->Items.size();
//			if( operation[idx] == 1 ) //UNION ALL
//				totalTable->TotalCount += table->TotalCount;
//			for( size_t idx2 = 0; idx2 < table->Columns[0]->Items.size(); ++idx2 )
//			{
//				if( operation[idx] == 1 ) //UNION ALL
//				{
//					for( size_t idx3 = 0; idx3 < table->Columns.size(); ++idx3 )
//						totalTable->Columns[idx3]->Items.push_back( table->Columns[idx3]->Items[idx2] );
//				}
//				else
//				{
//					size_t nrColumns = table->Columns.size();
//					if( table->HasCount )
//						--nrColumns;
//					bool found = false;
//					for( size_t idx4 = 0; idx4 < totalTableSize; ++idx4 )
//					{
//						bool allEqual = true;
//						for( size_t idx3 = 0; idx3 < nrColumns; ++idx3 )
//							if( !ColumnItem::equal(	table->Columns[idx3]->Items[idx2].get(),
//										totalTable->Columns[idx3]->Items[idx4].get(), 
//										table->Columns[idx3]->Type ) )
//							{
//								allEqual = false;
//								break;
//							}
//						if( allEqual )
//						{
//							if( operation[idx] == 0 )
//							{
//								found = true;
//								break;
//							}
//							else
//							{
//								//MINUS
//								deletedRows.push_back( idx4 );
//								for( size_t idx3 = 0; idx3 < nrColumns; ++idx3 )
//                  totalTable->Columns[idx3]->Items[idx4] = NULL;
//							}
//						}
//					}
//					if( (operation[idx] == 0) && !found ) //UNION
//					{
//						if( table->HasCount )
//						{
//							size_t countCol = table->Columns.size()-1;
//							Column& count = *table->Columns[countCol];
//							ColumnItem& item = *count.Items[idx2];
//							totalTable->TotalCount += (llong) item.numval;
//						}
//						else
//							totalTable->TotalCount += 1;
//						for( size_t idx3 = 0; idx3 < table->Columns.size(); ++idx3 )
//						{
//							ColumnItem::Ptr item = table->Columns[idx3]->Items[idx2];
//							totalTable->Columns[idx3]->Items.push_back( item );
//						}
//					}
//				}
//			}
//			totalTable->groupBy();
//		}
//		else
//			totalTable = table;
//	}
//	totalTable->saveToAnswer(	pSettings->szAnswerFN, pSettings->fieldSeparator, deletedRows );
//};
//
////------------------------------------------------------------------------------
//void SolveTruncate(aq::tnode* pNode,
//                   TProjectSettings * pSettings, AQEngine_Intf * aq_engine, Base& BaseDesc)
//{
//	size_t tableIdx = -1;
//	Table::Ptr table = new Table();
//	table->setName( pNode->left->getData().val_str );
//	bool found = true;
//	for( size_t idx = 0; idx < BaseDesc.Tables.size(); ++idx )
//		if( table->getName() == BaseDesc.Tables[idx]->getName() )
//			tableIdx = idx;
//	if( tableIdx < 0 )
//		throw aq::generic_error(aq::generic_error::INVALID_TABLE, "");
//
//	BaseDesc.Tables[tableIdx]->TotalCount = 0;
//
//	//write to disk
//	BaseDesc.saveToRawFile( pSettings->szDBDescFN );
//
//	//delete table related files?
//	std::string tablePath = pSettings->szRootPath;
//	tablePath += "data_orga\\tables\\" + table->getOriginalName() + ".txt";
//	//..
//}
//
////------------------------------------------------------------------------------
//void SolveCreate(aq::tnode* pNode,
//                 TProjectSettings * pSettings, AQEngine_Intf * aq_engine, Base& BaseDesc)
//{
//	Table::Ptr table = new Table();
//	table->setName( pNode->left->getData().val_str );
//	for( size_t idx = 0; idx < BaseDesc.Tables.size(); ++idx )
//		if( table->getName() == BaseDesc.Tables[idx]->getName() )
//			throw aq::generic_error(aq::generic_error::TABLE_ALREADY_EXISTS, "");
//  
//  unsigned int id_generator = 1;
//	QueryResolver queryResolver( pNode->right, pSettings, aq_engine, BaseDesc, id_generator);
//	queryResolver.solve();
//	table = queryResolver.getResult();
//	assert( table );
//	size_t ID = 0;
//	if( BaseDesc.Tables.size() > 0 )
//	{
//		ID = BaseDesc.Tables[0]->ID;
//		for( size_t idx = 1; idx < BaseDesc.Tables.size(); ++idx )
//			ID = (std::max)(ID, (size_t) BaseDesc.Tables[idx]->ID);
//	}
//	++ID;
//	table->ID = ID;
//	table->setName( pNode->left->getData().val_str );
//	BaseDesc.Tables.push_back( table );
//	//write to disk
//	BaseDesc.saveToRawFile( pSettings->szDBDescFN );
//	std::string tablePath = pSettings->szRootPath;
//	tablePath += "data_orga\\tables\\" + table->getOriginalName() + ".txt";
//	table->saveToAnswer( tablePath.c_str(), pSettings->fieldSeparator, false );
//	//apply cut in col on the new columns
//	size_t nrColumns = table->Columns.size();
//	if( table->HasCount )
//		--nrColumns;
//	for( size_t idx = 0; idx < nrColumns; ++idx )
//	{
//    // FIXME
//		//sprintf( szBuffer, "cmd /s /c \"%s %s \"%u\" \"%u\"\"", pSettings->szCutInColPath, pSettings->iniFile.c_str(), BaseDesc.Tables.size(), idx + 1 ); //debug13 - not portable
//		//system( szBuffer );
//	}
//}
//
//}

using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Data;

namespace AlgoQuest.Core.Import
{
    public interface IConnector : IDisposable
    {
        void Initialize(string connectionString);

        void TestConnection();

        List<string> GetTableList(string request);

        List<DataImportColumn> GetTableDataColumnList(string request);

        string GetBaseName();

        string ConnectionString();

        UInt64 GetCount(string tableName);

    }
}

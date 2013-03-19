using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.IO;
using System.Data.Odbc;


namespace AlgoQuest.Core.Import
{
    public class OracleConnector : IConnector, IDisposable
    {
        private string _connectionString;

        private string _baseName = string.Empty;

        public void Initialize(string connectionString)
        {
            _connectionString = connectionString;
        }

        public string ConnectionString()
        {
            return _connectionString;
        }

        public void TestConnection()
        {
            using (OdbcConnection conn = new OdbcConnection(_connectionString))
            {
                conn.Open();
                conn.Close();
            }
        }

        public List<string> GetTableList(string request)
        {
            List<string> tableList = new List<string>();
            using (OdbcConnection conn = new OdbcConnection(_connectionString))
            {
                conn.Open();
                request = String.Format(request, conn.Database);
                OdbcCommand command = new OdbcCommand(request, conn);

                OdbcDataReader dr = command.ExecuteReader();

                while (dr.Read())
                {
                    string tableName = dr[0].ToString();
                    tableList.Add(tableName);
                }
                dr.Close();
                dr.Dispose();
                command.Dispose();
                conn.Close();
            }
            return tableList;
        }

        public UInt64 GetCount(string tableName)
        {
            UInt64 count;
            using (OdbcConnection connCount = new OdbcConnection(_connectionString))
            {
                OdbcCommand commandCount = new OdbcCommand(String.Format("SELECT count(*) FROM {0}", tableName), connCount);
                commandCount.CommandTimeout = 300;
                connCount.Open();
                count = UInt64.Parse(commandCount.ExecuteScalar().ToString());
                commandCount.Dispose();
                connCount.Close();
            }
            return count;
        }

        public List<DataImportColumn> GetTableDataColumnList(string request)
        {
            List<DataImportColumn> lstDc = new List<DataImportColumn>();
            using (OdbcConnection conn = new OdbcConnection(_connectionString))
            {
                OdbcCommand command = new OdbcCommand(request, conn);
                conn.Open();
                OdbcDataReader dr = command.ExecuteReader();

                while (dr.Read())
                {
                    lstDc.Add(new DataImportColumn()
                    {
                        Name = dr[0].ToString()
                        ,
                        DataType = dr[3].ToString()
                        ,
                        Size = dr[2].ToString()
                    });
                }
                dr.Close();
                dr.Dispose();
                command.Dispose();
                conn.Close();
            }
            return lstDc;
        }

       
        public string GetBaseName()
        {
            using (OdbcConnection conn = new OdbcConnection(_connectionString))
            {
                conn.Open();
                _baseName = conn.Database;
                conn.Close();
            }
            return _baseName;
        }

        public void Dispose()
        {
            
        }
    }
}

using System;
using System.Collections.Generic;
using System.Data;
using System.Data.SqlClient;
using System.Linq;
using System.Text;
using System.IO;

namespace AlgoQuest.Core.Import
{
    public class SqlConnector : IConnector, IDisposable
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


        public List<string> GetTableList(string request)
        {
            List<string> tableList = new List<string>();
            using (SqlConnection conn = new SqlConnection(_connectionString))
            {
                conn.Open();
                request = String.Format(request, conn.Database);
                SqlCommand command = new SqlCommand(request, conn);

                SqlDataReader dr = command.ExecuteReader();

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
            using (SqlConnection connCount = new SqlConnection(_connectionString))
            {
                SqlCommand commandCount = new SqlCommand(String.Format("SELECT count(*) FROM {0}", tableName), connCount);
                connCount.Open();
                commandCount.CommandTimeout = 300;
                count = UInt64.Parse(commandCount.ExecuteScalar().ToString());
                commandCount.Dispose();
                connCount.Close();
            }
            return count;
        }

        public List<DataImportColumn> GetTableDataColumnList(string request)
        {
            List<DataImportColumn> lstDc = new List<DataImportColumn>();
            using (SqlConnection conn = new SqlConnection(_connectionString))
            {
                SqlCommand command = new SqlCommand(request, conn);
                conn.Open();
                SqlDataReader dr = command.ExecuteReader();

                while (dr.Read())
                {
                    lstDc.Add(new DataImportColumn()
                    {
                        Name = dr[0].ToString()
                        , DataType = dr[3].ToString()
                        , Size = dr[2].ToString()
                    });
                }
                dr.Close();
                dr.Dispose();
                command.Dispose();
                conn.Close();
            }
            return lstDc;
        }

        public void TestConnection()
        {
            using (SqlConnection conn = new SqlConnection(_connectionString))
            {
                conn.Open();
                conn.Close();
            }
        }

        public string GetBaseName()
        {
            using (SqlConnection conn = new SqlConnection(_connectionString))
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

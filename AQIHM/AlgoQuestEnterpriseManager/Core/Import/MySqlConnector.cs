using System;
using System.Collections.Generic;
using System.Data;
using System.Data.OleDb;
using System.Linq;
using System.Text;
using MySql.Data.MySqlClient;
using System.IO;

namespace AlgoQuest.Core.Import
{
    public class MySqlConnector : IConnector, IDisposable
    {

        private string _connectionString;

        private string _baseName = string.Empty;

        public void Initialize(string connectionString)
        {
            _connectionString = connectionString;
        }

        public void TestConnection()
        {
            using (MySqlConnection conn = new MySqlConnection(_connectionString))
            {
                conn.Open();
                conn.Close();
            }
        }

        public string ConnectionString()
        {
            return _connectionString;
        }

        public List<string> GetTableList(string request)
        {
            //List<KeyValuePair<string, UInt64>> tableList = new List<KeyValuePair<string, UInt64>>();
            List<string> tableList = new List<string>();
            using (MySqlConnection conn = new MySqlConnection(_connectionString))
            {
                conn.Open();
                request = String.Format(request, conn.Database);
                MySqlCommand command = new MySqlCommand(request, conn);
                
                MySqlDataReader dr = command.ExecuteReader();

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
            using (MySqlConnection connCount = new MySqlConnection(_connectionString))
            {
                MySqlCommand commandCount = new MySqlCommand(String.Format("SELECT count(*) FROM {0}", tableName), connCount);
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
            using (MySqlConnection conn = new MySqlConnection(_connectionString))
            {
                MySqlCommand command = new MySqlCommand(request, conn);
                conn.Open();
                MySqlDataReader dr = command.ExecuteReader();

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
            using (MySqlConnection conn = new MySqlConnection(_connectionString))
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

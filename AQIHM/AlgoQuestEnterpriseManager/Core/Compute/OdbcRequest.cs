using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data.Odbc;
using System.Data;

namespace AlgoQuest.Core.Compute
{
    public class OdbcRequest : IDisposable, ISelectRequest
    {
        private string _connectionString;
        private int _nbRecords;

        public OdbcRequest(string connectionString)
        {
            _connectionString = connectionString;
            _nbRecords = 0;
        }

        public string PathFile { get; set; }

        public int NbRecords
        {
            get
            {
                return _nbRecords;
            }
        }

        public DataTable Execute(string query)
        {
            DataTable dt = new DataTable("Results");
            using (OdbcConnection conn = new OdbcConnection(_connectionString))
            {
                conn.Open();

                OdbcCommand command = new OdbcCommand(query.Replace("\n", " "), conn);
                OdbcDataReader dr = command.ExecuteReader();
                for (int i = 0; i < dr.FieldCount; i++)
                {
                    dt.Columns.Add("");
                }
                while (dr.Read())
                {
                    DataRow row = dt.NewRow();
                    for (int i = 0; i < dr.FieldCount; i++)
                    {
                        string s = dr[i].ToString();
                        row[i] = dr[i].ToString();
                    }
                    _nbRecords++;
                    dt.Rows.Add(row);
                }
                dr.Close();
                dr.Dispose();
                command.Dispose();

                conn.Close();
            }

            return dt;
        }

        public void ExecuteTest(string query)
        {
        }

        public void Show()
        {
            using (OdbcConnection conn = new OdbcConnection(_connectionString))
            {
                conn.Open();
                conn.Close();
            }
        }

        public void Dispose()
        {
        }
    }
}

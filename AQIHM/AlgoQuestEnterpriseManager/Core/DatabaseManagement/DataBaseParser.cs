using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Net.Sockets;
using System.Xml;
using System.Data.Odbc;

namespace AlgoQuest.Core.DatabaseManagement
{
    public class DataBaseParser : IDisposable
    {

        public void fill(TreeNode node, string odbcConnStr)
        {
            using (OdbcConnection conn = new OdbcConnection(odbcConnStr))
            {
                conn.Open();
                System.Data.DataTable tableschema = conn.GetSchema(OdbcMetaDataCollectionNames.Tables);
                foreach (System.Data.DataRow row in tableschema.Rows)
                {
                    string tn = row["TABLE_NAME"].ToString();
                    TreeNode trDt = node.Nodes.Add(tn, tn, 2, 2);
                    foreach (System.Data.DataColumn col in tableschema.Columns)
                    {
                        tn = row[col].ToString();
                        string cn = col.ColumnName;
                    }
                }
                conn.Close();
            }
        }

        public void fill(TreeNode node, string server, UInt16 port, string database)
        {
            Byte[] data;
            String responseData = String.Empty;
            Int32 bytes = 0;
            TcpClient client = null;
            NetworkStream stream = null;

            try
            {

                // connect to server
                client = new TcpClient(server, port);
                stream = client.GetStream();
                data = System.Text.Encoding.ASCII.GetBytes("connect " + database + "\n");
                stream.Write(data, 0, data.Length);

                // send request
                data = System.Text.Encoding.ASCII.GetBytes("desc\n");
                stream.Write(data, 0, data.Length);

                // read response
                data = new Byte[256];
                stream.ReadTimeout = 1; // FIXME
                do
                {
                    bytes = stream.Read(data, 0, data.Length);
                    responseData += System.Text.Encoding.ASCII.GetString(data, 0, bytes);

                    if (responseData.IndexOf("</Database>") != -1) break;
                } while (bytes != 0);
            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: {0}", ex.Message);
            }
            finally
            {
                // close
                if (stream != null)
                    stream.Close();
                if (client != null)
                    client.Close();
            }

            int pos = responseData.IndexOf("<Database");
            responseData = responseData.Substring(pos);

            // Parse ResponseData
            XmlDocument doc = new XmlDocument();
            doc.LoadXml(responseData);

            XmlNode tableListNode = doc.SelectSingleNode("//Tables");
            XmlNodeList tableList = tableListNode.SelectNodes("Table");
            foreach (XmlNode table in tableList)
            {
                TreeNode trDt = node.Nodes.Add(table.Attributes["Name"].Value, table.Attributes["Name"].Value, 2, 2);
                XmlNode columnListNode = table.SelectSingleNode("//Columns");
                XmlNodeList columnList = columnListNode.SelectNodes("Column");
                foreach (XmlNode column in columnList)
                {
                    trDt.Nodes.Add(column.Attributes["Name"].Value
                        , String.Format("{0} ({1}, {2})", column.Attributes["Name"].Value, column.Attributes["Type"].Value, column.Attributes["Size"].Value)
                        , 3
                        , 3);
                }
            }
        }

        public void Dispose()
        {
        }

    }
}

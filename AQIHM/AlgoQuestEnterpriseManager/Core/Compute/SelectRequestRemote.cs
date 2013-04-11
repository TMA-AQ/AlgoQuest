using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data;
using System.Net.Sockets;

namespace AlgoQuest.Core.Compute
{
    class SelectRequestRemote : ISelectRequest
    {
        private string _server;
        private string _database;
        private UInt16 _port;
        private int _nbRecordsToPrint;
        int _nbRecords;

        public SelectRequestRemote(string server, UInt16 port, string database, int nbRecordsToPrint)
        {
            this._server = server;
            this._database = database;
            this._port = port;
            this._nbRecordsToPrint = nbRecordsToPrint;
            this._nbRecords = 0;
        }


        public string PathFile
        {
            get;
            set;
        }

        public int NbRecords
        {
            get
            {
                return _nbRecords;
            }
        }

        public DataTable Execute(string request)
        {
            Byte[] data;
            String responseData = String.Empty;
            AnswerParser parser = new AnswerParser();
            Int32 bytes = 0;
            DataTable dt = null;
            TcpClient client = null;
            NetworkStream stream = null;
            request = request.Replace("\n", " ");

            try
            {

                // connect to server
                client = new TcpClient(_server, _port);
                stream = client.GetStream();
                data = System.Text.Encoding.ASCII.GetBytes("connect " + _database + "\n");
                stream.Write(data, 0, data.Length);

                // send request
                data = System.Text.Encoding.ASCII.GetBytes("execute " + request + "\n");
                stream.Write(data, 0, data.Length);

                // read response
                data = new Byte[256];
                do
                {
                    bytes = stream.Read(data, 0, data.Length);
                    responseData = System.Text.Encoding.ASCII.GetString(data, 0, bytes);
                    parser.push(responseData);
                } while (!parser.eos());
                dt = parser.getResult();
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

            return dt;
        }

        public void ExecuteTest(string request)
        {
        }
    }
}

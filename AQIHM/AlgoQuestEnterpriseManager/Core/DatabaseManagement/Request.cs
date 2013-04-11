using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Xml;

namespace AlgoQuest.Core.DatabaseManagement
{
    public class Request
    {
        private string _dbName;
        private string _dbPath;
        private string _requestPath;
        public Request(string dbPath, string dbName)
        {
            _dbName = dbName;
            _dbPath = Path.Combine(Path.GetDirectoryName(dbPath), _dbName);
            _requestPath = Path.Combine(_dbPath, "Requete.xml");
        }

        public string LoadRequest(string requestName)
        {
            string request = string.Empty;

            return request;
        }

        public void SaveRequest(RequestData rd)
        {
            XmlDocument xmlRequest = new XmlDocument();
            if (File.Exists(_requestPath))
                xmlRequest.Load(_requestPath);
            else
            {
                xmlRequest.LoadXml("<AlgoQuest></AlgoQuest>");
                xmlRequest.Save(_requestPath);
            }
            XmlNodeList nodeList = xmlRequest.SelectNodes(String.Format("//Request[Name='{0}']",rd.Name));
            if (nodeList.Count == 1)
            {
                ((XmlElement)nodeList[0]).GetElementsByTagName("SQL")[0].InnerText = rd.Content;
                ((XmlElement)nodeList[0]).GetElementsByTagName("Comment")[0].InnerText = rd.Comment;
            }
            else
            {
                XmlElement elemRequest = xmlRequest.CreateElement("Request");
                XmlElement elemName = xmlRequest.CreateElement("Name");
                elemName.InnerText = rd.Name;
                XmlElement elemSql = xmlRequest.CreateElement("SQL");
                elemSql.InnerText = rd.Content;
                XmlElement elemComment = xmlRequest.CreateElement("Comment");
                elemComment.InnerText = rd.Comment;

                elemRequest.AppendChild(elemName);
                elemRequest.AppendChild(elemSql);
                elemRequest.AppendChild(elemComment);
                xmlRequest.DocumentElement.AppendChild(elemRequest);
            }
            xmlRequest.Save(_requestPath);
        }

        public List<RequestData> GetRequest()
        {
            List<RequestData> requestList = new List<RequestData>(); ;

            XmlDocument xmlRequest = new XmlDocument();
            if (File.Exists(_requestPath))
                xmlRequest.Load(_requestPath);
            else
            {
                try
                {
                    xmlRequest.LoadXml("<AlgoQuest></AlgoQuest>");
                    xmlRequest.Save(_requestPath);
                }
                catch (IOException)
                {
                }
            }

            XmlNodeList xmlRequestList = xmlRequest.SelectNodes("//Request");

            foreach (XmlNode node in xmlRequestList)
            {
                RequestData rd = new RequestData();
                rd.Name = ((XmlElement)node).GetElementsByTagName("Name")[0].InnerText;
                rd.Content = ((XmlElement)node).GetElementsByTagName("SQL")[0].InnerText;
                rd.Comment = ((XmlElement)node).GetElementsByTagName("Comment")[0].InnerText;
                requestList.Add(rd);
            }

            return requestList;
        }
    }
    public class RequestData
    {
        public string Name { get; set; }
        public string Content { get; set; }
        public string Comment { get; set; }

        public override string ToString()
        {
            return Name;
        }
    }
}

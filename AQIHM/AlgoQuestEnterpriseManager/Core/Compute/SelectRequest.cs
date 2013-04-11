using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using AlgoQuest.Configuration.Core;
using System.Diagnostics;
using System.Data;
using System.Runtime.InteropServices;

namespace AlgoQuest.Core.Compute
{

    public class SelectRequest : ISelectRequest
    {
        [DllImport("E:/Project_AQ/AQSuite/x64/Release/AQLib.dll", CharSet = CharSet.Auto)]
        public static extern int solve_query(
            string query, string iniFilename, string workingDirectory, 
            string logIdent, string logMode, uint logLevel,
            bool clean, bool force);

        // [DllImport("E:\\Project_AQ\\AQSuite\\x64\\Release\\AQLib.dll", CharSet = CharSet.Auto)]
        // public static extern int test_aq_lib();

        // [DllImport("C:\\Users\\AlgoQuest\\Documents\\Visual Studio 2012\\Projects\\DLLTest\\Debug\\DLLTest.dll", CharSet = CharSet.Auto)]
        [DllImport("C:/Users/AlgoQuest/Documents/Visual Studio 2012/Projects/DLLTest/x64/Debug/DLLTest.dll")]
        public static extern int test_aq_lib();

        string _cfgPath;
        IniProperties _ip;
        string _tmpFolder;
        string _rootFolder;
        string _randomIdFolder;
        int _nbRecordsToPrint;
        int _nbRecords;

        public SelectRequest(string cfgPath, string database, int nbRecordsToPrint)
        {
            _cfgPath = cfgPath;
            _ip = new IniProperties(_cfgPath, database);
            _tmpFolder = _ip.Keys["tmp-folder"].ToString();
            _rootFolder = _ip.Keys["root-folder"].ToString();
            _nbRecordsToPrint = nbRecordsToPrint;
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

        private string getRandomIdFolder()
        {
            Random rd = new Random();
            string randomIdFolder = string.Empty;
            
            StreamWriter sw;
                        
            randomIdFolder = rd.Next().ToString();
           
            while (Directory.Exists(Path.Combine(_tmpFolder, randomIdFolder)))
                randomIdFolder = rd.Next().ToString();

            Directory.CreateDirectory(Path.Combine(_tmpFolder, randomIdFolder));
            Directory.CreateDirectory(Path.Combine(_tmpFolder, randomIdFolder, "dpy"));
            Directory.CreateDirectory(Path.Combine(_rootFolder, "Calculus", randomIdFolder));

            sw = new StreamWriter(Path.Combine(_rootFolder, "Calculus", randomIdFolder, "request.txt"));
            sw.Close();
            sw = new StreamWriter(Path.Combine(_rootFolder, "Calculus", randomIdFolder, "New_request.txt"));
            sw.Close();
            sw = new StreamWriter(Path.Combine(_rootFolder, "Calculus", randomIdFolder, "Answer.txt"));
            sw.Close();
            
            return randomIdFolder;
        }

        private void writeRequest(string request)
        {
            StreamWriter sw = new StreamWriter(Path.Combine(_rootFolder,"Calculus", _randomIdFolder,"request.txt"));
            sw.Write(request);
            sw.Close();
        }

        private DataTable getResult()
        {
            DataTable dt = new DataTable("results");
            PathFile = Path.Combine(_rootFolder, "Calculus", _randomIdFolder, "Answer.txt");
            StreamReader sr = new StreamReader(PathFile);
            string entete = sr.ReadLine();
            string columnName;
            try
            {
                string[] strColumns = entete.Split(new Char[] { ';' });
                for (int i = 1; i < strColumns.Length; i++)
                {
                    columnName = strColumns[i];
                    if (columnName.Trim() == string.Empty)
                        columnName = "(Column " + i.ToString() + ")";
                    dt.Columns.Add(columnName);
                }
                string[] strData;
                _nbRecords = 0;
                while (sr.Peek() > -1)
                {
                    strData = sr.ReadLine().Split(new Char[] { ';' });
                    if (strData.Length != strColumns.Length) continue;
                    if (_nbRecords < _nbRecordsToPrint)
                    {
                        DataRow row = dt.NewRow();
                        for (int j = 0; j < strData.Length - 1; j++)
                        {
                            row[j] = strData[j + 1];
                        }
                        dt.Rows.Add(row);
                    }
                    _nbRecords += 1;
                }
            }
            catch { throw new ApplicationException("Le format de la requête est incorrect."); }
            finally{
            sr.Close();
            sr.Dispose();
            }
            return dt;
        }

        private void clearTempFolder()
        {
            try
            {
                if (Directory.Exists(_ip.Keys["tmp-folder"] + "\\" + _randomIdFolder))
                    Directory.Delete(_ip.Keys["tmp-folder"] + "\\" + _randomIdFolder, true);
            }
            catch { }

        }

        public DataTable Execute(string request)
        {

            _randomIdFolder = getRandomIdFolder();
            writeRequest(request);
            
            string query = request;
            string iniFilename = _ip.getFilename();
            string workingDirectory = _randomIdFolder;
            string logIdent = "aq_query_resolver";
            string logMode = "LOCALFILE";
            uint logLevel = 7;
            bool clean = false;
            bool force = false;

            int rc = test_aq_lib();
            rc = solve_query(request, _ip.getFilename(), _randomIdFolder, logIdent, logMode, logLevel, clean, force);

            //ProcessStartInfo psi = new ProcessStartInfo(_ip.Keys["aq-tools"].ToString()
            //                             , _ip.getFilename() + " " + _randomIdFolder);
            //psi.RedirectStandardOutput = true;
            //psi.UseShellExecute = false;
            //psi.CreateNoWindow = true;
            //Process p = Process.Start(psi);
            //p.WaitForExit();

            clearTempFolder();

            return getResult();
        }

        public void ExecuteTest(string request)
        {
            _randomIdFolder = getRandomIdFolder();
            writeRequest(request);
            ProcessStartInfo psi = new ProcessStartInfo(_ip.Keys["aq-tools"].ToString()
                                         , _ip.getFilename() + " " + _randomIdFolder);

            psi.RedirectStandardOutput = true;
            psi.UseShellExecute = false;
            psi.CreateNoWindow = true;

            Process p = Process.Start(psi);

            p.WaitForExit();
            clearTempFolder();
        }
    }
}

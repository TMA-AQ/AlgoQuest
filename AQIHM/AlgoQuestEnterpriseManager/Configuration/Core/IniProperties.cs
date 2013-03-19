using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;
using System.IO;
using System.Collections;

namespace AlgoQuest.Configuration.Core
{
    public class IniProperties
    {
        [DllImport("kernel32")]
        private static extern long WritePrivateProfileString(string section, string key, string val, string filePath);
        [DllImport("kernel32")]
        private static extern int GetPrivateProfileString(string section, string key, string def, StringBuilder retVal, int size, string filePath);

        private string _iniPath;
        private Hashtable _keys = new Hashtable();

        public Hashtable Keys { get { return _keys; }   set { _keys = value; }  }

        private const string newline = "\r\n";

        public IniProperties(string iniPath)
        {
            _iniPath = iniPath;
            load();
        }

        public IniProperties(string cfgPath, string database)
        {
            _iniPath = Path.Combine(Path.GetDirectoryName(cfgPath), database + ".ini");
            load();
        }

        public string getFilename()
        {
            return _iniPath;
        }

        private void load()
        {
            StreamReader sr;
            if (!File.Exists(_iniPath))
                sr = new StreamReader(File.Open(_iniPath, FileMode.OpenOrCreate));
            else
                sr = new StreamReader(_iniPath);
            string file = sr.ReadToEnd();
            string[] lines = file.Split('\r', '\n');
            string currentSection = string.Empty;

            for (int i = 0; i < lines.Length; i++)
            {
                string line = lines[i];
                if (line!=string.Empty)
                {
                    char[] ca = new char[1] { '=' };
                    string[] scts = line.Split(ca, 2);
                    _keys.Add(scts[0],scts[1]);
                }
            }
            sr.Close();
        }

        public void Save()
        {
            StreamWriter str = new StreamWriter(_iniPath, false);

            foreach (DictionaryEntry de  in _keys)
            {
                str.Write((string)de.Key + "=" + (string)de.Value + newline);
            }
            
            str.Flush();
            str.Close();
        }
    }
}

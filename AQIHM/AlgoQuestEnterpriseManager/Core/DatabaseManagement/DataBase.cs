using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Xml;
using Microsoft.Win32;

namespace AlgoQuest.Core.DatabaseManagement
{
    
    public class DataBase
    {
        private string _name;

        public string Name
        {
            get { return _name; }
            set { _name = value; }
        }

        private string _path;
        private string _cfgPath;

        private string _baseStructPathRaw;
        private string _baseStructPathXml;

        private enum BaseDesc_t
        {
            RAW,
            XML
        }

        private BaseDesc_t base_desc_type;
        private List<DataTable> _datatableList;
        private StreamReader _srProperties;
        private XmlDocument _xmlReader;
        private int _tablecount;
        private string _baseStruct;

        public List<DataTable> DataTableList
        {
            set { _datatableList = value; }
            get
            {
                if (_datatableList == null)
                    getDataTableList();
                
                return _datatableList;
            }
        }

        public int TableCount
        {
            get { return _tablecount; }
        }

        public string BaseStruct
        {
            get
            {
                if (string.IsNullOrEmpty(_baseStruct))
                    readBaseStruct();
                return _baseStruct;
            }
            set
            {
                writeBaseStruct(value);
            }
        }

        public string DataOrgaPath
        {
            get
            {
                return Path.Combine(_path, "data_orga");
            }
        }

        public string DataOrgaTablesPath
        {
            get
            {
                if (!Directory.Exists(Path.Combine(DataOrgaPath, "tables")))
                    Directory.CreateDirectory(Path.Combine(DataOrgaPath, "tables"));
                return Path.Combine(DataOrgaPath, "tables");
            }
        }

        public string IniPropertiesPath
        {
            get
            {
                return Path.Combine(_cfgPath, _name + ".ini");
            }
        }

        private void writeBaseStruct(string value)
        {
            StreamWriter sw = new StreamWriter(_baseStructPathRaw, false);
            sw.Write(value);
            sw.Flush();
            sw.Close();
            sw.Dispose();
        }

        private void readBaseStruct()
        {
            StreamReader sr = new StreamReader(_baseStructPathRaw);
            _baseStruct = sr.ReadToEnd();
            sr.Close();
            sr.Dispose();
        }



        

        public static List<DataBase> GetDatabases(string dbPath, string cfgPath)
        {
            List<DataBase> lstDb = new List<DataBase>();

            string[] allDb = Directory.GetDirectories(dbPath);

            for (int i = 0; i < allDb.Length; i++)
            {
                DataBase db = new DataBase();
                db._path = allDb[i];
                db._cfgPath = cfgPath;
                db._baseStructPathRaw = Path.Combine(allDb[i], "base_struct\\base");
                db._baseStructPathXml = Path.Combine(allDb[i], "base_struct\\base.xml");
                db.getProperties();
                lstDb.Add(db);
            }

            return lstDb;
        }

        public static void CreateDataBase(string DataBaseName, string dbPath, string cfgPath)
        {
            DirectoryInfo di = Directory.CreateDirectory(Path.Combine(dbPath, DataBaseName));
            DirectoryInfo diBaseStruct = di.CreateSubdirectory("base_struct");
            di.CreateSubdirectory("calculus");
            di.CreateSubdirectory("data_orga");
            DirectoryInfo diDataOrga = di.CreateSubdirectory("data_orga");
            diDataOrga.CreateSubdirectory("columns");
            diDataOrga.CreateSubdirectory("tables");
            diDataOrga.CreateSubdirectory("tmp");
            diDataOrga.CreateSubdirectory("vdg");
            diDataOrga.CreateSubdirectory("vdg\\data");

            StreamWriter sw = File.CreateText(Path.Combine(diBaseStruct.FullName, "base"));
            sw.Write(DataBaseName);
            sw.Flush();
            sw.Close();
            createIniProperties(DataBaseName, cfgPath);
        }

        private static void createIniProperties(string DataBaseName, string cfgPath)
        {
            StreamReader sr = new StreamReader(Path.Combine(cfgPath, "template.ini"));
            string iniPropertiesRef = sr.ReadToEnd();
            sr.Close();
            sr.Dispose();
            string iniProperties = iniPropertiesRef.Replace("[DB_NAME]", DataBaseName);
            StreamWriter sw = new StreamWriter(Path.Combine(cfgPath, DataBaseName + ".ini"), false);
            sw.Write(iniProperties);
            sw.Flush();
            sw.Close();
            sw.Dispose();
        }

        private void getProperties()
        {
            try
            {
                base_desc_type = DataBase.BaseDesc_t.XML;
                _xmlReader = new XmlDocument();
                _xmlReader.Load(@_baseStructPathXml);
                this._name = _xmlReader.SelectSingleNode("Database/@Name").Value;

            }
            catch (FileNotFoundException)
            {
                base_desc_type = DataBase.BaseDesc_t.RAW;
                _srProperties = new StreamReader(_baseStructPathRaw);
                if (!_srProperties.EndOfStream)
                    _name = _srProperties.ReadLine();
                if (!_srProperties.EndOfStream)
                    _tablecount = int.Parse(_srProperties.ReadLine());
            }
            catch (XmlException)
            {
                _xmlReader = null;
            }
        }

        private void getDataTableList()
        {
            string line;
            string[] tbline;
            DataTable dt=new DataTable();
            DataColumn dc;
            try
            {
                if (_srProperties != null)
                {
                    if (!_srProperties.EndOfStream)
                        while (!_srProperties.EndOfStream)
                        {
                            line = _srProperties.ReadLine();
                            if (line == string.Empty) // Traitement des tables
                            {
                                line = _srProperties.ReadLine();
                                if ((line != string.Empty) && (!_srProperties.EndOfStream))
                                {
                                    tbline = line.Split(new Char[] { ' ' });
                                    dt = new DataTable();
                                    dt.DataTableName = tbline[0].Substring(1, tbline[0].Length-2);
                                    dt.Order = int.Parse(tbline[1]);
                                    dt.RowCount = long.Parse(tbline[2]);
                                    if (_datatableList == null)
                                        _datatableList = new List<DataTable>();
                                    _datatableList.Add(dt);
                                }
                            }
                            else // Traitement des colonnes
                            {
                                tbline = line.Split(new Char[] { ' ' });
                                dc = new DataColumn();
                                dc.ColumnName = tbline[0].Substring(1, tbline[0].Length - 2);
                                dc.Order = int.Parse(tbline[1]);
                                dc.DataTypeName = tbline[3];
                                if((dc.DataTypeName=="VARCHAR") || (dc.DataTypeName=="VARCHAR2"))
                                    dc.DataTypeLength = UInt64.Parse(tbline[2])-1;
                                else
                                    dc.DataTypeLength = UInt64.Parse(tbline[2]);

                                if (dt != null)
                                {
                                    if (dt.DataColumns == null)
                                        dt.DataColumns = new List<DataColumn>();
                                    dt.DataColumns.Add(dc);
                                }
                            }
                        }
                }
                else if (_xmlReader != null)
                {
                    _datatableList = new List<DataTable>();
                    XmlNode tableListNode = _xmlReader.SelectSingleNode("//Tables");
                    XmlNodeList tableList = tableListNode.SelectNodes("Table");
                    foreach (XmlNode table in tableList)
                    {
                        dt = new DataTable();
                        dt.DataColumns = new List<DataColumn>();
                        dt.DataTableName = table.Attributes["Name"].Value;
                        dt.Order = Int32.Parse(table.Attributes["ID"].Value);
                        dt.RowCount = Int64.Parse(table.Attributes["NbRows"].Value);
                        _datatableList.Add(dt);
                        
                        XmlNode columnListNode = table.SelectSingleNode("//Columns");
                        XmlNodeList columnList = columnListNode.SelectNodes("Column");
                        foreach (XmlNode column in columnList)
                        {
                            dc = new DataColumn();
                            dc.ColumnName = column.Attributes["Name"].Value;
                            dc.Order = Int32.Parse(column.Attributes["ID"].Value);
                            dc.DataTypeName = column.Attributes["Type"].Value;
                            dc.DataTypeLength = UInt64.Parse(column.Attributes["Size"].Value);
                            dt.DataColumns.Add(dc);
                        }
                    }
                }
                else
                {
                    throw new ApplicationException(
                        string.Format("le fichier ini.Properties est manquant dans le répertoire {0}", this._path)
                        );
                }
            }
            finally
            {
                if (_srProperties != null)
                {
                    _srProperties.Close();
                    _srProperties.Dispose();
                }
            }

        }
    }
}

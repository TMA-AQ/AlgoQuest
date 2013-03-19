using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Windows.Forms;
using AlgoQuest.Core.Import;
using AlgoQuest.Core.DatabaseManagement;
using System.Configuration;
using System.IO;
using System.Data.SqlClient;
using MySql.Data.MySqlClient;
using System.Data.Odbc;

namespace AlgoQuest.UI.Forms.Import
{
    public partial class FrmDatabaseImportProcess : Form
    {
        private Connector.SgbdType _sgbdType;
        private DataBase _db;
        private List<DataImportTable> _tableList;
        private AppSettingsReader _appReader;
        private int _threadCount;
        private IConnector _conn;
        private List<int> _availableTableProcess = new List<int>();
        private int _columnsCount;

        public FrmDatabaseImportProcess(List<DataImportTable> tableList, DataBase newBase, Connector.SgbdType sgbdType, IConnector conn)
        {
            InitializeComponent();
            Cursor.Current = Cursors.WaitCursor;
            _sgbdType = sgbdType;
            _db = newBase;
            _tableList = tableList;
            _conn = conn;
            foreach (DataImportTable table in tableList)
                table.Count = conn.GetCount(table.Name);
            _columnsCount = tableList.Sum(t => t.Columns.Count);
            pbCic.Maximum = _columnsCount;

            launch();
            Cursor.Current = Cursors.Default;
        }
        private void launch()
        {
            generateBaseStruct();
            lblAvancementCsv.Text = "Chargement des tables...";
            importData();
        }

        private void isCsvFinished()
        {
            if(!_tableList.Exists(t => t.Loaded == false))
                lblAvancementCsv.Invoke(new Action(() =>
                {
                    lblAvancementCsv.Text = "Toutes les tables ont été exportées.";
                }));
        }

        private void isCicFinished()
        {
            if(!_tableList.Exists(t=>t.Columns.Exists(c => c.Loaded==false)))
                lblAvancementCic.Invoke(new Action(() =>
                {
                    lblAvancementCic.Text = "Toutes les données ont été importées.";
                }));
        }

        private void importData()
        {
            _appReader = new AppSettingsReader();
            _threadCount = (int)_appReader.GetValue("ThreadCount", typeof(int));
            initTableLayout();
            pbCsv.Maximum = _tableList.Count;
            pbCsv.Value = 0;
            foreach (DataImportTable table in _tableList)
            {
                ThreadPool.QueueUserWorkItem(new WaitCallback(imporTable), table);
            }
        }

        private void imporTable(object objtable)
        {
            int tableProcessKey;
            DataImportTable table = (DataImportTable)objtable;
            lock (_availableTableProcess)
            {
                tableProcessKey = _availableTableProcess.First();
                _availableTableProcess.Remove(tableProcessKey);
            }
            
            Label lbl = (Label)tlpProcess.Controls.Find("lbl" + tableProcessKey.ToString(),true).First();
            ProgressBar pb = (ProgressBar)tlpProcess.Controls.Find("pb" + tableProcessKey.ToString(), true).First();

            while (!lbl.IsHandleCreated)
                Thread.Sleep(100);
                                    
            lbl.Invoke(new Action(() => { lbl.Text = String.Format("Les données de la table {0} sont en cours d'export... ", table.Name); }));

            switch (_sgbdType)
            {
                case Connector.SgbdType.SqlServer:
                    GetSqlData(table, pb, lbl);
                    break;
                case Connector.SgbdType.MySql:
                    GetMySqlData(table, pb, lbl);
                    break;
                case Connector.SgbdType.Oracle:
                    GetOracleData(table, pb, lbl);
                    break;
            }

            lbl.Invoke(new Action(() => { lbl.Text = String.Format("Les données de la table {0} ont été exportées. ", ((DataImportTable)table).Name); }));

            while (!lblAvancementCsv.IsHandleCreated)
                Thread.Sleep(100);
            lblAvancementCsv.Invoke(new Action(() =>
            {
                lblAvancementCsv.Text =
                    String.Format("{0} tables exportées sur {1}.", table.Order, _tableList.Count);
            }));

            foreach (DataImportColumn column in table.Columns)
            {
                KeyValuePair<int, int> parameters = new KeyValuePair<int,int>(table.Order,column.Order);
                ThreadPool.QueueUserWorkItem(new WaitCallback(cutInColMultiThreaded), parameters);
            }

            lock (_availableTableProcess)
            {
                _availableTableProcess.Add(tableProcessKey);
            }
            table.Loaded = true;
            isCsvFinished();
        }

        private void cutInColMultiThreaded(object state)
        {
            KeyValuePair<int, int> parameters = (KeyValuePair<int, int>)state;
            CutInCol processCic = new CutInCol(_db);

            processCic.Process(parameters.Key, parameters.Value);
            pbCic.Invoke(new Action(() => { pbCic.Value++; }));
            lblAvancementCic.Invoke(new Action(() => { 
                lblAvancementCic.Text = String.Format("{0} colonnes importées sur {1}.", pbCic.Value, _columnsCount); 
            }));
            _tableList.Single(t => t.Order == parameters.Key).Columns.Single(c => c.Order == parameters.Value).Loaded = true;
            processCic = null;
            isCicFinished();
        }

        private void generateBaseStruct()
        {
            BaseStructGenerator td = new BaseStructGenerator(_sgbdType);
            string base_struct = td.GetBaseStruct(_db.Name, _tableList);
            _db.BaseStruct = base_struct;
        }


        private int addTableRow()
        {
            int index = tlpProcess.RowCount++;
            RowStyle style = new RowStyle(SizeType.AutoSize);
            tlpProcess.RowStyles.Add(style);
            return index;
        }

        private void initTableLayout()
        {
            if (_threadCount > 0)
                ThreadPool.SetMaxThreads(_threadCount, _threadCount);
            else
                ThreadPool.GetMaxThreads(out _threadCount, out _threadCount);

            int i = 0;
            int max = 0;
            if (_tableList.Count < _threadCount)
                max = _tableList.Count;
            else
                max = _threadCount;

            for (i = 0; i < max;)
            {
                Label lbl = new Label();
                lbl.Name = "lbl" + i.ToString();
                lbl.Dock = DockStyle.Fill;
                ProgressBar pb = new ProgressBar();
                pb.Name = "pb" + i.ToString();
                pb.Dock = DockStyle.Fill;
                tlpProcess.Controls.Add(lbl, 0, i);
                tlpProcess.Controls.Add(pb, 1, i);
                _availableTableProcess.Add(i);
                i = addTableRow();
            }
        }

        public void GetSqlData(DataImportTable table, ProgressBar pg, Label lbl)
        {
            using (SqlConnection conn = new SqlConnection(_conn.ConnectionString()))
            {
                SqlCommand command = new SqlCommand(getRequest(table), conn);
                command.CommandTimeout = 3600;
                conn.Open();
                SqlDataReader dr = command.ExecuteReader();
                writeTableCsv(lbl, pg, table, dr);
                dr.Close();
                command.Dispose();
                conn.Close();
            }
            pbCsv.Invoke(new Action(() => { pbCsv.Value++; }));
        }

        public void GetMySqlData(DataImportTable table, ProgressBar pg, Label lbl)
        {
            using (MySqlConnection conn = new MySqlConnection(_conn.ConnectionString()))
            {
                MySqlCommand command = new MySqlCommand(getRequest(table), conn);
                command.CommandTimeout = 3600;
                conn.Open();
                MySqlDataReader dr = command.ExecuteReader();
                writeTableCsv(lbl, pg, table, dr);
                dr.Close();
                command.Dispose();
                conn.Close();
            }
            pbCsv.Invoke(new Action(() => { pbCsv.Value++; }));
        }

        public void GetOracleData(DataImportTable table, ProgressBar pg, Label lbl)
        {
            using (OdbcConnection conn = new OdbcConnection(_conn.ConnectionString()))
            {
                OdbcCommand command = new OdbcCommand(getRequest(table), conn);
                conn.ConnectionTimeout = 3600;
                command.CommandTimeout = 3600;
                conn.Open();
                OdbcDataReader dr = command.ExecuteReader();
                writeTableCsv(lbl, pg, table, dr);
                dr.Close();
                command.Dispose();
                conn.Close();
            }
            pbCsv.Invoke(new Action(() => { pbCsv.Value++; }));
        }

        private void writeTableCsv(Label lbl, ProgressBar pb, DataImportTable table, IDataReader dr)
        {
            lbl.Invoke(new Action(() => { lbl.Text = String.Format("Les données de la table {0} sont chargées... ", table.Name); }));
            pb.Invoke(new Action(() => { pb.Value = 0; }));
            pb.Invoke(new Action(() => { pb.Maximum = (int)table.Count; }));
            using (StreamWriter sw = new StreamWriter(_db.DataOrgaTablesPath + "\\" + table.Name + ".txt"))
            {
                StringBuilder sb = new StringBuilder(0, int.MaxValue);
                int columnCount = dr.FieldCount - 1;
                string sep = ",";

                while (dr.Read())
                {
                    if (sb.Length > 10000)
                    {
                        sw.Write(sb.ToString());
                        sb.Clear();
                    }

                    int i = 0;
                    while (i <= columnCount)
                    {
                        string str = dr[i].ToString();
                        if (
                            (table.Columns[i].DataType == "DATE1")
                            ||
                            (table.Columns[i].DataType == "DATE2")
                            ||
                            (table.Columns[i].DataType == "DATE3")
                            )
                        {
                            DateTime dt = DateTime.Parse(dr[i].ToString());
                            switch (table.Columns[i].DataType)
                            {
                                case "DATE1":
                                    str = dt.ToString("dd/MM/yyyy HH:mm:ss");
                                    break;
                                case "DATE2" :
                                    str = dt.ToString("dd/MM/yyyy");
                                    break;
                                case "DATE3":
                                    str = dt.ToString("dd/MM/yy");
                                    break;
                            }
                        }
                        str = str.Replace("\"", "\"\""); //on double les guillements pour les protéger
                        str = "\"" + str + "\""; //on encadre les champs par des guillemets

                        sb.Append(str);
                        if (i != columnCount)
                            sb.Append(sep);
                        i++;
                    }
                    sb.AppendLine();
                    pb.Invoke(new Action(() => { pb.Value++; }));
                }
                sw.Write(sb.ToString());
                sw.Flush();
            }
        }

        private string getRequest(DataImportTable table)
        {
            string request = "SELECT {0} FROM " + table.Name;
            string columnList = string.Empty;
            foreach (DataImportColumn column in table.Columns.OrderBy(v => v.Order))
            {
                if (columnList != string.Empty)
                    columnList += ",";
                columnList += column.Name;
            }
            request = String.Format(request, columnList);
            return request;
        }

        #region Zone Cut in Col seul
        public FrmDatabaseImportProcess(DataBase newBase)
        {
            InitializeComponent();
            pbCsv.Visible = false;
            _db = newBase;
            try
            {
                launchCutInColOnly();
            }
            catch
            {
                MessageBox.Show("L'import a échoué. Vérifiez la présence des fichiers nécessaires."
                    , "Erreur", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }

        private Dictionary<AlgoQuest.Core.DatabaseManagement.DataColumn, bool> _columnCutInColList;

        private void launchCutInColOnly()
        {
            _columnsCount = _db.DataTableList.Sum(t => t.DataColumns.Count);
            pbCic.Maximum = _columnsCount;
            lblAvancementCsv.Text = "Import des données en cours.";

            _appReader = new AppSettingsReader();
            _threadCount = (int)_appReader.GetValue("ThreadCount", typeof(int));
            initTableLayoutCutInColOnly();
            
            _columnCutInColList = new Dictionary<AlgoQuest.Core.DatabaseManagement.DataColumn, bool>();
            foreach (AlgoQuest.Core.DatabaseManagement.DataTable table in _db.DataTableList)
            {
                foreach (AlgoQuest.Core.DatabaseManagement.DataColumn column in table.DataColumns)
                {
                    _columnCutInColList.Add(column, false);
                    KeyValuePair<AlgoQuest.Core.DatabaseManagement.DataTable, AlgoQuest.Core.DatabaseManagement.DataColumn> parameters =
                        new KeyValuePair<Core.DatabaseManagement.DataTable, Core.DatabaseManagement.DataColumn>(table, column);
                    ThreadPool.QueueUserWorkItem(new WaitCallback(cutInColOnly), parameters);
                }
            }
        }

        private void initTableLayoutCutInColOnly()
        {
            if (_threadCount > 0)
                ThreadPool.SetMaxThreads(_threadCount, _threadCount);
            else
                ThreadPool.GetMaxThreads(out _threadCount, out _threadCount);

            int i = 0;
            int max = 0;
            if (_columnsCount < _threadCount)
                max = _columnsCount;
            else
                max = _threadCount;

            for (i = 0; i < max;)
            {
                Label lbl = new Label();
                lbl.Name = "lbl" + i.ToString();
                lbl.Dock = DockStyle.Fill;
                ProgressBar pb = new ProgressBar();
                pb.Name = "pb" + i.ToString();
                pb.Dock = DockStyle.Fill;
                tlpProcess.Controls.Add(lbl, 0, i);
                tlpProcess.Controls.Add(pb, 1, i);
                _availableTableProcess.Add(i);
                i = addTableRow();
            }
        }

        private void cutInColOnly(object state)
        {
            int tableProcessKey;
            KeyValuePair<AlgoQuest.Core.DatabaseManagement.DataTable, AlgoQuest.Core.DatabaseManagement.DataColumn> parameters =
                        (KeyValuePair<Core.DatabaseManagement.DataTable, Core.DatabaseManagement.DataColumn>)state;
            AlgoQuest.Core.DatabaseManagement.DataTable table = parameters.Key;
            AlgoQuest.Core.DatabaseManagement.DataColumn column = parameters.Value;
            lock (_availableTableProcess)
            {
                tableProcessKey = _availableTableProcess.First();
                _availableTableProcess.Remove(tableProcessKey);
            }

            Label lbl = (Label)tlpProcess.Controls.Find("lbl" + tableProcessKey.ToString(), true).First();
            ProgressBar pb = (ProgressBar)tlpProcess.Controls.Find("pb" + tableProcessKey.ToString(), true).First();

            while (!lbl.IsHandleCreated)
                Thread.Sleep(100);

            lbl.Invoke(new Action(() => { lbl.Text = String.Format("La colonne {0} de la table {1} est en cours d'import. ",column.ColumnName, table.DataTableName); }));


            pb.Invoke(new Action(() => { pb.Value = 0; }));
            pb.Invoke(new Action(() => { pb.Maximum = 1; }));
            CutInCol processCic = new CutInCol(_db);
            processCic.Process(table.Order, column.Order);
            pb.Invoke(new Action(() => { pb.Value++; }));
            pbCic.Invoke(new Action(() => { pbCic.Value++; }));
            lbl.Invoke(new Action(() => { lbl.Text = String.Format("La colonne {0} de la table {1} a été importée. ", column.ColumnName, table.DataTableName); }));
            lblAvancementCic.Invoke(new Action(() =>
            {
                lblAvancementCic.Text = String.Format("{0} colonnes importées sur {1}.", pbCic.Value, _columnsCount);
            }));
            _columnCutInColList[column] = true;
            isCutInColOnlyFinished();
            lock (_availableTableProcess)
            {
                _availableTableProcess.Add(tableProcessKey);
            }
            
        }

        private void isCutInColOnlyFinished()
        {
            if (!_columnCutInColList.ContainsValue(false))
                lblAvancementCic.Invoke(new Action(() =>
                {
                    lblAvancementCic.Text = "Toutes les données ont été importées.";
                }));
        }

        #endregion

        
    }
}

using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using AlgoQuest.Core.Import;
using System.Configuration;
using AlgoQuest.Core.DatabaseManagement;

namespace AlgoQuest.UI.Forms.Import
{
    public partial class FrmDatabaseImportExplorer : Form
    {
        private string _connectionString;
        private Connector.SgbdType _sgbdType;
        private AppSettingsReader _appReader;
        private List<DataType> _lstDt = DataType.GetDataTypeList();
        private DataTypeMapper _mapper;
        private string _baseName;
        private DataBase _db;
        public FrmDatabaseImportExplorer(Connector.SgbdType sgbdType, String connectionString, DataBase db)
        {
            InitializeComponent();
            _connectionString = connectionString;
            _sgbdType = sgbdType;
            _appReader = new AppSettingsReader();
            _db = db;
            populateTreeView();
            ((DataGridViewComboBoxColumn)dgvDataType.Columns["DataTypeTargetColumn"]).DataSource = _lstDt;
            ((DataGridViewComboBoxColumn)dgvDataType.Columns["DataTypeTargetColumn"]).DisplayMember = "Name";
            ((DataGridViewComboBoxColumn)dgvDataType.Columns["DataTypeTargetColumn"]).ValueMember = "Name";
            DataGridViewTextBoxCell cell = new DataGridViewTextBoxCell();
            cell.ValueType = typeof(Int64);
            dgvDataType.Columns[5].CellTemplate = cell;
            _mapper = new DataTypeMapper(_sgbdType);
        }

        private string getTableListRequest()
        {
            if (_sgbdType == Connector.SgbdType.SqlServer)
                return _appReader.GetValue("SqlTablesQuery", typeof(System.String)).ToString();
            else if (_sgbdType == Connector.SgbdType.Oracle)
                return _appReader.GetValue("OracleTablesQuery", typeof(System.String)).ToString();
            else if (_sgbdType == Connector.SgbdType.MySql)
                return _appReader.GetValue("MySqlTablesQuery", typeof(System.String)).ToString();
            else if (_sgbdType == Connector.SgbdType.AlgoQuest)
                return _appReader.GetValue("AlgoQuestTablesQuery", typeof(System.String)).ToString();
            else
                return null;
        }

        private string getColumnListRequest()
        {
            if (_sgbdType == Connector.SgbdType.SqlServer)
                return _appReader.GetValue("SqlColumnsQuery", typeof(System.String)).ToString();
            else if (_sgbdType == Connector.SgbdType.Oracle)
                return _appReader.GetValue("OracleColumnsQuery", typeof(System.String)).ToString();
            else if (_sgbdType == Connector.SgbdType.MySql)
                return _appReader.GetValue("MySqlColumnsQuery", typeof(System.String)).ToString();
            else if (_sgbdType == Connector.SgbdType.AlgoQuest)
                return _appReader.GetValue("AlgoQuestColumnQuery", typeof(System.String)).ToString();
            else
                return null;
        }

        private void populateTreeView()
        {
            tvObjects.Nodes[0].Nodes.Clear();
            TreeNode DBnode = tvObjects.Nodes[0];

            IConnector _conn = Connector.GetConnector(_sgbdType);
            _conn.Initialize(_connectionString);
            _baseName = _conn.GetBaseName();;
            DBnode.Text = _baseName;

            List<string> tableList = _conn.GetTableList(getTableListRequest());

            foreach (string tableName in tableList)
            {
                //string tableName = table.Key;
                TreeNode tableNode = DBnode.Nodes.Add(tableName, tableName, 1, 1);
                //tableNode.Tag = table.Value;
                string request = String.Format(getColumnListRequest(), tableName);
                List<Core.Import.DataImportColumn> lstDc = _conn.GetTableDataColumnList(request);
                foreach (Core.Import.DataImportColumn dc in lstDc)
                {
                    TreeNode columnNode = tableNode.Nodes.Add(dc.Name
                        , String.Format("{0} ({1}, {2})", dc.Name, dc.DataType, dc.Size)
                        , 2
                        , 2);
                    List<string> meta = new List<string>();
                    meta.Add(tableName);
                    meta.Add(dc.Name);
                    meta.Add(dc.DataType);
                    meta.Add(dc.Size);
                    columnNode.Tag = meta;
                }
            }
            _conn = null;
        }

        private void tvObjects_AfterCheck(object sender, TreeViewEventArgs e)
        {
            foreach (TreeNode child in e.Node.Nodes)
            {
                child.Checked = e.Node.Checked;
                if (e.Node.Checked)
                    e.Node.Expand();
                else
                    e.Node.Collapse(false);
            }

            if (e.Node.Level == 2) //niveau colonne
                if (e.Node.Checked)
                {
                    List<string> meta = (List<string>)e.Node.Tag;
                    
                    int i = dgvDataType.Rows.Add();
                    dgvDataType.Rows[i].Cells[0].Value = meta[0];
                    dgvDataType.Rows[i].Cells[1].Value = meta[1];
                    dgvDataType.Rows[i].Cells[2].Value = meta[2];
                    dgvDataType.Rows[i].Cells[3].Value = meta[3];
                    string finalDataType = _mapper.Map(meta[2]);
                    string finalDataTypeLength = _mapper.MapLength(meta[2], meta[3]);

                    if (_lstDt.Exists(item => item.Name == finalDataType))
                    {
                        dgvDataType.Rows[i].Cells[4].Value = finalDataType;
                        if (finalDataTypeLength != string.Empty)
                            dgvDataType.Rows[i].Cells[5].Value = Int64.Parse(finalDataTypeLength);
                        else
                            dgvDataType.Rows[i].Cells[5].Value = 0;
                    }
                }
                else
                {
                    List<string> meta = (List<string>)e.Node.Tag;
                    foreach (DataGridViewRow row in dgvDataType.Rows)
                        if ((string)row.Cells[0].Value == meta[0] && (string)row.Cells[1].Value == meta[1])
                            dgvDataType.Rows.Remove(row);
                }

        }

        private void btnNext_Click(object sender, EventArgs e)
        {
      
            List<DataImportTable> tableList = new List<DataImportTable>();
            foreach (DataGridViewRow row in dgvDataType.Rows)
            {
                string tableName = (string)row.Cells[0].Value;
                DataImportTable table;
                if (!tableList.Exists(t => t.Name == tableName))
                {
                    table = new DataImportTable() { 
                        Name = tableName
                        , Order = tableList.Count+1
                        //, Count = (UInt64)tvObjects.Nodes[0].Nodes.Find(tableName, false)[0].Tag
                        , Columns = new List<DataImportColumn>() 
                        , Loaded=false
                    };
                    
                    tableList.Add(table);
                }
                else
                    table = tableList.Single(t => t.Name == tableName);

                if (row.Cells[4].Value == null)
                    row.ErrorText = "Vous devez sélectionner un type de données.\n";

                if(row.Cells[5].Value==null)
                    row.ErrorText += "Vous devez sélectionner une taille pour le type de données.";

                if (row.ErrorText != string.Empty)
                    return;

                DataImportColumn column = new DataImportColumn()
                    {
                        Name = (string)row.Cells[1].Value
                        ,
                        DataType = (string)row.Cells[4].Value
                        ,
                        Size = row.Cells[5].Value.ToString()
                        ,
                        Order = table.Columns.Count+1
                        ,
                        Loaded = false
                    };
                table.Columns.Add(column);
            }
            IConnector conn = Connector.GetConnector(_sgbdType);
            conn.Initialize(_connectionString);
            Cursor.Current = Cursors.WaitCursor;
            FrmDatabaseImportProcess frmImportProcess = new FrmDatabaseImportProcess(tableList, _db, _sgbdType, conn);
            frmImportProcess.ShowDialog(this);
            Cursor.Current = Cursors.Default;
        }

        private void btnPrevious_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private void dgvDataType_DataError(object sender, DataGridViewDataErrorEventArgs e)
        {
            DataGridView dgv = (DataGridView)sender;
            
            dgv.Rows[e.RowIndex].ErrorText = "La valeur saisie doit être un entier.";
            e.Cancel = true;
            e.ThrowException = false;
        }

        private void dgvDataType_CellEndEdit(object sender, DataGridViewCellEventArgs e)
        {
            DataGridView dgv = (DataGridView)sender;
            dgv.Rows[e.RowIndex].ErrorText = "";
        }
    }
}

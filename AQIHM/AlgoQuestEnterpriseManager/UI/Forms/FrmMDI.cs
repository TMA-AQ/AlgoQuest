using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Configuration;
using System.Security.Cryptography;
using Aq = AlgoQuest.Core.DatabaseManagement;
using AlgoQuest.UI.Forms.Import;
using AlgoQuest.Core.DatabaseManagement;
using AlgoQuest.Configuration.Connection;
using AlgoQuest.Configuration.Core;

namespace AlgoQuest.UI.Forms
{
    public partial class FrmMdi : Form
    {
        private string _selectedBase;
        private List<Aq.DataBase> _lstDb;
        private Dictionary<string, FrmSqlEditor.ConnectAttr> connsAttr;

        public string SelectedBase
        {
            get {return _selectedBase;}
            set
            {
                _selectedBase = value;
                if (_selectedBase != string.Empty)
                {
                    tsbRequest.Enabled = true;
                    iniPropertiesToolStripMenuItem.Enabled = true;
                    ouvrirToolStripMenuItem.Enabled = true;
                }
                else
                {
                    tsbRequest.Enabled = false;
                    iniPropertiesToolStripMenuItem.Enabled = false;
                    ouvrirToolStripMenuItem.Enabled = false;
                }
            }
        }
        public FrmMdi()
        {
            connsAttr = new Dictionary<string, FrmSqlEditor.ConnectAttr>();
            InitializeComponent();
            if (connect())
            {
                this.Enabled = true;
                populateTreeView();
            }
            else
            {
                Application.Exit();
            }
            
        }

        private bool connect()
        {
            AppSettingsReader _appReader = new AppSettingsReader();
            string pass = _appReader.GetValue("Password", typeof(System.String)).ToString();
            string log = _appReader.GetValue("Login", typeof(System.String)).ToString();
            if ((pass == "") && (log == "")) return true;
            bool connected = false;
            int attempts = 0;
            while (connected == false && attempts < 3)
            {
                FrmConnection frmConnexion = new FrmConnection();
                DialogResult dg = frmConnexion.ShowDialog(this);
                if (dg == DialogResult.OK)
                {
                    SHA256 mySHA256 = SHA256Managed.Create();
                    string login = frmConnexion.Login;
                    string password = frmConnexion.Password;
                    byte[] data = Encoding.UTF8.GetBytes(password);
                    byte[] hashValue = mySHA256.ComputeHash(data);
                    string hashString = String.Empty;
                    foreach (byte x in hashValue)
                    {
                        hashString += String.Format("{0:x2}", x);
                    }
                    if (login == log && pass == hashString)
                        connected = true;
                    if (connected == false)
                    {
                        attempts++;
                        MessageBox.Show("Votre identification est erronée. Veuillez vérifier votre login et mot de passe."
                                      , "Erreur d'authentification"
                                      , MessageBoxButtons.OK
                                      , MessageBoxIcon.Stop);
                    }
                    frmConnexion.Close();
                }
                else
                {
                    frmConnexion.Close();
                    return false;
                }
            }
            return connected;
        }

        private void tsbRequest_Click(object sender, EventArgs e)
        {
            FrmSqlEditor.ConnectAttr connAttr = this.connsAttr[_selectedBase];
            FrmSqlEditor frmSqlEditor = new FrmSqlEditor(connAttr);
            frmSqlEditor.MdiParent = this;
            frmSqlEditor.Show();
            frmSqlEditor.WindowState = FormWindowState.Maximized;
        }

        private void iniPropertiesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FrmIniProperties frmProp = new FrmIniProperties(_selectedBase);
            frmProp.Show(this);
        }

        private void scriptSQLToolStripMenuItem_Click(object sender, EventArgs e)
        {
            ofdSqlScript.ShowDialog(this);
        }

        private void ofdSqlScript_FileOk(object sender, CancelEventArgs e)
        {
            FrmSqlEditor frmSqlEditor = new FrmSqlEditor(_selectedBase, ofdSqlScript.FileName);
            frmSqlEditor.MdiParent = this;
            frmSqlEditor.Show();
            frmSqlEditor.WindowState = FormWindowState.Maximized;
        }

        private void cascadeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            LayoutMdi(MdiLayout.Cascade);
        }

        private void mosaïqueHorizontaleToolStripMenuItem_Click(object sender, EventArgs e)
        {
            LayoutMdi(MdiLayout.TileHorizontal);
        }

        private void mosaïqueVerticaleToolStripMenuItem_Click(object sender, EventArgs e)
        {
            LayoutMdi(MdiLayout.TileVertical);
        }
        

        #region Explorer
        private void populateTreeView()
        {
            tvObjects.Nodes[0].Nodes.Clear();
            try
            {
                TreeNode trDb = null;
                TreeNode trDt = null;
                AppSettingsReader _appReader = new AppSettingsReader();
                String dbPath = _appReader.GetValue("DataBasePath", typeof(System.String)).ToString();
                String cfgPath = _appReader.GetValue("ConfigPath", typeof(System.String)).ToString();
                _lstDb = Aq.DataBase.GetDatabases(dbPath, cfgPath);
                foreach (Aq.DataBase db in _lstDb)
                {
                    trDb = tvObjects.Nodes[0].Nodes.Add(db.Name, db.Name, 1, 1);
                    trDb.ContextMenuStrip = cmsDataBase;
                    if (db.DataTableList != null)
                    {
                        foreach (Aq.DataTable dt in db.DataTableList)
                        {
                            trDt = trDb.Nodes.Add(dt.DataTableName, dt.DataTableName, 2, 2);
                            foreach (Aq.DataColumn dc in dt.DataColumns)
                            {
                                trDt.Nodes.Add(dc.ColumnName
                                    , String.Format("{0} ({1}, {2})", dc.ColumnName, dc.DataTypeName, dc.DataTypeLength.ToString())
                                    , 3
                                    , 3);
                            }
                        }
                    }

                    FrmSqlEditor.ConnectAttr connAttr = new FrmSqlEditor.ConnectAttr();
                    connAttr._type = FrmSqlEditor.ConnectAttr.type.LOCAL;
                    connAttr._database = db.Name;
                    connsAttr.Add(db.Name, connAttr);
                }

                //
                DataBaseParser dbParser = new DataBaseParser();
                TreeNode server = new TreeNode("Servers");
                tvObjects.Nodes.Add(server);

                RemoteDatabaseSection remoteSection = (RemoteDatabaseSection)ConfigurationManager.GetSection("RemoteDatabaseSection");
                if (remoteSection != null)
                {
                    List<RemoteDatabaseElement> _listMapElement = remoteSection.MapItems.AsQueryable().Cast<RemoteDatabaseElement>().ToList<RemoteDatabaseElement>();
                    foreach (RemoteDatabaseElement e in _listMapElement)
                    {
                        trDb = server.Nodes.Add(e.key, e.key, 1, 1);
                        dbParser.fill(trDb, e.host, UInt16.Parse(e.port), e.dbname);
                        trDb.ContextMenuStrip = cmsDataBase;
                        FrmSqlEditor.ConnectAttr connAttr = new FrmSqlEditor.ConnectAttr();
                        connAttr._type = FrmSqlEditor.ConnectAttr.type.REMOTE;
                        connAttr._database = e.dbname;
                        connAttr._server = e.host;
                        connAttr._port = UInt16.Parse(e.port);
                        connsAttr.Add(e.key, connAttr);
                    }
                }

                OdbcConnectionSection odbcSection = (OdbcConnectionSection)ConfigurationManager.GetSection("OdbcConnectionSection");
                if (odbcSection != null)
                {
                    List<OdbcConnectionElement> _listMapElement = odbcSection.MapItems.AsQueryable().Cast<OdbcConnectionElement>().ToList<OdbcConnectionElement>();
                    foreach (OdbcConnectionElement e in _listMapElement)
                    {
                        trDb = server.Nodes.Add(e.key, e.key, 1, 1);
                        dbParser.fill(trDb, e.value);
                        trDb.ContextMenuStrip = cmsDataBase;
                        FrmSqlEditor.ConnectAttr connAttr = new FrmSqlEditor.ConnectAttr();
                        connAttr._type = FrmSqlEditor.ConnectAttr.type.ODBC;
                        connAttr._odbc_conn_str = e.value;
                        connsAttr.Add(e.key, connAttr);
                    }
                }

            }
            catch (Exception ex)
            {
                MessageBox.Show(
                    String.Format("Certaines bases sont inaccessibles\n{0}\n{1}"
                    , Application.StartupPath, ex.Message)
                    , "Erreur"
                    , MessageBoxButtons.OK
                    , MessageBoxIcon.Error);

            }
        }

        private void tsmConnexion_Click(object sender, EventArgs e)
        {
            Aq.DataBase db = _lstDb.Single(d => d.Name == tvObjects.SelectedNode.Name);
            if (db != null)
            {
                FrmSourceConnection frmConnexion = new FrmSourceConnection(db);
                frmConnexion.Show(this);
            }
        }

        private void tvObjects_AfterSelect(object sender, TreeViewEventArgs e)
        {
            string selectedBase = string.Empty;
            if (e.Node.Level > 0)
            {
                int level = e.Node.Level;
                TreeNode node = e.Node;
                while (level != 1)
                {
                    node = node.Parent;
                    level = node.Level;
                }
                selectedBase = node.Name;
            }
            SelectedBase = selectedBase;
        }

        private void tvObjects_MouseDown(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                TreeNode selectedNode = tvObjects.GetNodeAt(e.X, e.Y);
                tvObjects.SelectedNode = selectedNode;
            }
        }

        private void actualiserToolStripMenuItem_Click(object sender, EventArgs e)
        {
            populateTreeView();
        }

        private void tsmExistingFiles_Click(object sender, EventArgs e)
        {
            Aq.DataBase db = _lstDb.Single(d => d.Name == tvObjects.SelectedNode.Name);
            if (db != null)
            {
                FrmDatabaseImportProcess frmDataImportProcess = new FrmDatabaseImportProcess(db);
                frmDataImportProcess.Show(this);
            }
        }
        private void tsmCreateDataBase_Click(object sender, EventArgs e)
        {
            FrmCreateDataBase frmCdb = new FrmCreateDataBase(_lstDb);
            DialogResult dr = frmCdb.ShowDialog(this);
            if (dr == System.Windows.Forms.DialogResult.OK)
                populateTreeView();
        }

        #endregion

    }
}

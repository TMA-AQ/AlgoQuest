using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Configuration;
using Aq = AlgoQuest.Core.DatabaseManagement;
using AlgoQuest.UI.Forms.Import;

namespace AlgoQuest.UI.Forms
{
    public partial class FrmMdi : Form
    {
        private string _selectedBase;
        private List<Aq.DataBase> _lstDb;
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
            return true;
            bool connected = false;
            int attempts = 0;
            while (connected == false && attempts < 3)
            {
                FrmConnection frmConnexion = new FrmConnection();
                DialogResult dg = frmConnexion.ShowDialog(this);
                if (dg == DialogResult.OK)
                {
                    string login = frmConnexion.Login;
                    string password = frmConnexion.Password;
                    AppSettingsReader _appReader = new AppSettingsReader();
                    string pass = _appReader.GetValue("Password", typeof(System.String)).ToString();
                    string log = _appReader.GetValue("Login", typeof(System.String)).ToString();
                    if (login == log && pass == password)
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
            FrmSqlEditor frmSqlEditor = new FrmSqlEditor(_selectedBase);
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
                AppSettingsReader _appReader = new AppSettingsReader();
                String dbPath = _appReader.GetValue("DataBasePath", typeof(System.String)).ToString();
                String cfgPath = _appReader.GetValue("ConfigPath", typeof(System.String)).ToString();
                _lstDb = Aq.DataBase.GetDatabases(dbPath, cfgPath);
                foreach (Aq.DataBase db in _lstDb)
                {
                    TreeNode trDb = tvObjects.Nodes[0].Nodes.Add(db.Name, db.Name, 1, 1);
                    trDb.ContextMenuStrip = cmsDataBase;
                    if (db.DataTableList != null)
                        foreach (Aq.DataTable dt in db.DataTableList)
                        {
                            TreeNode trDt = trDb.Nodes.Add(dt.DataTableName, dt.DataTableName, 2, 2);
                            foreach (Aq.DataColumn dc in dt.DataColumns)
                            {
                                trDt.Nodes.Add(dc.ColumnName
                                    , String.Format("{0} ({1}, {2})", dc.ColumnName, dc.DataTypeName, dc.DataTypeLength.ToString())
                                    , 3
                                    , 3);
                            }
                        }
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show(
                    String.Format("Les bases de données sont inaccessibles. Vérifiez l'accès au répertoire 'DB' du système\n{0}\n{1}"
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

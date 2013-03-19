using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using AlgoQuest.UI.Forms.Import;
using AlgoQuest.Core.DatabaseManagement;
using System.Configuration;

namespace AlgoQuest.UI.Forms
{
    public partial class FrmExplorer : Form
    {
        private List<DataBase> lstDb;
        public FrmExplorer()
        {
            InitializeComponent();
            populateTreeView();
        }

        private void populateTreeView()
        {
            tvObjects.Nodes[0].Nodes.Clear();
            try
            {
                AppSettingsReader _appReader = new AppSettingsReader();
                String dbPath = _appReader.GetValue("DataBasePath", typeof(System.String)).ToString();
                String cfgPath = _appReader.GetValue("ConfigPath", typeof(System.String)).ToString();
                lstDb = DataBase.GetDatabases(dbPath, cfgPath);
                foreach (DataBase db in lstDb)
                {
                    TreeNode trDb = tvObjects.Nodes[0].Nodes.Add(db.Name, db.Name, 1, 1);
                    trDb.ContextMenuStrip = cmsDataBase;
                    if (db.DataTableList != null)
                        foreach (DataTable dt in db.DataTableList)
                        {
                            TreeNode trDt = trDb.Nodes.Add(dt.DataTableName, dt.DataTableName, 2, 2);
                            foreach (DataColumn dc in dt.DataColumns)
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
            DataBase db = lstDb.Single(d => d.Name == tvObjects.SelectedNode.Name);
            if (db != null)
            {
                FrmSourceConnection frmConnexion = new FrmSourceConnection(db);
                frmConnexion.Show(this);
            }
        }

        private void cmsServer_ItemClicked(object sender, ToolStripItemClickedEventArgs e)
        {
            if (e.ClickedItem.Name == "tsmCreateDataBase")
            {
                FrmCreateDataBase frmCdb = new FrmCreateDataBase(lstDb);
                DialogResult dr = frmCdb.ShowDialog(this);
                if (dr == System.Windows.Forms.DialogResult.OK)
                    populateTreeView();
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
            ((FrmMdi)this.MdiParent).SelectedBase = selectedBase;
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
            DataBase db = lstDb.Single(d => d.Name == tvObjects.SelectedNode.Name);
            if (db != null)
            {
                FrmDatabaseImportProcess frmDataImportProcess = new FrmDatabaseImportProcess(db);
                frmDataImportProcess.Show(this);
            }
        }
    }
}

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
    public partial class FrmSourceConnection : Form
    {
        private string _connectionString = string.Empty;
        private AppSettingsReader _appReader;
        private Connector.SgbdType _sgbdType;
        private DataBase _db;

        public FrmSourceConnection(DataBase db)
        {
            InitializeComponent();
            _appReader = new AppSettingsReader();
            _db = db;
        }

        private void btnConnexion_Click(object sender, EventArgs e)
        {
            Cursor.Current = Cursors.WaitCursor;
            bool blnOpenTreeview = false;
            if (_sgbdType == Connector.SgbdType.SqlServer)
                blnOpenTreeview = connectSQLServer();
            else if (_sgbdType == Connector.SgbdType.Oracle)
                blnOpenTreeview = connectOracle();
            else if (_sgbdType == Connector.SgbdType.MySql)
                blnOpenTreeview = connectMySql();
            Cursor.Current = Cursors.Default;

            if (blnOpenTreeview)
            {
                FrmDatabaseImportExplorer frmDie = new FrmDatabaseImportExplorer(_sgbdType, _connectionString, _db);
                frmDie.Show();
                this.Close();
            }
        }

        private void showSQLServerControls()
        {
            lblInitialCatalog.Visible = true;
            txtInitialCatalog.Visible = true;
            chkIntegratedSecurity.Visible = true;
        }

        private void hideSQLServerControls()
        {
            lblInitialCatalog.Visible = false;
            txtInitialCatalog.Visible = false;
            chkIntegratedSecurity.Visible = false;
        }

        private void showMySqlControls()
        {
            lblDatabase.Visible = true;
            txtDatabase.Visible = true;
        }

        private void hideMySqlControls()
        {
            lblDatabase.Visible = false;
            txtDatabase.Visible = false;
        }

        private void cbDbChoice_SelectedIndexChanged(object sender, EventArgs e)
        {
            hideSQLServerControls();
            hideMySqlControls();
            if ((string)cbDbChoice.SelectedItem == "SQL Server")
            {
                showSQLServerControls();
                _sgbdType = Connector.SgbdType.SqlServer;
            }
            else if ((string)cbDbChoice.SelectedItem == "Oracle")
            {
                _sgbdType = Connector.SgbdType.Oracle;
            }
            else if ((string)cbDbChoice.SelectedItem == "MySql")
            {
                showMySqlControls();
                _sgbdType = Connector.SgbdType.MySql;
            }
        }

        private bool connectSQLServer()
        {
            bool blnConnection = false;
            IConnector _conn = Connector.GetConnector(Connector.SgbdType.SqlServer);
            if (chkIntegratedSecurity.Checked)
            {
                _connectionString = String.Format("Data Source={0};Initial Catalog={1};Integrated Security=SSPI;", txtDataSource.Text, txtInitialCatalog.Text);
            }
            else
            {
                _connectionString = String.Format("Data Source={0};Initial Catalog={1};User Id={2};Password={3};", txtDataSource.Text, txtInitialCatalog.Text, txtUserId.Text, txtPassword.Text);
            }
            _conn.Initialize(_connectionString);
            try
            {
                _conn.TestConnection();
                blnConnection = true;
            }
            catch (Exception ex)
            {
                MessageBox.Show("La connexion à la base a échoué : \n" + ex.Message, "Erreur de connexion", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally { _conn = null; }
            

            return blnConnection;
        }

        private bool connectOracle()
        {
            bool blnConnection = false;
            IConnector _conn = Connector.GetConnector(Connector.SgbdType.Oracle);
            //_connectionString = String.Format("Data Source={0};User ID={1};Password={2}", txtDataSource.Text, txtUserId.Text, txtPassword.Text);
            _connectionString = String.Format("DSN={0};Uid={1};Pwd={2}", txtDataSource.Text, txtUserId.Text, txtPassword.Text);
            _conn.Initialize(_connectionString);
            try
            {
                _conn.TestConnection();
                blnConnection = true;
            }
            catch (Exception ex)
            {
                MessageBox.Show("La connexion à la base a échoué : \n" + ex.Message, "Erreur de connexion", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally { _conn = null; }

            return blnConnection;
        }

        private bool connectMySql()
        {
            bool blnConnection = false;
            IConnector _conn = Connector.GetConnector(Connector.SgbdType.MySql);
            _connectionString = String.Format("datasource={0};username={1};password={2};database={3}", txtDataSource.Text, txtUserId.Text, txtPassword.Text, txtDatabase.Text);
            _conn.Initialize(_connectionString);
            try
            {
                _conn.TestConnection();
                blnConnection = true;
            }
            catch (Exception ex)
            {
                MessageBox.Show("La connexion à la base a échoué : \n" + ex.Message, "Erreur de connexion", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            finally { _conn = null; }

            return blnConnection;
        }

        private void chkPasswordClear_CheckedChanged(object sender, EventArgs e)
        {
            if (chkPasswordClear.Checked)
                txtPassword.UseSystemPasswordChar = false;
            else
                txtPassword.UseSystemPasswordChar = true;
        }


    }
}

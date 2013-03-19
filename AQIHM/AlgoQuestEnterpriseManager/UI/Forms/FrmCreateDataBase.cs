using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using AlgoQuest.Core.DatabaseManagement;
using System.Configuration;

namespace AlgoQuest.UI.Forms
{
    public partial class FrmCreateDataBase : Form
    {
        private List<DataBase> dataBaseList;
        public FrmCreateDataBase(List<DataBase> dbList)
        {
            InitializeComponent();
            dataBaseList = dbList;
        }

        private void btnAccept_Click(object sender, EventArgs e)
        {
            if (dataBaseList.Where(v => v.Name == txtDataBaseName.Text).Count() == 0)
            {
                AppSettingsReader _appReader = new AppSettingsReader();
                String dbPath = _appReader.GetValue("DataBasePath", typeof(System.String)).ToString();
                String cfgPath = _appReader.GetValue("ConfigPath", typeof(System.String)).ToString();
                DataBase.CreateDataBase(txtDataBaseName.Text, dbPath, cfgPath);
                this.DialogResult = System.Windows.Forms.DialogResult.OK;
                this.Close();
            }
            else
                MessageBox.Show((String.Format("La base {0} existe déjà.", txtDataBaseName.Text)), "Erreur", MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);

        }
    }
}

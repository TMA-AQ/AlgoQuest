using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace AlgoQuest.UI.Forms
{
    public partial class FrmConnection : Form
    {
        public FrmConnection()
        {
            InitializeComponent();
            this.cbDatabase.SelectedIndex = 0;
        }

        public string Login { get; set; }
        public string Password { get; set; }

        private void cbDatabase_SelectedIndexChanged(object sender, EventArgs e)
        {

        }

        private void btnConnection_Click(object sender, EventArgs e)
        {
            Login = txtUser.Text;
            Password = txtPassword.Text;
            this.DialogResult = System.Windows.Forms.DialogResult.OK;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.DialogResult = System.Windows.Forms.DialogResult.Cancel;
        }
    }
}

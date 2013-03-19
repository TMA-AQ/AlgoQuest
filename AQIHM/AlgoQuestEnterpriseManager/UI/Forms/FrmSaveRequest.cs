using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using AlgoQuest.Core.DatabaseManagement;

namespace AlgoQuest.UI.Forms
{
    public partial class FrmSaveRequest : Form
    {
        private RequestData _rd;
        private List<RequestData> _requestList;
        public FrmSaveRequest(RequestData rd, List<RequestData> requestList)
        {
            InitializeComponent();
            _rd = rd;
            _requestList = requestList;
            tbRequestName.Text = _rd.Name;
            tbRequestComment.Text = _rd.Comment;
        }

        private void btnOk_Click(object sender, EventArgs e)
        {
            if (tbRequestName.Text == string.Empty)
                epRequest.SetError(tbRequestName, "Saisissez un nom.");
            else
            {
                epRequest.SetError(tbRequestName, "");
                if (_requestList.Exists(item => item.Name == tbRequestName.Text))
                {
                    DialogResult dr =
                        MessageBox.Show(String.Format("Souhaitez-vous écraser la requête {0} existante?", _rd.Name)
                                , "Sauvegarde de la requête", MessageBoxButtons.YesNo, MessageBoxIcon.Question);
                    if (dr == System.Windows.Forms.DialogResult.Yes)
                    {
                        _rd.Comment = tbRequestComment.Text;
                        this.DialogResult = System.Windows.Forms.DialogResult.OK;
                        this.Close();
                    }
                }
                else
                {
                    _rd.Name = tbRequestName.Text;
                    _rd.Comment = tbRequestComment.Text;
                    this.DialogResult = System.Windows.Forms.DialogResult.OK;
                    this.Close();
                }
            }
        }
    }
}

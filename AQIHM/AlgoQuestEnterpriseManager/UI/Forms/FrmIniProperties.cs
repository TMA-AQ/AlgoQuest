using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using AlgoQuest.Configuration.Core;
using System.Collections;
using System.Configuration;

namespace AlgoQuest.UI.Forms
{
    public partial class FrmIniProperties : Form
    {
        private IniProperties _ip;
        private Hashtable _keys;
                
        public FrmIniProperties(string database)
        {
            InitializeComponent();
            tlpProperties.SuspendLayout();
            AppSettingsReader _appReader = new AppSettingsReader();
            String dbPath = _appReader.GetValue("DataBasePath", typeof(System.String)).ToString();
            _ip = new IniProperties(dbPath, database);
            _keys=_ip.Keys;
            int i = 0;
            foreach (DictionaryEntry de in _keys)
            {
                i = addTableRow();
                Label lbl = new Label();
                lbl.Name = "lbl" + i.ToString();
                lbl.Text = (string)de.Key;
                lbl.Dock = DockStyle.Fill;
                TextBox tb = new TextBox();
                tb.Name = "tb" + i.ToString();
                tb.Text = (string)de.Value;
                tb.Dock = DockStyle.Fill;
                tlpProperties.Controls.Add(lbl, 0, i);
                tlpProperties.Controls.Add(tb, 1, i);
            }
            i = addTableRow();
            btnCancel.Dock = DockStyle.Right;
            tlpProperties.Controls.Add(btnCancel, 0, i);
            tlpProperties.Controls.Add(btnSave, 1, i);
            tlpProperties.ResumeLayout();
        }

        private int addTableRow()
        {
            int index = tlpProperties.RowCount++;
            RowStyle style = new RowStyle(SizeType.AutoSize);
            tlpProperties.RowStyles.Add(style);
            return index;
        }

        private void btnSave_Click(object sender, EventArgs e)
        {
            Cursor.Current = Cursors.WaitCursor;
            
            for (int i = 1; i <= _keys.Count; i++)
            {
                Label lbl = (Label)tlpProperties.Controls.Find("lbl" + i.ToString(),false)[0];
                TextBox tb = (TextBox)tlpProperties.Controls.Find("tb" + i.ToString(), false)[0];
                _ip.Keys[lbl.Text] = tb.Text;
            }
            _ip.Save();
            Cursor.Current = Cursors.Default;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            this.Close();
        }

    }
}

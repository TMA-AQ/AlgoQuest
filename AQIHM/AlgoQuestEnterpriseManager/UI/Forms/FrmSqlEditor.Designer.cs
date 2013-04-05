namespace AlgoQuest.UI.Forms
{
    partial class FrmSqlEditor
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FrmSqlEditor));
            this.rtbEditor = new System.Windows.Forms.RichTextBox();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.tsbAnalyse = new System.Windows.Forms.ToolStripButton();
            this.tsbExecute = new System.Windows.Forms.ToolStripButton();
            this.tsbSave = new System.Windows.Forms.ToolStripButton();
            this.tsbOpen = new System.Windows.Forms.ToolStripButton();
            this.tlcRequest = new System.Windows.Forms.ToolStripComboBox();
            this.scSql = new System.Windows.Forms.SplitContainer();
            this.ssResult = new System.Windows.Forms.StatusStrip();
            this.tsslDuration = new System.Windows.Forms.ToolStripStatusLabel();
            this.tsslNumber = new System.Windows.Forms.ToolStripStatusLabel();
            this.dgResult = new System.Windows.Forms.DataGridView();
            this.odbcCombo = new System.Windows.Forms.ComboBox();
            this.odbcLabel = new System.Windows.Forms.Label();
            this.toolStrip1.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.scSql)).BeginInit();
            this.scSql.Panel1.SuspendLayout();
            this.scSql.Panel2.SuspendLayout();
            this.scSql.SuspendLayout();
            this.ssResult.SuspendLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dgResult)).BeginInit();
            this.SuspendLayout();
            // 
            // rtbEditor
            // 
            this.rtbEditor.Dock = System.Windows.Forms.DockStyle.Fill;
            this.rtbEditor.Location = new System.Drawing.Point(0, 0);
            this.rtbEditor.Name = "rtbEditor";
            this.rtbEditor.Size = new System.Drawing.Size(724, 551);
            this.rtbEditor.TabIndex = 0;
            this.rtbEditor.Text = "";
            // 
            // toolStrip1
            // 
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsbAnalyse,
            this.tsbExecute,
            this.tsbSave,
            this.tsbOpen,
            this.tlcRequest});
            this.toolStrip1.Location = new System.Drawing.Point(0, 0);
            this.toolStrip1.MaximumSize = new System.Drawing.Size(0, 25);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(724, 25);
            this.toolStrip1.TabIndex = 1;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // tsbAnalyse
            // 
            this.tsbAnalyse.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbAnalyse.Image = global::AlgoQuest.Properties.Resources.AnalyseSql;
            this.tsbAnalyse.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbAnalyse.Name = "tsbAnalyse";
            this.tsbAnalyse.Size = new System.Drawing.Size(23, 22);
            this.tsbAnalyse.Text = "toolStripButton1";
            this.tsbAnalyse.ToolTipText = "Analyser la requête";
            this.tsbAnalyse.Click += new System.EventHandler(this.tsbAnalyse_Click);
            // 
            // tsbExecute
            // 
            this.tsbExecute.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbExecute.Image = global::AlgoQuest.Properties.Resources.ExecuteSql;
            this.tsbExecute.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbExecute.Name = "tsbExecute";
            this.tsbExecute.Size = new System.Drawing.Size(23, 22);
            this.tsbExecute.Text = "toolStripButton2";
            this.tsbExecute.ToolTipText = "Exécuter la requête";
            this.tsbExecute.Click += new System.EventHandler(this.tsbExecute_Click);
            // 
            // tsbSave
            // 
            this.tsbSave.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbSave.Image = ((System.Drawing.Image)(resources.GetObject("tsbSave.Image")));
            this.tsbSave.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbSave.Name = "tsbSave";
            this.tsbSave.Size = new System.Drawing.Size(23, 22);
            this.tsbSave.Text = "Sauvegarder le script";
            this.tsbSave.Click += new System.EventHandler(this.tsbSave_Click);
            // 
            // tsbOpen
            // 
            this.tsbOpen.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
            this.tsbOpen.Enabled = false;
            this.tsbOpen.Image = global::AlgoQuest.Properties.Resources.ViewScript;
            this.tsbOpen.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbOpen.Name = "tsbOpen";
            this.tsbOpen.Size = new System.Drawing.Size(23, 22);
            this.tsbOpen.Text = "Ouvrir";
            this.tsbOpen.Click += new System.EventHandler(this.tsbOpen_Click);
            // 
            // tlcRequest
            // 
            this.tlcRequest.Name = "tlcRequest";
            this.tlcRequest.Size = new System.Drawing.Size(121, 25);
            this.tlcRequest.SelectedIndexChanged += new System.EventHandler(this.tlcRequest_SelectedIndexChanged);
            // 
            // scSql
            // 
            this.scSql.Dock = System.Windows.Forms.DockStyle.Fill;
            this.scSql.Location = new System.Drawing.Point(0, 25);
            this.scSql.Name = "scSql";
            this.scSql.Orientation = System.Windows.Forms.Orientation.Horizontal;
            // 
            // scSql.Panel1
            // 
            this.scSql.Panel1.Controls.Add(this.rtbEditor);
            // 
            // scSql.Panel2
            // 
            this.scSql.Panel2.Controls.Add(this.ssResult);
            this.scSql.Panel2.Controls.Add(this.dgResult);
            this.scSql.Panel2Collapsed = true;
            this.scSql.Panel2MinSize = 100;
            this.scSql.Size = new System.Drawing.Size(724, 551);
            this.scSql.SplitterDistance = 25;
            this.scSql.TabIndex = 2;
            // 
            // ssResult
            // 
            this.ssResult.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsslDuration,
            this.tsslNumber});
            this.ssResult.Location = new System.Drawing.Point(0, 24);
            this.ssResult.Name = "ssResult";
            this.ssResult.Size = new System.Drawing.Size(150, 22);
            this.ssResult.TabIndex = 1;
            this.ssResult.Text = "statusStrip1";
            this.ssResult.Visible = false;
            // 
            // tsslDuration
            // 
            this.tsslDuration.Name = "tsslDuration";
            this.tsslDuration.Size = new System.Drawing.Size(104, 17);
            this.tsslDuration.Text = "Durée d\'exécution : ";
            this.tsslDuration.Visible = false;
            // 
            // tsslNumber
            // 
            this.tsslNumber.Name = "tsslNumber";
            this.tsslNumber.Size = new System.Drawing.Size(143, 13);
            this.tsslNumber.Text = "Nombre d\'enregistrements : ";
            this.tsslNumber.Visible = false;
            // 
            // dgResult
            // 
            this.dgResult.AllowUserToAddRows = false;
            this.dgResult.AllowUserToDeleteRows = false;
            this.dgResult.BackgroundColor = System.Drawing.SystemColors.ControlLightLight;
            this.dgResult.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgResult.Dock = System.Windows.Forms.DockStyle.Fill;
            this.dgResult.Location = new System.Drawing.Point(0, 0);
            this.dgResult.Name = "dgResult";
            this.dgResult.ReadOnly = true;
            this.dgResult.RowHeadersVisible = false;
            this.dgResult.Size = new System.Drawing.Size(150, 46);
            this.dgResult.TabIndex = 0;
            // 
            // odbcCombo
            // 
            this.odbcCombo.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
            this.odbcCombo.FormattingEnabled = true;
            this.odbcCombo.Location = new System.Drawing.Point(283, -2);
            this.odbcCombo.Name = "odbcCombo";
            this.odbcCombo.Size = new System.Drawing.Size(440, 21);
            this.odbcCombo.TabIndex = 3;
            this.odbcCombo.SelectedIndexChanged += new System.EventHandler(this.odbcCombo_SelectedIndexChanged);
            // 
            // odbcLabel
            // 
            this.odbcLabel.AutoSize = true;
            this.odbcLabel.Location = new System.Drawing.Point(246, 6);
            this.odbcLabel.Name = "odbcLabel";
            this.odbcLabel.Size = new System.Drawing.Size(31, 13);
            this.odbcLabel.TabIndex = 4;
            this.odbcLabel.Text = "odbc";
            // 
            // FrmSqlEditor
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(724, 576);
            this.Controls.Add(this.odbcLabel);
            this.Controls.Add(this.odbcCombo);
            this.Controls.Add(this.scSql);
            this.Controls.Add(this.toolStrip1);
            this.Name = "FrmSqlEditor";
            this.Text = "Analyseur de requêtes";
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.scSql.Panel1.ResumeLayout(false);
            this.scSql.Panel2.ResumeLayout(false);
            this.scSql.Panel2.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.scSql)).EndInit();
            this.scSql.ResumeLayout(false);
            this.ssResult.ResumeLayout(false);
            this.ssResult.PerformLayout();
            ((System.ComponentModel.ISupportInitialize)(this.dgResult)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.RichTextBox rtbEditor;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripButton tsbAnalyse;
        private System.Windows.Forms.ToolStripButton tsbExecute;
        private System.Windows.Forms.ToolStripButton tsbSave;
        private System.Windows.Forms.ToolStripComboBox tlcRequest;
        private System.Windows.Forms.SplitContainer scSql;
        private System.Windows.Forms.DataGridView dgResult;
        private System.Windows.Forms.StatusStrip ssResult;
        private System.Windows.Forms.ToolStripStatusLabel tsslDuration;
        private System.Windows.Forms.ToolStripStatusLabel tsslNumber;
        private System.Windows.Forms.ToolStripButton tsbOpen;
        private System.Windows.Forms.ComboBox odbcCombo;
        private System.Windows.Forms.Label odbcLabel;
    }
}
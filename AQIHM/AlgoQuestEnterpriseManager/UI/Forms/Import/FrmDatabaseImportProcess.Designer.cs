namespace AlgoQuest.UI.Forms.Import
{
    partial class FrmDatabaseImportProcess
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
            this.gbGlobalProcess = new System.Windows.Forms.GroupBox();
            this.pbCsv = new System.Windows.Forms.ProgressBar();
            this.lblAvancementCsv = new System.Windows.Forms.Label();
            this.tlpProcess = new System.Windows.Forms.TableLayoutPanel();
            this.gbGlobalProcess.SuspendLayout();
            this.SuspendLayout();
            // 
            // gbGlobalProcess
            // 
            this.gbGlobalProcess.Controls.Add(this.pbCsv);
            this.gbGlobalProcess.Controls.Add(this.lblAvancementCsv);
            this.gbGlobalProcess.Location = new System.Drawing.Point(12, 12);
            this.gbGlobalProcess.Name = "gbGlobalProcess";
            this.gbGlobalProcess.Size = new System.Drawing.Size(667, 59);
            this.gbGlobalProcess.TabIndex = 1;
            this.gbGlobalProcess.TabStop = false;
            this.gbGlobalProcess.Text = "Avancement général";
            // 
            // pbCsv
            // 
            this.pbCsv.Location = new System.Drawing.Point(311, 20);
            this.pbCsv.Name = "pbCsv";
            this.pbCsv.Size = new System.Drawing.Size(350, 23);
            this.pbCsv.TabIndex = 1;
            // 
            // lblAvancementCsv
            // 
            this.lblAvancementCsv.AutoSize = true;
            this.lblAvancementCsv.Location = new System.Drawing.Point(7, 30);
            this.lblAvancementCsv.Name = "lblAvancementCsv";
            this.lblAvancementCsv.Size = new System.Drawing.Size(0, 13);
            this.lblAvancementCsv.TabIndex = 0;
            // 
            // tlpProcess
            // 
            this.tlpProcess.AutoScroll = true;
            this.tlpProcess.AutoSize = true;
            this.tlpProcess.ColumnCount = 2;
            this.tlpProcess.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 46.62669F));
            this.tlpProcess.ColumnStyles.Add(new System.Windows.Forms.ColumnStyle(System.Windows.Forms.SizeType.Percent, 53.37331F));
            this.tlpProcess.Location = new System.Drawing.Point(12, 87);
            this.tlpProcess.Name = "tlpProcess";
            this.tlpProcess.RowCount = 1;
            this.tlpProcess.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tlpProcess.RowStyles.Add(new System.Windows.Forms.RowStyle(System.Windows.Forms.SizeType.Percent, 50F));
            this.tlpProcess.Size = new System.Drawing.Size(661, 36);
            this.tlpProcess.TabIndex = 2;
            // 
            // FrmDatabaseImportProcess
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.AutoScroll = true;
            this.AutoSize = true;
            this.ClientSize = new System.Drawing.Size(691, 138);
            this.Controls.Add(this.tlpProcess);
            this.Controls.Add(this.gbGlobalProcess);
            this.Name = "FrmDatabaseImportProcess";
            this.Text = "Import des données";
            this.gbGlobalProcess.ResumeLayout(false);
            this.gbGlobalProcess.PerformLayout();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.GroupBox gbGlobalProcess;
        private System.Windows.Forms.Label lblAvancementCsv;
        private System.Windows.Forms.TableLayoutPanel tlpProcess;
        private System.Windows.Forms.ProgressBar pbCsv;
    }
}
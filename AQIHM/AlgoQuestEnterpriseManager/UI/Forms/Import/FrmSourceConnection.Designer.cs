namespace AlgoQuest.UI.Forms.Import
{
    partial class FrmSourceConnection
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
            this.lblDataSource = new System.Windows.Forms.Label();
            this.txtDataSource = new System.Windows.Forms.TextBox();
            this.txtUserId = new System.Windows.Forms.TextBox();
            this.txtPassword = new System.Windows.Forms.TextBox();
            this.lblPassword = new System.Windows.Forms.Label();
            this.chkIntegratedSecurity = new System.Windows.Forms.CheckBox();
            this.lblUserId = new System.Windows.Forms.Label();
            this.btnConnexion = new System.Windows.Forms.Button();
            this.cbDbChoice = new System.Windows.Forms.ComboBox();
            this.lblDataBaseChoice = new System.Windows.Forms.Label();
            this.txtInitialCatalog = new System.Windows.Forms.TextBox();
            this.lblInitialCatalog = new System.Windows.Forms.Label();
            this.txtDatabase = new System.Windows.Forms.TextBox();
            this.lblDatabase = new System.Windows.Forms.Label();
            this.chkPasswordClear = new System.Windows.Forms.CheckBox();
            this.SuspendLayout();
            // 
            // lblDataSource
            // 
            this.lblDataSource.AutoSize = true;
            this.lblDataSource.Location = new System.Drawing.Point(34, 43);
            this.lblDataSource.Name = "lblDataSource";
            this.lblDataSource.Size = new System.Drawing.Size(67, 13);
            this.lblDataSource.TabIndex = 22;
            this.lblDataSource.Text = "Data Source";
            // 
            // txtDataSource
            // 
            this.txtDataSource.Location = new System.Drawing.Point(107, 43);
            this.txtDataSource.Name = "txtDataSource";
            this.txtDataSource.Size = new System.Drawing.Size(250, 20);
            this.txtDataSource.TabIndex = 10;
            // 
            // txtUserId
            // 
            this.txtUserId.Location = new System.Drawing.Point(107, 69);
            this.txtUserId.Name = "txtUserId";
            this.txtUserId.Size = new System.Drawing.Size(250, 20);
            this.txtUserId.TabIndex = 20;
            // 
            // txtPassword
            // 
            this.txtPassword.Location = new System.Drawing.Point(107, 95);
            this.txtPassword.Name = "txtPassword";
            this.txtPassword.Size = new System.Drawing.Size(250, 20);
            this.txtPassword.TabIndex = 30;
            this.txtPassword.UseSystemPasswordChar = true;
            // 
            // lblPassword
            // 
            this.lblPassword.AutoSize = true;
            this.lblPassword.Location = new System.Drawing.Point(30, 95);
            this.lblPassword.Name = "lblPassword";
            this.lblPassword.Size = new System.Drawing.Size(71, 13);
            this.lblPassword.TabIndex = 26;
            this.lblPassword.Text = "Mot de passe";
            // 
            // chkIntegratedSecurity
            // 
            this.chkIntegratedSecurity.AutoSize = true;
            this.chkIntegratedSecurity.Location = new System.Drawing.Point(12, 147);
            this.chkIntegratedSecurity.Name = "chkIntegratedSecurity";
            this.chkIntegratedSecurity.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.chkIntegratedSecurity.Size = new System.Drawing.Size(106, 17);
            this.chkIntegratedSecurity.TabIndex = 60;
            this.chkIntegratedSecurity.Text = "Sécurité intégrée";
            this.chkIntegratedSecurity.UseVisualStyleBackColor = true;
            this.chkIntegratedSecurity.Visible = false;
            // 
            // lblUserId
            // 
            this.lblUserId.AutoSize = true;
            this.lblUserId.Location = new System.Drawing.Point(25, 69);
            this.lblUserId.Name = "lblUserId";
            this.lblUserId.Size = new System.Drawing.Size(76, 13);
            this.lblUserId.TabIndex = 28;
            this.lblUserId.Text = "Nom utilisateur";
            // 
            // btnConnexion
            // 
            this.btnConnexion.Location = new System.Drawing.Point(223, 170);
            this.btnConnexion.Name = "btnConnexion";
            this.btnConnexion.Size = new System.Drawing.Size(134, 23);
            this.btnConnexion.TabIndex = 80;
            this.btnConnexion.Text = "Suivant";
            this.btnConnexion.UseVisualStyleBackColor = true;
            this.btnConnexion.Click += new System.EventHandler(this.btnConnexion_Click);
            // 
            // cbDbChoice
            // 
            this.cbDbChoice.FormattingEnabled = true;
            this.cbDbChoice.Items.AddRange(new object[] {
            "SQL Server",
            "Oracle",
            "MySql",
            "AlgoQuest"});
            this.cbDbChoice.Location = new System.Drawing.Point(223, 12);
            this.cbDbChoice.Name = "cbDbChoice";
            this.cbDbChoice.Size = new System.Drawing.Size(134, 21);
            this.cbDbChoice.TabIndex = 1;
            this.cbDbChoice.SelectedIndexChanged += new System.EventHandler(this.cbDbChoice_SelectedIndexChanged);
            // 
            // lblDataBaseChoice
            // 
            this.lblDataBaseChoice.AutoSize = true;
            this.lblDataBaseChoice.Location = new System.Drawing.Point(20, 15);
            this.lblDataBaseChoice.Name = "lblDataBaseChoice";
            this.lblDataBaseChoice.Size = new System.Drawing.Size(202, 13);
            this.lblDataBaseChoice.TabIndex = 53;
            this.lblDataBaseChoice.Text = "Sélectionnez le type de base de données";
            // 
            // txtInitialCatalog
            // 
            this.txtInitialCatalog.Location = new System.Drawing.Point(108, 121);
            this.txtInitialCatalog.Name = "txtInitialCatalog";
            this.txtInitialCatalog.Size = new System.Drawing.Size(250, 20);
            this.txtInitialCatalog.TabIndex = 50;
            this.txtInitialCatalog.Visible = false;
            // 
            // lblInitialCatalog
            // 
            this.lblInitialCatalog.AutoSize = true;
            this.lblInitialCatalog.Location = new System.Drawing.Point(31, 124);
            this.lblInitialCatalog.Name = "lblInitialCatalog";
            this.lblInitialCatalog.Size = new System.Drawing.Size(70, 13);
            this.lblInitialCatalog.TabIndex = 55;
            this.lblInitialCatalog.Text = "Initial Catalog";
            this.lblInitialCatalog.Visible = false;
            // 
            // txtDatabase
            // 
            this.txtDatabase.Location = new System.Drawing.Point(107, 121);
            this.txtDatabase.Name = "txtDatabase";
            this.txtDatabase.Size = new System.Drawing.Size(250, 20);
            this.txtDatabase.TabIndex = 40;
            this.txtDatabase.Visible = false;
            // 
            // lblDatabase
            // 
            this.lblDatabase.AutoSize = true;
            this.lblDatabase.Location = new System.Drawing.Point(48, 124);
            this.lblDatabase.Name = "lblDatabase";
            this.lblDatabase.Size = new System.Drawing.Size(53, 13);
            this.lblDatabase.TabIndex = 57;
            this.lblDatabase.Text = "Database";
            this.lblDatabase.Visible = false;
            // 
            // chkPasswordClear
            // 
            this.chkPasswordClear.AutoSize = true;
            this.chkPasswordClear.Location = new System.Drawing.Point(199, 147);
            this.chkPasswordClear.Name = "chkPasswordClear";
            this.chkPasswordClear.RightToLeft = System.Windows.Forms.RightToLeft.Yes;
            this.chkPasswordClear.Size = new System.Drawing.Size(158, 17);
            this.chkPasswordClear.TabIndex = 70;
            this.chkPasswordClear.Text = "Voir le mot de passe en clair";
            this.chkPasswordClear.UseVisualStyleBackColor = true;
            this.chkPasswordClear.CheckedChanged += new System.EventHandler(this.chkPasswordClear_CheckedChanged);
            // 
            // FrmSourceConnection
            // 
            this.AcceptButton = this.btnConnexion;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(370, 202);
            this.Controls.Add(this.chkPasswordClear);
            this.Controls.Add(this.lblDatabase);
            this.Controls.Add(this.txtDatabase);
            this.Controls.Add(this.lblInitialCatalog);
            this.Controls.Add(this.txtInitialCatalog);
            this.Controls.Add(this.lblDataBaseChoice);
            this.Controls.Add(this.cbDbChoice);
            this.Controls.Add(this.btnConnexion);
            this.Controls.Add(this.lblUserId);
            this.Controls.Add(this.chkIntegratedSecurity);
            this.Controls.Add(this.lblPassword);
            this.Controls.Add(this.txtPassword);
            this.Controls.Add(this.txtUserId);
            this.Controls.Add(this.lblDataSource);
            this.Controls.Add(this.txtDataSource);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "FrmSourceConnection";
            this.Text = "Connexion à la base de données source";
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblDataSource;
        private System.Windows.Forms.TextBox txtDataSource;
        private System.Windows.Forms.TextBox txtUserId;
        private System.Windows.Forms.TextBox txtPassword;
        private System.Windows.Forms.Label lblPassword;
        private System.Windows.Forms.CheckBox chkIntegratedSecurity;
        private System.Windows.Forms.Label lblUserId;
        private System.Windows.Forms.Button btnConnexion;
        private System.Windows.Forms.ComboBox cbDbChoice;
        private System.Windows.Forms.Label lblDataBaseChoice;
        private System.Windows.Forms.TextBox txtInitialCatalog;
        private System.Windows.Forms.Label lblInitialCatalog;
        private System.Windows.Forms.TextBox txtDatabase;
        private System.Windows.Forms.Label lblDatabase;
        private System.Windows.Forms.CheckBox chkPasswordClear;
    }
}
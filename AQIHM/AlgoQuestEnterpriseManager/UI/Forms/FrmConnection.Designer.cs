namespace AlgoQuest.UI.Forms
{
    partial class FrmConnection
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
            this.cbDatabase = new System.Windows.Forms.ComboBox();
            this.lblDatabase = new System.Windows.Forms.Label();
            this.lblUser = new System.Windows.Forms.Label();
            this.txtUser = new System.Windows.Forms.TextBox();
            this.lblPassword = new System.Windows.Forms.Label();
            this.txtPassword = new System.Windows.Forms.TextBox();
            this.btnConnection = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.pictureBox1 = new System.Windows.Forms.PictureBox();
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).BeginInit();
            this.SuspendLayout();
            // 
            // cbDatabase
            // 
            this.cbDatabase.FlatStyle = System.Windows.Forms.FlatStyle.Popup;
            this.cbDatabase.FormattingEnabled = true;
            this.cbDatabase.Items.AddRange(new object[] {
            "Local"});
            this.cbDatabase.Location = new System.Drawing.Point(113, 81);
            this.cbDatabase.Name = "cbDatabase";
            this.cbDatabase.Size = new System.Drawing.Size(142, 21);
            this.cbDatabase.TabIndex = 0;
            this.cbDatabase.SelectedIndexChanged += new System.EventHandler(this.cbDatabase_SelectedIndexChanged);
            // 
            // lblDatabase
            // 
            this.lblDatabase.AutoSize = true;
            this.lblDatabase.Location = new System.Drawing.Point(14, 84);
            this.lblDatabase.Name = "lblDatabase";
            this.lblDatabase.Size = new System.Drawing.Size(88, 13);
            this.lblDatabase.TabIndex = 1;
            this.lblDatabase.Text = "Nom du serveur :";
            this.lblDatabase.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // lblUser
            // 
            this.lblUser.AutoSize = true;
            this.lblUser.Location = new System.Drawing.Point(12, 132);
            this.lblUser.Name = "lblUser";
            this.lblUser.Size = new System.Drawing.Size(90, 13);
            this.lblUser.TabIndex = 2;
            this.lblUser.Text = "Nom d\'utilisateur :";
            this.lblUser.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // txtUser
            // 
            this.txtUser.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtUser.Location = new System.Drawing.Point(113, 132);
            this.txtUser.Name = "txtUser";
            this.txtUser.Size = new System.Drawing.Size(142, 13);
            this.txtUser.TabIndex = 3;
            // 
            // lblPassword
            // 
            this.lblPassword.AutoSize = true;
            this.lblPassword.Location = new System.Drawing.Point(25, 180);
            this.lblPassword.Name = "lblPassword";
            this.lblPassword.Size = new System.Drawing.Size(77, 13);
            this.lblPassword.TabIndex = 4;
            this.lblPassword.Text = "Mot de passe :";
            this.lblPassword.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
            // 
            // txtPassword
            // 
            this.txtPassword.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.txtPassword.Location = new System.Drawing.Point(113, 180);
            this.txtPassword.Name = "txtPassword";
            this.txtPassword.Size = new System.Drawing.Size(142, 13);
            this.txtPassword.TabIndex = 5;
            this.txtPassword.UseSystemPasswordChar = true;
            // 
            // btnConnection
            // 
            this.btnConnection.BackColor = System.Drawing.SystemColors.Control;
            this.btnConnection.FlatAppearance.BorderSize = 2;
            this.btnConnection.FlatStyle = System.Windows.Forms.FlatStyle.System;
            this.btnConnection.Location = new System.Drawing.Point(113, 227);
            this.btnConnection.Name = "btnConnection";
            this.btnConnection.Size = new System.Drawing.Size(68, 23);
            this.btnConnection.TabIndex = 6;
            this.btnConnection.Text = "Connecter";
            this.btnConnection.UseVisualStyleBackColor = true;
            this.btnConnection.Click += new System.EventHandler(this.btnConnection_Click);
            // 
            // btnCancel
            // 
            this.btnCancel.BackColor = System.Drawing.SystemColors.Control;
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.FlatAppearance.BorderSize = 2;
            this.btnCancel.Location = new System.Drawing.Point(187, 227);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(68, 23);
            this.btnCancel.TabIndex = 7;
            this.btnCancel.Text = "Annuler";
            this.btnCancel.UseVisualStyleBackColor = true;
            this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
            // 
            // pictureBox1
            // 
            this.pictureBox1.Image = global::AlgoQuest.Properties.Resources.Algoquest__244x64_;
            this.pictureBox1.Location = new System.Drawing.Point(3, 3);
            this.pictureBox1.Name = "pictureBox1";
            this.pictureBox1.Size = new System.Drawing.Size(160, 45);
            this.pictureBox1.TabIndex = 9;
            this.pictureBox1.TabStop = false;
            // 
            // FrmConnection
            // 
            this.AcceptButton = this.btnConnection;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.btnCancel;
            this.ClientSize = new System.Drawing.Size(284, 262);
            this.Controls.Add(this.pictureBox1);
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnConnection);
            this.Controls.Add(this.txtPassword);
            this.Controls.Add(this.lblPassword);
            this.Controls.Add(this.txtUser);
            this.Controls.Add(this.lblUser);
            this.Controls.Add(this.lblDatabase);
            this.Controls.Add(this.cbDatabase);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.MaximumSize = new System.Drawing.Size(294, 294);
            this.MinimumSize = new System.Drawing.Size(294, 294);
            this.Name = "FrmConnection";
            this.ShowIcon = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.StartPosition = System.Windows.Forms.FormStartPosition.CenterParent;
            this.Text = "Se connecter au serveur";
            ((System.ComponentModel.ISupportInitialize)(this.pictureBox1)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ComboBox cbDatabase;
        private System.Windows.Forms.Label lblDatabase;
        private System.Windows.Forms.Label lblUser;
        private System.Windows.Forms.TextBox txtUser;
        private System.Windows.Forms.Label lblPassword;
        private System.Windows.Forms.TextBox txtPassword;
        private System.Windows.Forms.Button btnConnection;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.PictureBox pictureBox1;
    }
}
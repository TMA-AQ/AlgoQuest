namespace AlgoQuest.UI.Forms
{
    partial class FrmSaveRequest
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
            this.components = new System.ComponentModel.Container();
            this.lblName = new System.Windows.Forms.Label();
            this.tbRequestName = new System.Windows.Forms.TextBox();
            this.lblComment = new System.Windows.Forms.Label();
            this.tbRequestComment = new System.Windows.Forms.TextBox();
            this.btnOk = new System.Windows.Forms.Button();
            this.btnCancel = new System.Windows.Forms.Button();
            this.epRequest = new System.Windows.Forms.ErrorProvider(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.epRequest)).BeginInit();
            this.SuspendLayout();
            // 
            // lblName
            // 
            this.lblName.AutoSize = true;
            this.lblName.Location = new System.Drawing.Point(61, 15);
            this.lblName.Name = "lblName";
            this.lblName.Size = new System.Drawing.Size(29, 13);
            this.lblName.TabIndex = 0;
            this.lblName.Text = "Nom";
            // 
            // tbRequestName
            // 
            this.epRequest.SetError(this.tbRequestName, "Saisissez un nom.");
            this.tbRequestName.Location = new System.Drawing.Point(96, 12);
            this.tbRequestName.Name = "tbRequestName";
            this.tbRequestName.Size = new System.Drawing.Size(224, 20);
            this.tbRequestName.TabIndex = 1;
            // 
            // lblComment
            // 
            this.lblComment.AutoSize = true;
            this.lblComment.Location = new System.Drawing.Point(17, 55);
            this.lblComment.Name = "lblComment";
            this.lblComment.Size = new System.Drawing.Size(73, 13);
            this.lblComment.TabIndex = 2;
            this.lblComment.Text = "Commentaires";
            // 
            // tbRequestComment
            // 
            this.tbRequestComment.Location = new System.Drawing.Point(97, 55);
            this.tbRequestComment.Multiline = true;
            this.tbRequestComment.Name = "tbRequestComment";
            this.tbRequestComment.Size = new System.Drawing.Size(223, 157);
            this.tbRequestComment.TabIndex = 3;
            // 
            // btnOk
            // 
            this.btnOk.Location = new System.Drawing.Point(244, 219);
            this.btnOk.Name = "btnOk";
            this.btnOk.Size = new System.Drawing.Size(75, 23);
            this.btnOk.TabIndex = 4;
            this.btnOk.Text = "OK";
            this.btnOk.UseVisualStyleBackColor = true;
            this.btnOk.Click += new System.EventHandler(this.btnOk_Click);
            // 
            // btnCancel
            // 
            this.btnCancel.DialogResult = System.Windows.Forms.DialogResult.Cancel;
            this.btnCancel.Location = new System.Drawing.Point(96, 218);
            this.btnCancel.Name = "btnCancel";
            this.btnCancel.Size = new System.Drawing.Size(75, 23);
            this.btnCancel.TabIndex = 5;
            this.btnCancel.Text = "Annuler";
            this.btnCancel.UseVisualStyleBackColor = true;
            // 
            // epRequest
            // 
            this.epRequest.ContainerControl = this;
            // 
            // FrmSaveRequest
            // 
            this.AcceptButton = this.btnOk;
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.CancelButton = this.btnCancel;
            this.ClientSize = new System.Drawing.Size(336, 250);
            this.ControlBox = false;
            this.Controls.Add(this.btnCancel);
            this.Controls.Add(this.btnOk);
            this.Controls.Add(this.tbRequestComment);
            this.Controls.Add(this.lblComment);
            this.Controls.Add(this.tbRequestName);
            this.Controls.Add(this.lblName);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.Name = "FrmSaveRequest";
            this.Text = "Sauvegarde de la requête";
            ((System.ComponentModel.ISupportInitialize)(this.epRequest)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Label lblName;
        private System.Windows.Forms.TextBox tbRequestName;
        private System.Windows.Forms.Label lblComment;
        private System.Windows.Forms.TextBox tbRequestComment;
        private System.Windows.Forms.Button btnOk;
        private System.Windows.Forms.Button btnCancel;
        private System.Windows.Forms.ErrorProvider epRequest;
    }
}
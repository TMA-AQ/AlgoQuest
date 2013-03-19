namespace AlgoQuest.UI.Forms
{
    partial class FrmExplorer
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
            System.Windows.Forms.TreeNode treeNode1 = new System.Windows.Forms.TreeNode("Serveur");
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FrmExplorer));
            this.cmsServer = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.tsmCreateDataBase = new System.Windows.Forms.ToolStripMenuItem();
            this.actualiserToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.tvObjects = new System.Windows.Forms.TreeView();
            this.imgTv = new System.Windows.Forms.ImageList(this.components);
            this.cmsDataBase = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.tsmImport = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmExistingFiles = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmConnexion = new System.Windows.Forms.ToolStripMenuItem();
            this.cmsServer.SuspendLayout();
            this.cmsDataBase.SuspendLayout();
            this.SuspendLayout();
            // 
            // cmsServer
            // 
            this.cmsServer.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmCreateDataBase,
            this.actualiserToolStripMenuItem});
            this.cmsServer.Name = "cmsServer";
            this.cmsServer.Size = new System.Drawing.Size(192, 48);
            this.cmsServer.ItemClicked += new System.Windows.Forms.ToolStripItemClickedEventHandler(this.cmsServer_ItemClicked);
            // 
            // tsmCreateDataBase
            // 
            this.tsmCreateDataBase.Name = "tsmCreateDataBase";
            this.tsmCreateDataBase.Size = new System.Drawing.Size(191, 22);
            this.tsmCreateDataBase.Text = "Créer une nouvelle base";
            // 
            // actualiserToolStripMenuItem
            // 
            this.actualiserToolStripMenuItem.Name = "actualiserToolStripMenuItem";
            this.actualiserToolStripMenuItem.Size = new System.Drawing.Size(191, 22);
            this.actualiserToolStripMenuItem.Text = "Actualiser";
            this.actualiserToolStripMenuItem.Click += new System.EventHandler(this.actualiserToolStripMenuItem_Click);
            // 
            // tvObjects
            // 
            this.tvObjects.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.tvObjects.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tvObjects.ImageIndex = 0;
            this.tvObjects.ImageList = this.imgTv;
            this.tvObjects.Location = new System.Drawing.Point(0, 0);
            this.tvObjects.MaximumSize = new System.Drawing.Size(240, 400);
            this.tvObjects.MinimumSize = new System.Drawing.Size(240, 400);
            this.tvObjects.Name = "tvObjects";
            treeNode1.ContextMenuStrip = this.cmsServer;
            treeNode1.Name = "Nœud0";
            treeNode1.Text = "Serveur";
            this.tvObjects.Nodes.AddRange(new System.Windows.Forms.TreeNode[] {
            treeNode1});
            this.tvObjects.SelectedImageIndex = 0;
            this.tvObjects.Size = new System.Drawing.Size(240, 400);
            this.tvObjects.TabIndex = 0;
            this.tvObjects.AfterSelect += new System.Windows.Forms.TreeViewEventHandler(this.tvObjects_AfterSelect);
            this.tvObjects.MouseDown += new System.Windows.Forms.MouseEventHandler(this.tvObjects_MouseDown);
            // 
            // imgTv
            // 
            this.imgTv.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imgTv.ImageStream")));
            this.imgTv.TransparentColor = System.Drawing.Color.Transparent;
            this.imgTv.Images.SetKeyName(0, "Server.ico");
            this.imgTv.Images.SetKeyName(1, "Database.ico");
            this.imgTv.Images.SetKeyName(2, "Folder.ico");
            this.imgTv.Images.SetKeyName(3, "propertysheets.ico");
            // 
            // cmsDataBase
            // 
            this.cmsDataBase.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmImport});
            this.cmsDataBase.Name = "cmsTable";
            this.cmsDataBase.Size = new System.Drawing.Size(166, 26);
            // 
            // tsmImport
            // 
            this.tsmImport.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmExistingFiles,
            this.tsmConnexion});
            this.tsmImport.Name = "tsmImport";
            this.tsmImport.Size = new System.Drawing.Size(165, 22);
            this.tsmImport.Text = "Import de données";
            // 
            // tsmExistingFiles
            // 
            this.tsmExistingFiles.Name = "tsmExistingFiles";
            this.tsmExistingFiles.Size = new System.Drawing.Size(212, 22);
            this.tsmExistingFiles.Text = "A partir de fichiers existants";
            this.tsmExistingFiles.Click += new System.EventHandler(this.tsmExistingFiles_Click);
            // 
            // tsmConnexion
            // 
            this.tsmConnexion.Name = "tsmConnexion";
            this.tsmConnexion.Size = new System.Drawing.Size(212, 22);
            this.tsmConnexion.Text = "Depuis une base de données";
            this.tsmConnexion.Click += new System.EventHandler(this.tsmConnexion_Click);
            // 
            // FrmExplorer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.BackColor = System.Drawing.SystemColors.Window;
            this.ClientSize = new System.Drawing.Size(250, 508);
            this.ControlBox = false;
            this.Controls.Add(this.tvObjects);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D;
            this.MinimumSize = new System.Drawing.Size(260, 450);
            this.Name = "FrmExplorer";
            this.ShowIcon = false;
            this.SizeGripStyle = System.Windows.Forms.SizeGripStyle.Hide;
            this.Text = "Explorateur d\'objets";
            this.cmsServer.ResumeLayout(false);
            this.cmsDataBase.ResumeLayout(false);
            this.ResumeLayout(false);

        }

        #endregion

        private System.Windows.Forms.TreeView tvObjects;
        private System.Windows.Forms.ImageList imgTv;
        private System.Windows.Forms.ContextMenuStrip cmsDataBase;
        private System.Windows.Forms.ToolStripMenuItem tsmImport;
        private System.Windows.Forms.ToolStripMenuItem tsmExistingFiles;
        private System.Windows.Forms.ToolStripMenuItem tsmConnexion;
        private System.Windows.Forms.ContextMenuStrip cmsServer;
        private System.Windows.Forms.ToolStripMenuItem tsmCreateDataBase;
        private System.Windows.Forms.ToolStripMenuItem actualiserToolStripMenuItem;
    }
}
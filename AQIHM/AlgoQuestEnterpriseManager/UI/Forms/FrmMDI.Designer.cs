namespace AlgoQuest.UI.Forms
{
    partial class FrmMdi
    {
        /// <summary>
        /// Variable nécessaire au concepteur.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Nettoyage des ressources utilisées.
        /// </summary>
        /// <param name="disposing">true si les ressources managées doivent être supprimées ; sinon, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Code généré par le Concepteur Windows Form

        /// <summary>
        /// Méthode requise pour la prise en charge du concepteur - ne modifiez pas
        /// le contenu de cette méthode avec l'éditeur de code.
        /// </summary>
        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FrmMdi));
            System.Windows.Forms.TreeNode treeNode2 = new System.Windows.Forms.TreeNode("Serveur");
            this.cmsServer = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.tsmCreateDataBase = new System.Windows.Forms.ToolStripMenuItem();
            this.actualiserToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.menuStrip1 = new System.Windows.Forms.MenuStrip();
            this.fichiersToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.connecterToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.déconnecterToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.ouvrirToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.scriptSQLToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.editionToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.outilsToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.extraireToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.importerToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.iniPropertiesToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.toolStrip1 = new System.Windows.Forms.ToolStrip();
            this.tsbRequest = new System.Windows.Forms.ToolStripButton();
            this.ofdSqlScript = new System.Windows.Forms.OpenFileDialog();
            this.pnlExplorer = new System.Windows.Forms.Panel();
            this.tvObjects = new System.Windows.Forms.TreeView();
            this.imgTv = new System.Windows.Forms.ImageList(this.components);
            this.splitter1 = new System.Windows.Forms.Splitter();
            this.cmsDataBase = new System.Windows.Forms.ContextMenuStrip(this.components);
            this.tsmImport = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmExistingFiles = new System.Windows.Forms.ToolStripMenuItem();
            this.tsmConnexion = new System.Windows.Forms.ToolStripMenuItem();
            this.fenetresToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.réorganiserToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.cascadeToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mosaïqueHorizontaleToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.mosaïqueVerticaleToolStripMenuItem = new System.Windows.Forms.ToolStripMenuItem();
            this.cmsServer.SuspendLayout();
            this.menuStrip1.SuspendLayout();
            this.toolStrip1.SuspendLayout();
            this.pnlExplorer.SuspendLayout();
            this.cmsDataBase.SuspendLayout();
            this.SuspendLayout();
            // 
            // cmsServer
            // 
            this.cmsServer.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmCreateDataBase,
            this.actualiserToolStripMenuItem});
            this.cmsServer.Name = "cmsServer";
            this.cmsServer.Size = new System.Drawing.Size(201, 48);
            // 
            // tsmCreateDataBase
            // 
            this.tsmCreateDataBase.Name = "tsmCreateDataBase";
            this.tsmCreateDataBase.Size = new System.Drawing.Size(200, 22);
            this.tsmCreateDataBase.Text = "Créer une nouvelle base";
            this.tsmCreateDataBase.Click += new System.EventHandler(this.tsmCreateDataBase_Click);
            // 
            // actualiserToolStripMenuItem
            // 
            this.actualiserToolStripMenuItem.Name = "actualiserToolStripMenuItem";
            this.actualiserToolStripMenuItem.Size = new System.Drawing.Size(200, 22);
            this.actualiserToolStripMenuItem.Text = "Actualiser";
            this.actualiserToolStripMenuItem.Click += new System.EventHandler(this.actualiserToolStripMenuItem_Click);
            // 
            // menuStrip1
            // 
            this.menuStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.fichiersToolStripMenuItem,
            this.editionToolStripMenuItem,
            this.outilsToolStripMenuItem,
            this.fenetresToolStripMenuItem});
            this.menuStrip1.Location = new System.Drawing.Point(0, 0);
            this.menuStrip1.Name = "menuStrip1";
            this.menuStrip1.Size = new System.Drawing.Size(999, 24);
            this.menuStrip1.TabIndex = 1;
            this.menuStrip1.Text = "menuStrip1";
            // 
            // fichiersToolStripMenuItem
            // 
            this.fichiersToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.connecterToolStripMenuItem,
            this.déconnecterToolStripMenuItem,
            this.ouvrirToolStripMenuItem});
            this.fichiersToolStripMenuItem.Name = "fichiersToolStripMenuItem";
            this.fichiersToolStripMenuItem.Size = new System.Drawing.Size(54, 20);
            this.fichiersToolStripMenuItem.Text = "Fichier";
            // 
            // connecterToolStripMenuItem
            // 
            this.connecterToolStripMenuItem.Name = "connecterToolStripMenuItem";
            this.connecterToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.connecterToolStripMenuItem.Text = "Connecter";
            this.connecterToolStripMenuItem.Visible = false;
            // 
            // déconnecterToolStripMenuItem
            // 
            this.déconnecterToolStripMenuItem.Name = "déconnecterToolStripMenuItem";
            this.déconnecterToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.déconnecterToolStripMenuItem.Text = "Déconnecter";
            this.déconnecterToolStripMenuItem.Visible = false;
            // 
            // ouvrirToolStripMenuItem
            // 
            this.ouvrirToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.scriptSQLToolStripMenuItem});
            this.ouvrirToolStripMenuItem.Enabled = false;
            this.ouvrirToolStripMenuItem.Name = "ouvrirToolStripMenuItem";
            this.ouvrirToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.ouvrirToolStripMenuItem.Text = "Ouvrir";
            // 
            // scriptSQLToolStripMenuItem
            // 
            this.scriptSQLToolStripMenuItem.Name = "scriptSQLToolStripMenuItem";
            this.scriptSQLToolStripMenuItem.Size = new System.Drawing.Size(128, 22);
            this.scriptSQLToolStripMenuItem.Text = "Script SQL";
            this.scriptSQLToolStripMenuItem.Click += new System.EventHandler(this.scriptSQLToolStripMenuItem_Click);
            // 
            // editionToolStripMenuItem
            // 
            this.editionToolStripMenuItem.Name = "editionToolStripMenuItem";
            this.editionToolStripMenuItem.Size = new System.Drawing.Size(56, 20);
            this.editionToolStripMenuItem.Text = "Edition";
            this.editionToolStripMenuItem.Visible = false;
            // 
            // outilsToolStripMenuItem
            // 
            this.outilsToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.extraireToolStripMenuItem,
            this.importerToolStripMenuItem,
            this.iniPropertiesToolStripMenuItem});
            this.outilsToolStripMenuItem.Name = "outilsToolStripMenuItem";
            this.outilsToolStripMenuItem.Size = new System.Drawing.Size(98, 20);
            this.outilsToolStripMenuItem.Text = "Administration";
            // 
            // extraireToolStripMenuItem
            // 
            this.extraireToolStripMenuItem.Name = "extraireToolStripMenuItem";
            this.extraireToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.extraireToolStripMenuItem.Text = "Extraire";
            this.extraireToolStripMenuItem.Visible = false;
            // 
            // importerToolStripMenuItem
            // 
            this.importerToolStripMenuItem.Name = "importerToolStripMenuItem";
            this.importerToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.importerToolStripMenuItem.Text = "Importer";
            this.importerToolStripMenuItem.Visible = false;
            // 
            // iniPropertiesToolStripMenuItem
            // 
            this.iniPropertiesToolStripMenuItem.Enabled = false;
            this.iniPropertiesToolStripMenuItem.Name = "iniPropertiesToolStripMenuItem";
            this.iniPropertiesToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.iniPropertiesToolStripMenuItem.Text = "Ini.Properties";
            this.iniPropertiesToolStripMenuItem.Click += new System.EventHandler(this.iniPropertiesToolStripMenuItem_Click);
            // 
            // toolStrip1
            // 
            this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsbRequest});
            this.toolStrip1.Location = new System.Drawing.Point(0, 24);
            this.toolStrip1.Name = "toolStrip1";
            this.toolStrip1.Size = new System.Drawing.Size(999, 25);
            this.toolStrip1.TabIndex = 3;
            this.toolStrip1.Text = "toolStrip1";
            // 
            // tsbRequest
            // 
            this.tsbRequest.Enabled = false;
            this.tsbRequest.Image = ((System.Drawing.Image)(resources.GetObject("tsbRequest.Image")));
            this.tsbRequest.ImageTransparentColor = System.Drawing.Color.Magenta;
            this.tsbRequest.Name = "tsbRequest";
            this.tsbRequest.Size = new System.Drawing.Size(117, 22);
            this.tsbRequest.Text = "Nouvelle requête";
            this.tsbRequest.TextImageRelation = System.Windows.Forms.TextImageRelation.TextBeforeImage;
            this.tsbRequest.Click += new System.EventHandler(this.tsbRequest_Click);
            // 
            // ofdSqlScript
            // 
            this.ofdSqlScript.Filter = "fichiers txt (*.txt)|*.txt|scripts SQL (*.sql)|*.sql|Tout fichier|*.*";
            this.ofdSqlScript.FileOk += new System.ComponentModel.CancelEventHandler(this.ofdSqlScript_FileOk);
            // 
            // pnlExplorer
            // 
            this.pnlExplorer.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
            this.pnlExplorer.Controls.Add(this.tvObjects);
            this.pnlExplorer.Dock = System.Windows.Forms.DockStyle.Left;
            this.pnlExplorer.Location = new System.Drawing.Point(0, 49);
            this.pnlExplorer.Name = "pnlExplorer";
            this.pnlExplorer.Size = new System.Drawing.Size(200, 613);
            this.pnlExplorer.TabIndex = 5;
            // 
            // tvObjects
            // 
            this.tvObjects.Dock = System.Windows.Forms.DockStyle.Fill;
            this.tvObjects.ImageIndex = 0;
            this.tvObjects.ImageList = this.imgTv;
            this.tvObjects.Location = new System.Drawing.Point(0, 0);
            this.tvObjects.Name = "tvObjects";
            treeNode2.ContextMenuStrip = this.cmsServer;
            treeNode2.Name = "Nœud0";
            treeNode2.Text = "Serveur";
            this.tvObjects.Nodes.AddRange(new System.Windows.Forms.TreeNode[] {
            treeNode2});
            this.tvObjects.SelectedImageIndex = 0;
            this.tvObjects.Size = new System.Drawing.Size(198, 611);
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
            // splitter1
            // 
            this.splitter1.Location = new System.Drawing.Point(200, 49);
            this.splitter1.Name = "splitter1";
            this.splitter1.Size = new System.Drawing.Size(2, 613);
            this.splitter1.TabIndex = 6;
            this.splitter1.TabStop = false;
            // 
            // cmsDataBase
            // 
            this.cmsDataBase.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmImport});
            this.cmsDataBase.Name = "cmsTable";
            this.cmsDataBase.Size = new System.Drawing.Size(175, 26);
            // 
            // tsmImport
            // 
            this.tsmImport.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.tsmExistingFiles,
            this.tsmConnexion});
            this.tsmImport.Name = "tsmImport";
            this.tsmImport.Size = new System.Drawing.Size(174, 22);
            this.tsmImport.Text = "Import de données";
            // 
            // tsmExistingFiles
            // 
            this.tsmExistingFiles.Name = "tsmExistingFiles";
            this.tsmExistingFiles.Size = new System.Drawing.Size(224, 22);
            this.tsmExistingFiles.Text = "A partir de fichiers existants";
            this.tsmExistingFiles.Click += new System.EventHandler(this.tsmExistingFiles_Click);
            // 
            // tsmConnexion
            // 
            this.tsmConnexion.Name = "tsmConnexion";
            this.tsmConnexion.Size = new System.Drawing.Size(224, 22);
            this.tsmConnexion.Text = "Depuis une base de données";
            this.tsmConnexion.Click += new System.EventHandler(this.tsmConnexion_Click);
            // 
            // fenetresToolStripMenuItem
            // 
            this.fenetresToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.réorganiserToolStripMenuItem});
            this.fenetresToolStripMenuItem.Name = "fenetresToolStripMenuItem";
            this.fenetresToolStripMenuItem.Size = new System.Drawing.Size(63, 20);
            this.fenetresToolStripMenuItem.Text = "Fenêtres";
            // 
            // réorganiserToolStripMenuItem
            // 
            this.réorganiserToolStripMenuItem.DropDownItems.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.cascadeToolStripMenuItem,
            this.mosaïqueHorizontaleToolStripMenuItem,
            this.mosaïqueVerticaleToolStripMenuItem});
            this.réorganiserToolStripMenuItem.Name = "réorganiserToolStripMenuItem";
            this.réorganiserToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.réorganiserToolStripMenuItem.Text = "Réorganiser";
            // 
            // cascadeToolStripMenuItem
            // 
            this.cascadeToolStripMenuItem.Name = "cascadeToolStripMenuItem";
            this.cascadeToolStripMenuItem.Size = new System.Drawing.Size(152, 22);
            this.cascadeToolStripMenuItem.Text = "Cascade";
            this.cascadeToolStripMenuItem.Click += new System.EventHandler(this.cascadeToolStripMenuItem_Click);
            // 
            // mosaïqueHorizontaleToolStripMenuItem
            // 
            this.mosaïqueHorizontaleToolStripMenuItem.Name = "mosaïqueHorizontaleToolStripMenuItem";
            this.mosaïqueHorizontaleToolStripMenuItem.Size = new System.Drawing.Size(188, 22);
            this.mosaïqueHorizontaleToolStripMenuItem.Text = "Mosaïque horizontale";
            this.mosaïqueHorizontaleToolStripMenuItem.Click += new System.EventHandler(this.mosaïqueHorizontaleToolStripMenuItem_Click);
            // 
            // mosaïqueVerticaleToolStripMenuItem
            // 
            this.mosaïqueVerticaleToolStripMenuItem.Name = "mosaïqueVerticaleToolStripMenuItem";
            this.mosaïqueVerticaleToolStripMenuItem.Size = new System.Drawing.Size(188, 22);
            this.mosaïqueVerticaleToolStripMenuItem.Text = "Mosaïque verticale";
            this.mosaïqueVerticaleToolStripMenuItem.Click += new System.EventHandler(this.mosaïqueVerticaleToolStripMenuItem_Click);
            // 
            // FrmMdi
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(999, 662);
            this.Controls.Add(this.splitter1);
            this.Controls.Add(this.pnlExplorer);
            this.Controls.Add(this.toolStrip1);
            this.Controls.Add(this.menuStrip1);
            this.Enabled = false;
            this.IsMdiContainer = true;
            this.MainMenuStrip = this.menuStrip1;
            this.Name = "FrmMdi";
            this.Text = "AlgoQuest Enterprise Manager";
            this.cmsServer.ResumeLayout(false);
            this.menuStrip1.ResumeLayout(false);
            this.menuStrip1.PerformLayout();
            this.toolStrip1.ResumeLayout(false);
            this.toolStrip1.PerformLayout();
            this.pnlExplorer.ResumeLayout(false);
            this.cmsDataBase.ResumeLayout(false);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.MenuStrip menuStrip1;
        private System.Windows.Forms.ToolStripMenuItem fichiersToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem connecterToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem déconnecterToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem ouvrirToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem scriptSQLToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem editionToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem outilsToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem extraireToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem importerToolStripMenuItem;
        private System.Windows.Forms.ToolStrip toolStrip1;
        private System.Windows.Forms.ToolStripButton tsbRequest;
        private System.Windows.Forms.ToolStripMenuItem iniPropertiesToolStripMenuItem;
        private System.Windows.Forms.OpenFileDialog ofdSqlScript;
        private System.Windows.Forms.Panel pnlExplorer;
        private System.Windows.Forms.TreeView tvObjects;
        private System.Windows.Forms.Splitter splitter1;
        private System.Windows.Forms.ContextMenuStrip cmsServer;
        private System.Windows.Forms.ToolStripMenuItem tsmCreateDataBase;
        private System.Windows.Forms.ToolStripMenuItem actualiserToolStripMenuItem;
        private System.Windows.Forms.ContextMenuStrip cmsDataBase;
        private System.Windows.Forms.ToolStripMenuItem tsmImport;
        private System.Windows.Forms.ToolStripMenuItem tsmExistingFiles;
        private System.Windows.Forms.ToolStripMenuItem tsmConnexion;
        private System.Windows.Forms.ImageList imgTv;
        private System.Windows.Forms.ToolStripMenuItem fenetresToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem réorganiserToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem cascadeToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem mosaïqueHorizontaleToolStripMenuItem;
        private System.Windows.Forms.ToolStripMenuItem mosaïqueVerticaleToolStripMenuItem;
    }
}


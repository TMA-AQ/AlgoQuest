namespace AlgoQuest.UI.Forms.Import
{
    partial class FrmDatabaseImportExplorer
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
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(FrmDatabaseImportExplorer));
            System.Windows.Forms.TreeNode treeNode1 = new System.Windows.Forms.TreeNode("");
            this.imgTv = new System.Windows.Forms.ImageList(this.components);
            this.tvObjects = new System.Windows.Forms.TreeView();
            this.btnNext = new System.Windows.Forms.Button();
            this.btnPrevious = new System.Windows.Forms.Button();
            this.dgvDataType = new System.Windows.Forms.DataGridView();
            this.DataTableColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DataColumnColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DataTypeSourcesColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DataTypeLengthSourceColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.DataTypeTargetColumn = new System.Windows.Forms.DataGridViewComboBoxColumn();
            this.DataTypeLengthTargetColumn = new System.Windows.Forms.DataGridViewTextBoxColumn();
            this.label1 = new System.Windows.Forms.Label();
            this.shapeContainer1 = new Microsoft.VisualBasic.PowerPacks.ShapeContainer();
            this.lineShape1 = new Microsoft.VisualBasic.PowerPacks.LineShape();
            this.label2 = new System.Windows.Forms.Label();
            this.frmDatabaseImportExplorerBindingSource = new System.Windows.Forms.BindingSource(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.dgvDataType)).BeginInit();
            ((System.ComponentModel.ISupportInitialize)(this.frmDatabaseImportExplorerBindingSource)).BeginInit();
            this.SuspendLayout();
            // 
            // imgTv
            // 
            this.imgTv.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imgTv.ImageStream")));
            this.imgTv.TransparentColor = System.Drawing.Color.Transparent;
            this.imgTv.Images.SetKeyName(0, "Database.ico");
            this.imgTv.Images.SetKeyName(1, "Folder.ico");
            this.imgTv.Images.SetKeyName(2, "propertysheets.ico");
            // 
            // tvObjects
            // 
            this.tvObjects.BorderStyle = System.Windows.Forms.BorderStyle.None;
            this.tvObjects.CheckBoxes = true;
            this.tvObjects.ImageIndex = 0;
            this.tvObjects.ImageList = this.imgTv;
            this.tvObjects.Location = new System.Drawing.Point(0, 0);
            this.tvObjects.MaximumSize = new System.Drawing.Size(240, 400);
            this.tvObjects.MinimumSize = new System.Drawing.Size(240, 400);
            this.tvObjects.Name = "tvObjects";
            treeNode1.Name = "Nœud0";
            treeNode1.Text = "";
            this.tvObjects.Nodes.AddRange(new System.Windows.Forms.TreeNode[] {
            treeNode1});
            this.tvObjects.SelectedImageIndex = 0;
            this.tvObjects.Size = new System.Drawing.Size(240, 400);
            this.tvObjects.TabIndex = 1;
            this.tvObjects.AfterCheck += new System.Windows.Forms.TreeViewEventHandler(this.tvObjects_AfterCheck);
            // 
            // btnNext
            // 
            this.btnNext.Location = new System.Drawing.Point(142, 406);
            this.btnNext.Name = "btnNext";
            this.btnNext.Size = new System.Drawing.Size(87, 23);
            this.btnNext.TabIndex = 52;
            this.btnNext.Text = "Suivant";
            this.btnNext.UseVisualStyleBackColor = true;
            this.btnNext.Click += new System.EventHandler(this.btnNext_Click);
            // 
            // btnPrevious
            // 
            this.btnPrevious.Location = new System.Drawing.Point(12, 406);
            this.btnPrevious.Name = "btnPrevious";
            this.btnPrevious.Size = new System.Drawing.Size(87, 23);
            this.btnPrevious.TabIndex = 53;
            this.btnPrevious.Text = "Précédent";
            this.btnPrevious.UseVisualStyleBackColor = true;
            this.btnPrevious.Click += new System.EventHandler(this.btnPrevious_Click);
            // 
            // dgvDataType
            // 
            this.dgvDataType.AllowUserToAddRows = false;
            this.dgvDataType.AllowUserToResizeColumns = false;
            this.dgvDataType.AllowUserToResizeRows = false;
            this.dgvDataType.BackgroundColor = System.Drawing.SystemColors.ControlLightLight;
            this.dgvDataType.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
            this.dgvDataType.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.DataTableColumn,
            this.DataColumnColumn,
            this.DataTypeSourcesColumn,
            this.DataTypeLengthSourceColumn,
            this.DataTypeTargetColumn,
            this.DataTypeLengthTargetColumn});
            this.dgvDataType.EditMode = System.Windows.Forms.DataGridViewEditMode.EditOnEnter;
            this.dgvDataType.Location = new System.Drawing.Point(246, 27);
            this.dgvDataType.Name = "dgvDataType";
            this.dgvDataType.Size = new System.Drawing.Size(644, 372);
            this.dgvDataType.TabIndex = 54;
            this.dgvDataType.CellEndEdit += new System.Windows.Forms.DataGridViewCellEventHandler(this.dgvDataType_CellEndEdit);
            this.dgvDataType.DataError += new System.Windows.Forms.DataGridViewDataErrorEventHandler(this.dgvDataType_DataError);
            // 
            // DataTableColumn
            // 
            this.DataTableColumn.HeaderText = "Table";
            this.DataTableColumn.Name = "DataTableColumn";
            this.DataTableColumn.ReadOnly = true;
            // 
            // DataColumnColumn
            // 
            this.DataColumnColumn.HeaderText = "Colonne";
            this.DataColumnColumn.Name = "DataColumnColumn";
            this.DataColumnColumn.ReadOnly = true;
            // 
            // DataTypeSourcesColumn
            // 
            this.DataTypeSourcesColumn.HeaderText = "Source";
            this.DataTypeSourcesColumn.Name = "DataTypeSourcesColumn";
            this.DataTypeSourcesColumn.ReadOnly = true;
            // 
            // DataTypeLengthSourceColumn
            // 
            this.DataTypeLengthSourceColumn.HeaderText = "Longueur";
            this.DataTypeLengthSourceColumn.Name = "DataTypeLengthSourceColumn";
            this.DataTypeLengthSourceColumn.ReadOnly = true;
            // 
            // DataTypeTargetColumn
            // 
            this.DataTypeTargetColumn.HeaderText = "Destination";
            this.DataTypeTargetColumn.Name = "DataTypeTargetColumn";
            this.DataTypeTargetColumn.Resizable = System.Windows.Forms.DataGridViewTriState.True;
            this.DataTypeTargetColumn.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.Automatic;
            // 
            // DataTypeLengthTargetColumn
            // 
            this.DataTypeLengthTargetColumn.HeaderText = "Longueur";
            this.DataTypeLengthTargetColumn.Name = "DataTypeLengthTargetColumn";
            this.DataTypeLengthTargetColumn.Resizable = System.Windows.Forms.DataGridViewTriState.True;
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Location = new System.Drawing.Point(367, 8);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(140, 13);
            this.label1.TabIndex = 55;
            this.label1.Text = "Types de données \"source\"";
            // 
            // shapeContainer1
            // 
            this.shapeContainer1.Location = new System.Drawing.Point(0, 0);
            this.shapeContainer1.Margin = new System.Windows.Forms.Padding(0);
            this.shapeContainer1.Name = "shapeContainer1";
            this.shapeContainer1.Shapes.AddRange(new Microsoft.VisualBasic.PowerPacks.Shape[] {
            this.lineShape1});
            this.shapeContainer1.Size = new System.Drawing.Size(904, 429);
            this.shapeContainer1.TabIndex = 56;
            this.shapeContainer1.TabStop = false;
            // 
            // lineShape1
            // 
            this.lineShape1.Name = "lineShape1";
            this.lineShape1.X1 = 687;
            this.lineShape1.X2 = 687;
            this.lineShape1.Y1 = 2;
            this.lineShape1.Y2 = 24;
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Location = new System.Drawing.Point(707, 8);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(159, 13);
            this.label2.TabIndex = 57;
            this.label2.Text = "Types de données \"destination\"";
            // 
            // frmDatabaseImportExplorerBindingSource
            // 
            this.frmDatabaseImportExplorerBindingSource.DataSource = typeof(AlgoQuest.UI.Forms.Import.FrmDatabaseImportExplorer);
            // 
            // FrmDatabaseImportExplorer
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(904, 429);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.dgvDataType);
            this.Controls.Add(this.btnPrevious);
            this.Controls.Add(this.btnNext);
            this.Controls.Add(this.tvObjects);
            this.Controls.Add(this.shapeContainer1);
            this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedToolWindow;
            this.Name = "FrmDatabaseImportExplorer";
            this.Text = "Explorateur de base";
            ((System.ComponentModel.ISupportInitialize)(this.dgvDataType)).EndInit();
            ((System.ComponentModel.ISupportInitialize)(this.frmDatabaseImportExplorerBindingSource)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.ImageList imgTv;
        private System.Windows.Forms.TreeView tvObjects;
        private System.Windows.Forms.Button btnNext;
        private System.Windows.Forms.Button btnPrevious;
        private System.Windows.Forms.DataGridView dgvDataType;
        private System.Windows.Forms.Label label1;
        private Microsoft.VisualBasic.PowerPacks.ShapeContainer shapeContainer1;
        private Microsoft.VisualBasic.PowerPacks.LineShape lineShape1;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.BindingSource frmDatabaseImportExplorerBindingSource;
        private System.Windows.Forms.DataGridViewTextBoxColumn DataTableColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn DataColumnColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn DataTypeSourcesColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn DataTypeLengthSourceColumn;
        private System.Windows.Forms.DataGridViewComboBoxColumn DataTypeTargetColumn;
        private System.Windows.Forms.DataGridViewTextBoxColumn DataTypeLengthTargetColumn;

    }
}
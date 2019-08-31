namespace PDBManager
{
  partial class NewTableForm
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
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(NewTableForm));
      this.txbTabName = new System.Windows.Forms.TextBox();
      this.fieldImgList = new System.Windows.Forms.ImageList(this.components);
      this.label2 = new System.Windows.Forms.Label();
      this.groupBox1 = new System.Windows.Forms.GroupBox();
      this.DGVFields = new System.Windows.Forms.DataGridView();
      this.btnCreateTable = new System.Windows.Forms.Button();
      this.btnCancel = new System.Windows.Forms.Button();
      this.fieldName = new System.Windows.Forms.DataGridViewTextBoxColumn();
      this.fieldType = new System.Windows.Forms.DataGridViewComboBoxColumn();
      this.groupBox1.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)(this.DGVFields)).BeginInit();
      this.SuspendLayout();
      // 
      // txbTabName
      // 
      this.txbTabName.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
      this.txbTabName.Location = new System.Drawing.Point(66, 25);
      this.txbTabName.Name = "txbTabName";
      this.txbTabName.Size = new System.Drawing.Size(360, 21);
      this.txbTabName.TabIndex = 1;
      // 
      // fieldImgList
      // 
      this.fieldImgList.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("fieldImgList.ImageStream")));
      this.fieldImgList.TransparentColor = System.Drawing.Color.Transparent;
      this.fieldImgList.Images.SetKeyName(0, "field_key_white.png");
      this.fieldImgList.Images.SetKeyName(1, "field.png");
      // 
      // label2
      // 
      this.label2.AutoSize = true;
      this.label2.Location = new System.Drawing.Point(25, 28);
      this.label2.Name = "label2";
      this.label2.Size = new System.Drawing.Size(35, 12);
      this.label2.TabIndex = 6;
      this.label2.Text = "表名:";
      // 
      // groupBox1
      // 
      this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
      this.groupBox1.Controls.Add(this.DGVFields);
      this.groupBox1.ImeMode = System.Windows.Forms.ImeMode.Off;
      this.groupBox1.Location = new System.Drawing.Point(27, 63);
      this.groupBox1.Name = "groupBox1";
      this.groupBox1.Size = new System.Drawing.Size(399, 311);
      this.groupBox1.TabIndex = 5;
      this.groupBox1.TabStop = false;
      this.groupBox1.Text = "字段";
      // 
      // DGVFields
      // 
      this.DGVFields.Anchor = ((System.Windows.Forms.AnchorStyles)((((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Bottom) 
            | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
      this.DGVFields.ColumnHeadersHeightSizeMode = System.Windows.Forms.DataGridViewColumnHeadersHeightSizeMode.AutoSize;
      this.DGVFields.Columns.AddRange(new System.Windows.Forms.DataGridViewColumn[] {
            this.fieldName,
            this.fieldType});
      this.DGVFields.EditMode = System.Windows.Forms.DataGridViewEditMode.EditOnEnter;
      this.DGVFields.GridColor = System.Drawing.SystemColors.ActiveBorder;
      this.DGVFields.Location = new System.Drawing.Point(6, 20);
      this.DGVFields.Name = "DGVFields";
      this.DGVFields.RowTemplate.Height = 23;
      this.DGVFields.Size = new System.Drawing.Size(387, 274);
      this.DGVFields.TabIndex = 8;
      // 
      // btnCreateTable
      // 
      this.btnCreateTable.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.btnCreateTable.Image = global::PDBManager.Properties.Resources.accept;
      this.btnCreateTable.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
      this.btnCreateTable.Location = new System.Drawing.Point(261, 388);
      this.btnCreateTable.Name = "btnCreateTable";
      this.btnCreateTable.Padding = new System.Windows.Forms.Padding(8, 0, 0, 0);
      this.btnCreateTable.Size = new System.Drawing.Size(75, 23);
      this.btnCreateTable.TabIndex = 6;
      this.btnCreateTable.Text = "创建 ";
      this.btnCreateTable.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
      this.btnCreateTable.UseVisualStyleBackColor = true;
      this.btnCreateTable.Click += new System.EventHandler(this.btnCreateTable_Click);
      // 
      // btnCancel
      // 
      this.btnCancel.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Bottom | System.Windows.Forms.AnchorStyles.Right)));
      this.btnCancel.Image = global::PDBManager.Properties.Resources.del;
      this.btnCancel.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
      this.btnCancel.Location = new System.Drawing.Point(351, 388);
      this.btnCancel.Name = "btnCancel";
      this.btnCancel.Padding = new System.Windows.Forms.Padding(8, 0, 0, 0);
      this.btnCancel.Size = new System.Drawing.Size(75, 23);
      this.btnCancel.TabIndex = 7;
      this.btnCancel.Text = "取消 ";
      this.btnCancel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
      this.btnCancel.UseVisualStyleBackColor = true;
      this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
      // 
      // fieldName
      // 
      this.fieldName.HeaderText = "字段名";
      this.fieldName.Name = "fieldName";
      this.fieldName.SortMode = System.Windows.Forms.DataGridViewColumnSortMode.NotSortable;
      this.fieldName.Width = 200;
      // 
      // fieldType
      // 
      this.fieldType.HeaderText = "字段类型";
      this.fieldType.Items.AddRange(new object[] {
            "bool",
            "bigint",
            "datetime",
            "double",
            "string",
            "blob",
            "real2",
            "real3",
            "real4",
            "real6"});
      this.fieldType.Name = "fieldType";
      this.fieldType.Resizable = System.Windows.Forms.DataGridViewTriState.True;
      this.fieldType.Width = 200;
      // 
      // NewTableForm
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(453, 427);
      this.Controls.Add(this.btnCreateTable);
      this.Controls.Add(this.label2);
      this.Controls.Add(this.groupBox1);
      this.Controls.Add(this.btnCancel);
      this.Controls.Add(this.txbTabName);
      this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
      this.Name = "NewTableForm";
      this.Load += new System.EventHandler(this.NewTableForm_Load);
      this.groupBox1.ResumeLayout(false);
      ((System.ComponentModel.ISupportInitialize)(this.DGVFields)).EndInit();
      this.ResumeLayout(false);
      this.PerformLayout();

    }

    #endregion

    private System.Windows.Forms.TextBox txbTabName;
    private System.Windows.Forms.Button btnCancel;
    private System.Windows.Forms.Label label2;
    private System.Windows.Forms.Button btnCreateTable;
    private System.Windows.Forms.ImageList fieldImgList;
    private System.Windows.Forms.GroupBox groupBox1;
    private System.Windows.Forms.DataGridView DGVFields;
    private System.Windows.Forms.DataGridViewTextBoxColumn fieldName;
    private System.Windows.Forms.DataGridViewComboBoxColumn fieldType;
  }
}
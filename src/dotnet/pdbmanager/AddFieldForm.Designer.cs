namespace PDBManager
{
  partial class AddFieldForm
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
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AddFieldForm));
      this.txbFieldName = new System.Windows.Forms.TextBox();
      this.cmbFieldType = new System.Windows.Forms.ComboBox();
      this.btnCancel = new System.Windows.Forms.Button();
      this.btnAccept = new System.Windows.Forms.Button();
      this.label2 = new System.Windows.Forms.Label();
      this.label1 = new System.Windows.Forms.Label();
      this.SuspendLayout();
      // 
      // txbFieldName
      // 
      this.txbFieldName.Location = new System.Drawing.Point(113, 29);
      this.txbFieldName.Name = "txbFieldName";
      this.txbFieldName.Size = new System.Drawing.Size(136, 21);
      this.txbFieldName.TabIndex = 2;
      this.txbFieldName.KeyUp += new System.Windows.Forms.KeyEventHandler(this.txbFieldName_KeyUp);
      // 
      // cmbFieldType
      // 
      this.cmbFieldType.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList;
      this.cmbFieldType.FormattingEnabled = true;
      this.cmbFieldType.Items.AddRange(new object[] {
            "bigint",
            "float",
            "double",
            "string",
            "blob"});
      this.cmbFieldType.Location = new System.Drawing.Point(113, 65);
      this.cmbFieldType.Name = "cmbFieldType";
      this.cmbFieldType.Size = new System.Drawing.Size(136, 20);
      this.cmbFieldType.TabIndex = 3;
      this.cmbFieldType.KeyUp += new System.Windows.Forms.KeyEventHandler(this.cmbFieldType_KeyUp);
      // 
      // btnCancel
      // 
      this.btnCancel.Image = global::PDBManager.Properties.Resources.del;
      this.btnCancel.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
      this.btnCancel.Location = new System.Drawing.Point(174, 119);
      this.btnCancel.Name = "btnCancel";
      this.btnCancel.Padding = new System.Windows.Forms.Padding(8, 0, 8, 0);
      this.btnCancel.Size = new System.Drawing.Size(75, 23);
      this.btnCancel.TabIndex = 5;
      this.btnCancel.Text = "取消";
      this.btnCancel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
      this.btnCancel.UseVisualStyleBackColor = true;
      this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
      // 
      // btnAccept
      // 
      this.btnAccept.Image = global::PDBManager.Properties.Resources.accept;
      this.btnAccept.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
      this.btnAccept.Location = new System.Drawing.Point(80, 119);
      this.btnAccept.Name = "btnAccept";
      this.btnAccept.Padding = new System.Windows.Forms.Padding(8, 0, 8, 0);
      this.btnAccept.Size = new System.Drawing.Size(75, 23);
      this.btnAccept.TabIndex = 4;
      this.btnAccept.Text = "确定";
      this.btnAccept.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
      this.btnAccept.UseVisualStyleBackColor = true;
      this.btnAccept.Click += new System.EventHandler(this.btnAccept_Click);
      // 
      // label2
      // 
      this.label2.AutoSize = true;
      this.label2.Image = global::PDBManager.Properties.Resources.edit_field;
      this.label2.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
      this.label2.Location = new System.Drawing.Point(12, 68);
      this.label2.Name = "label2";
      this.label2.Size = new System.Drawing.Size(83, 12);
      this.label2.TabIndex = 1;
      this.label2.Text = "    字段类型:";
      this.label2.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
      // 
      // label1
      // 
      this.label1.AutoSize = true;
      this.label1.Image = global::PDBManager.Properties.Resources.field;
      this.label1.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
      this.label1.Location = new System.Drawing.Point(12, 32);
      this.label1.Name = "label1";
      this.label1.Size = new System.Drawing.Size(71, 12);
      this.label1.TabIndex = 0;
      this.label1.Text = "    字段名:";
      this.label1.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
      // 
      // AddFieldForm
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(287, 154);
      this.Controls.Add(this.btnCancel);
      this.Controls.Add(this.btnAccept);
      this.Controls.Add(this.cmbFieldType);
      this.Controls.Add(this.txbFieldName);
      this.Controls.Add(this.label2);
      this.Controls.Add(this.label1);
      this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
      this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
      this.MaximizeBox = false;
      this.MinimizeBox = false;
      this.Name = "AddFieldForm";
      this.Text = "添加字段";
      this.Load += new System.EventHandler(this.AddFieldForm_Load);
      this.ResumeLayout(false);
      this.PerformLayout();

    }

    #endregion

    private System.Windows.Forms.Label label1;
    private System.Windows.Forms.Label label2;
    private System.Windows.Forms.TextBox txbFieldName;
    private System.Windows.Forms.ComboBox cmbFieldType;
    private System.Windows.Forms.Button btnAccept;
    private System.Windows.Forms.Button btnCancel;
  }
}
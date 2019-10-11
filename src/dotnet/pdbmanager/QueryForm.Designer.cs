namespace PDBManager
{
  partial class QueryForm
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
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(QueryForm));
      this.toolStrip1 = new System.Windows.Forms.ToolStrip();
      this.btnExecute = new System.Windows.Forms.ToolStripButton();
      this.toolStripSeparator1 = new System.Windows.Forms.ToolStripSeparator();
      this.btnOpenFile = new System.Windows.Forms.ToolStripButton();
      this.btnSaveFile = new System.Windows.Forms.ToolStripButton();
      this.toolStripSeparator2 = new System.Windows.Forms.ToolStripSeparator();
      this.btnSaveResult = new System.Windows.Forms.ToolStripButton();
      this.splitContainer1 = new System.Windows.Forms.SplitContainer();
      this.sqlContext = new ICSharpCode.TextEditor.TextEditorControl();
      this.tabControlResult = new System.Windows.Forms.TabControl();
      this.tabPageData = new System.Windows.Forms.TabPage();
      this.lvResult = new System.Windows.Forms.ListView();
      this.tabPageMsg = new System.Windows.Forms.TabPage();
      this.richTxtMsg = new System.Windows.Forms.RichTextBox();
      this.imgListQuery = new System.Windows.Forms.ImageList(this.components);
      this.toolStrip1.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).BeginInit();
      this.splitContainer1.Panel1.SuspendLayout();
      this.splitContainer1.Panel2.SuspendLayout();
      this.splitContainer1.SuspendLayout();
      this.tabControlResult.SuspendLayout();
      this.tabPageData.SuspendLayout();
      this.tabPageMsg.SuspendLayout();
      this.SuspendLayout();
      // 
      // toolStrip1
      // 
      this.toolStrip1.Items.AddRange(new System.Windows.Forms.ToolStripItem[] {
            this.btnExecute,
            this.toolStripSeparator1,
            this.btnOpenFile,
            this.btnSaveFile,
            this.toolStripSeparator2,
            this.btnSaveResult});
      this.toolStrip1.Location = new System.Drawing.Point(0, 0);
      this.toolStrip1.Name = "toolStrip1";
      this.toolStrip1.Size = new System.Drawing.Size(584, 25);
      this.toolStrip1.TabIndex = 1;
      this.toolStrip1.Text = "toolStrip1";
      // 
      // btnExecute
      // 
      this.btnExecute.Image = global::PDBManager.Properties.Resources.execute;
      this.btnExecute.ImageTransparentColor = System.Drawing.Color.Magenta;
      this.btnExecute.Name = "btnExecute";
      this.btnExecute.Size = new System.Drawing.Size(52, 22);
      this.btnExecute.Text = "执行";
      this.btnExecute.Click += new System.EventHandler(this.btnExecute_Click);
      // 
      // toolStripSeparator1
      // 
      this.toolStripSeparator1.Name = "toolStripSeparator1";
      this.toolStripSeparator1.Size = new System.Drawing.Size(6, 25);
      // 
      // btnOpenFile
      // 
      this.btnOpenFile.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
      this.btnOpenFile.Image = global::PDBManager.Properties.Resources.open_file;
      this.btnOpenFile.ImageTransparentColor = System.Drawing.Color.Magenta;
      this.btnOpenFile.Name = "btnOpenFile";
      this.btnOpenFile.Size = new System.Drawing.Size(23, 22);
      this.btnOpenFile.Text = "打开";
      this.btnOpenFile.Click += new System.EventHandler(this.btnOpenFile_Click);
      // 
      // btnSaveFile
      // 
      this.btnSaveFile.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
      this.btnSaveFile.Image = global::PDBManager.Properties.Resources.save;
      this.btnSaveFile.ImageTransparentColor = System.Drawing.Color.Magenta;
      this.btnSaveFile.Name = "btnSaveFile";
      this.btnSaveFile.Size = new System.Drawing.Size(23, 22);
      this.btnSaveFile.Text = "保存";
      this.btnSaveFile.Click += new System.EventHandler(this.btnSaveFile_Click);
      // 
      // toolStripSeparator2
      // 
      this.toolStripSeparator2.Name = "toolStripSeparator2";
      this.toolStripSeparator2.Size = new System.Drawing.Size(6, 25);
      // 
      // btnSaveResult
      // 
      this.btnSaveResult.DisplayStyle = System.Windows.Forms.ToolStripItemDisplayStyle.Image;
      this.btnSaveResult.Image = global::PDBManager.Properties.Resources.save_result;
      this.btnSaveResult.ImageTransparentColor = System.Drawing.Color.Magenta;
      this.btnSaveResult.Name = "btnSaveResult";
      this.btnSaveResult.Size = new System.Drawing.Size(23, 22);
      this.btnSaveResult.Text = "将结果保存到文件";
      this.btnSaveResult.Click += new System.EventHandler(this.btnSavaResult_Click);
      // 
      // splitContainer1
      // 
      this.splitContainer1.Dock = System.Windows.Forms.DockStyle.Fill;
      this.splitContainer1.Location = new System.Drawing.Point(0, 25);
      this.splitContainer1.Name = "splitContainer1";
      this.splitContainer1.Orientation = System.Windows.Forms.Orientation.Horizontal;
      // 
      // splitContainer1.Panel1
      // 
      this.splitContainer1.Panel1.Controls.Add(this.sqlContext);
      // 
      // splitContainer1.Panel2
      // 
      this.splitContainer1.Panel2.Controls.Add(this.tabControlResult);
      this.splitContainer1.Size = new System.Drawing.Size(584, 337);
      this.splitContainer1.SplitterDistance = 115;
      this.splitContainer1.SplitterWidth = 12;
      this.splitContainer1.TabIndex = 2;
      // 
      // sqlContext
      // 
      this.sqlContext.Font = new System.Drawing.Font("Courier New", 13);
      this.sqlContext.BackColor = System.Drawing.SystemColors.Control;
      this.sqlContext.Dock = System.Windows.Forms.DockStyle.Fill;
      this.sqlContext.Encoding = ((System.Text.Encoding)(resources.GetObject("sqlContext.Encoding")));
      this.sqlContext.Location = new System.Drawing.Point(0, 0);
      this.sqlContext.Name = "sqlContext";
      this.sqlContext.ShowEOLMarkers = true;
      this.sqlContext.ShowSpaces = true;
      this.sqlContext.ShowTabs = true;
      this.sqlContext.ShowVRuler = true;
      this.sqlContext.Size = new System.Drawing.Size(584, 115);
      this.sqlContext.TabIndex = 3;
      // 
      // tabControlResult
      // 
      this.tabControlResult.Controls.Add(this.tabPageData);
      this.tabControlResult.Controls.Add(this.tabPageMsg);
      this.tabControlResult.Dock = System.Windows.Forms.DockStyle.Fill;
      this.tabControlResult.ImageList = this.imgListQuery;
      this.tabControlResult.Location = new System.Drawing.Point(0, 0);
      this.tabControlResult.Name = "tabControlResult";
      this.tabControlResult.SelectedIndex = 0;
      this.tabControlResult.Size = new System.Drawing.Size(584, 210);
      this.tabControlResult.TabIndex = 3;
      // 
      // tabPageData
      // 
      this.tabPageData.Controls.Add(this.lvResult);
      this.tabPageData.ImageIndex = 0;
      this.tabPageData.Location = new System.Drawing.Point(4, 23);
      this.tabPageData.Name = "tabPageData";
      this.tabPageData.Padding = new System.Windows.Forms.Padding(3);
      this.tabPageData.Size = new System.Drawing.Size(576, 183);
      this.tabPageData.TabIndex = 0;
      this.tabPageData.Text = "结果";
      this.tabPageData.UseVisualStyleBackColor = true;
      // 
      // lvResult
      // 
      this.lvResult.Dock = System.Windows.Forms.DockStyle.Fill;
      this.lvResult.FullRowSelect = true;
      this.lvResult.GridLines = true;
      this.lvResult.HeaderStyle = System.Windows.Forms.ColumnHeaderStyle.Nonclickable;
      this.lvResult.Location = new System.Drawing.Point(3, 3);
      this.lvResult.Name = "lvResult";
      this.lvResult.Size = new System.Drawing.Size(570, 177);
      this.lvResult.TabIndex = 0;
      this.lvResult.UseCompatibleStateImageBehavior = false;
      this.lvResult.View = System.Windows.Forms.View.Details;
      // 
      // tabPageMsg
      // 
      this.tabPageMsg.Controls.Add(this.richTxtMsg);
      this.tabPageMsg.ImageIndex = 1;
      this.tabPageMsg.Location = new System.Drawing.Point(4, 23);
      this.tabPageMsg.Name = "tabPageMsg";
      this.tabPageMsg.Padding = new System.Windows.Forms.Padding(3);
      this.tabPageMsg.Size = new System.Drawing.Size(576, 183);
      this.tabPageMsg.TabIndex = 1;
      this.tabPageMsg.Text = "消息";
      this.tabPageMsg.UseVisualStyleBackColor = true;
      // 
      // richTxtMsg
      // 
      this.richTxtMsg.Dock = System.Windows.Forms.DockStyle.Fill;
      this.richTxtMsg.Location = new System.Drawing.Point(3, 3);
      this.richTxtMsg.Name = "richTxtMsg";
      this.richTxtMsg.ReadOnly = true;
      this.richTxtMsg.Size = new System.Drawing.Size(570, 177);
      this.richTxtMsg.TabIndex = 4;
      this.richTxtMsg.Text = "";
      // 
      // imgListQuery
      // 
      this.imgListQuery.ImageStream = ((System.Windows.Forms.ImageListStreamer)(resources.GetObject("imgListQuery.ImageStream")));
      this.imgListQuery.TransparentColor = System.Drawing.Color.Transparent;
      this.imgListQuery.Images.SetKeyName(0, "result.png");
      this.imgListQuery.Images.SetKeyName(1, "message.png");
      // 
      // QueryForm
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(584, 362);
      this.Controls.Add(this.splitContainer1);
      this.Controls.Add(this.toolStrip1);
      this.Name = "QueryForm";
      this.toolStrip1.ResumeLayout(false);
      this.toolStrip1.PerformLayout();
      this.splitContainer1.Panel1.ResumeLayout(false);
      this.splitContainer1.Panel2.ResumeLayout(false);
      ((System.ComponentModel.ISupportInitialize)(this.splitContainer1)).EndInit();
      this.splitContainer1.ResumeLayout(false);
      this.tabControlResult.ResumeLayout(false);
      this.tabPageData.ResumeLayout(false);
      this.tabPageMsg.ResumeLayout(false);
      this.ResumeLayout(false);
      this.PerformLayout();

    }

    #endregion

    private System.Windows.Forms.ToolStrip toolStrip1;
    private System.Windows.Forms.ToolStripButton btnExecute;
    private System.Windows.Forms.SplitContainer splitContainer1;
    private System.Windows.Forms.TabControl tabControlResult;
    private System.Windows.Forms.TabPage tabPageData;
    private System.Windows.Forms.TabPage tabPageMsg;
    private System.Windows.Forms.ListView lvResult;
    private System.Windows.Forms.RichTextBox richTxtMsg;
    private System.Windows.Forms.ImageList imgListQuery;
    private System.Windows.Forms.ToolStripSeparator toolStripSeparator1;
    private System.Windows.Forms.ToolStripButton btnOpenFile;
    private System.Windows.Forms.ToolStripButton btnSaveFile;
    private System.Windows.Forms.ToolStripSeparator toolStripSeparator2;
    private System.Windows.Forms.ToolStripButton btnSaveResult;
    private ICSharpCode.TextEditor.TextEditorControl sqlContext;
  }
}
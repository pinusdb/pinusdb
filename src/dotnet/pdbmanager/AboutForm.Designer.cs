namespace PDBManager
{
  partial class AboutForm
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
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(AboutForm));
      this.label1 = new System.Windows.Forms.Label();
      this.label2 = new System.Windows.Forms.Label();
      this.lblDBVersion = new System.Windows.Forms.Label();
      this.lblToolVer = new System.Windows.Forms.Label();
      this.label3 = new System.Windows.Forms.Label();
      this.label4 = new System.Windows.Forms.Label();
      this.label5 = new System.Windows.Forms.Label();
      this.label6 = new System.Windows.Forms.Label();
      this.btnConfirm = new System.Windows.Forms.Button();
      this.label7 = new System.Windows.Forms.Label();
      this.SuspendLayout();
      // 
      // label1
      // 
      this.label1.AutoSize = true;
      this.label1.Location = new System.Drawing.Point(64, 61);
      this.label1.Name = "label1";
      this.label1.Size = new System.Drawing.Size(71, 12);
      this.label1.TabIndex = 0;
      this.label1.Text = "数据库版本:";
      // 
      // label2
      // 
      this.label2.AutoSize = true;
      this.label2.Location = new System.Drawing.Point(52, 90);
      this.label2.Name = "label2";
      this.label2.Size = new System.Drawing.Size(83, 12);
      this.label2.TabIndex = 1;
      this.label2.Text = "管理工具版本:";
      // 
      // lblDBVersion
      // 
      this.lblDBVersion.AutoSize = true;
      this.lblDBVersion.Location = new System.Drawing.Point(156, 61);
      this.lblDBVersion.Name = "lblDBVersion";
      this.lblDBVersion.Size = new System.Drawing.Size(17, 12);
      this.lblDBVersion.TabIndex = 2;
      this.lblDBVersion.Text = "  ";
      // 
      // lblToolVer
      // 
      this.lblToolVer.AutoSize = true;
      this.lblToolVer.Location = new System.Drawing.Point(156, 90);
      this.lblToolVer.Name = "lblToolVer";
      this.lblToolVer.Size = new System.Drawing.Size(23, 12);
      this.lblToolVer.TabIndex = 3;
      this.lblToolVer.Text = "3.0";
      // 
      // label3
      // 
      this.label3.AutoSize = true;
      this.label3.Location = new System.Drawing.Point(100, 119);
      this.label3.Name = "label3";
      this.label3.Size = new System.Drawing.Size(35, 12);
      this.label3.TabIndex = 4;
      this.label3.Text = "网址:";
      // 
      // label4
      // 
      this.label4.AutoSize = true;
      this.label4.Location = new System.Drawing.Point(156, 119);
      this.label4.Name = "label4";
      this.label4.Size = new System.Drawing.Size(131, 12);
      this.label4.TabIndex = 5;
      this.label4.Text = "http://www.pinusdb.cn";
      // 
      // label5
      // 
      this.label5.AutoSize = true;
      this.label5.Location = new System.Drawing.Point(100, 148);
      this.label5.Name = "label5";
      this.label5.Size = new System.Drawing.Size(35, 12);
      this.label5.TabIndex = 6;
      this.label5.Text = "邮箱:";
      // 
      // label6
      // 
      this.label6.AutoSize = true;
      this.label6.Location = new System.Drawing.Point(156, 148);
      this.label6.Name = "label6";
      this.label6.Size = new System.Drawing.Size(113, 12);
      this.label6.TabIndex = 7;
      this.label6.Text = "service@pinusdb.cn";
      // 
      // btnConfirm
      // 
      this.btnConfirm.Location = new System.Drawing.Point(125, 186);
      this.btnConfirm.Name = "btnConfirm";
      this.btnConfirm.Size = new System.Drawing.Size(75, 23);
      this.btnConfirm.TabIndex = 8;
      this.btnConfirm.Text = "确定";
      this.btnConfirm.UseVisualStyleBackColor = true;
      this.btnConfirm.Click += new System.EventHandler(this.btnConfirm_Click);
      // 
      // label7
      // 
      this.label7.AutoSize = true;
      this.label7.Font = new System.Drawing.Font("宋体", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
      this.label7.Location = new System.Drawing.Point(51, 23);
      this.label7.Name = "label7";
      this.label7.Size = new System.Drawing.Size(276, 16);
      this.label7.TabIndex = 9;
      this.label7.Text = "松果时序数据库(PinusDB)管理工具";
      // 
      // AboutForm
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(346, 239);
      this.Controls.Add(this.label7);
      this.Controls.Add(this.btnConfirm);
      this.Controls.Add(this.label6);
      this.Controls.Add(this.label5);
      this.Controls.Add(this.label4);
      this.Controls.Add(this.label3);
      this.Controls.Add(this.lblToolVer);
      this.Controls.Add(this.lblDBVersion);
      this.Controls.Add(this.label2);
      this.Controls.Add(this.label1);
      this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
      this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
      this.MaximizeBox = false;
      this.MinimizeBox = false;
      this.Name = "AboutForm";
      this.Text = "关于我们";
      this.Load += new System.EventHandler(this.AboutForm_Load);
      this.ResumeLayout(false);
      this.PerformLayout();

    }

    #endregion

    private System.Windows.Forms.Label label1;
    private System.Windows.Forms.Label label2;
    private System.Windows.Forms.Label lblDBVersion;
    private System.Windows.Forms.Label lblToolVer;
    private System.Windows.Forms.Label label3;
    private System.Windows.Forms.Label label4;
    private System.Windows.Forms.Label label5;
    private System.Windows.Forms.Label label6;
    private System.Windows.Forms.Button btnConfirm;
    private System.Windows.Forms.Label label7;
  }
}
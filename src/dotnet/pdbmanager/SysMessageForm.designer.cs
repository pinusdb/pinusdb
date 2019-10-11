namespace PDBManager
{
  partial class SysMessageForm
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
      this.label4 = new System.Windows.Forms.Label();
      this.lblLicDevCnt = new System.Windows.Forms.Label();
      this.groupBox1 = new System.Windows.Forms.GroupBox();
      this.linkWeb = new System.Windows.Forms.LinkLabel();
      this.linkEmail = new System.Windows.Forms.LinkLabel();
      this.label5 = new System.Windows.Forms.Label();
      this.lblDBVer = new System.Windows.Forms.Label();
      this.label6 = new System.Windows.Forms.Label();
      this.groupBox1.SuspendLayout();
      this.SuspendLayout();
      // 
      // label4
      // 
      this.label4.AutoSize = true;
      this.label4.Font = new System.Drawing.Font("华文中宋", 12F);
      this.label4.Location = new System.Drawing.Point(32, 62);
      this.label4.Name = "label4";
      this.label4.Size = new System.Drawing.Size(78, 19);
      this.label4.TabIndex = 27;
      this.label4.Text = "设备数量:";
      // 
      // lblLicDevCnt
      // 
      this.lblLicDevCnt.AutoSize = true;
      this.lblLicDevCnt.Font = new System.Drawing.Font("宋体", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
      this.lblLicDevCnt.Location = new System.Drawing.Point(116, 65);
      this.lblLicDevCnt.Name = "lblLicDevCnt";
      this.lblLicDevCnt.Size = new System.Drawing.Size(88, 16);
      this.lblLicDevCnt.TabIndex = 28;
      this.lblLicDevCnt.Text = "          ";
      this.lblLicDevCnt.TextAlign = System.Drawing.ContentAlignment.TopRight;
      // 
      // groupBox1
      // 
      this.groupBox1.Anchor = ((System.Windows.Forms.AnchorStyles)(((System.Windows.Forms.AnchorStyles.Top | System.Windows.Forms.AnchorStyles.Left) 
            | System.Windows.Forms.AnchorStyles.Right)));
      this.groupBox1.Controls.Add(this.lblLicDevCnt);
      this.groupBox1.Controls.Add(this.label4);
      this.groupBox1.Controls.Add(this.linkWeb);
      this.groupBox1.Controls.Add(this.linkEmail);
      this.groupBox1.Controls.Add(this.label5);
      this.groupBox1.Controls.Add(this.lblDBVer);
      this.groupBox1.Controls.Add(this.label6);
      this.groupBox1.Font = new System.Drawing.Font("宋体", 12F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
      this.groupBox1.Location = new System.Drawing.Point(17, 20);
      this.groupBox1.Name = "groupBox1";
      this.groupBox1.Size = new System.Drawing.Size(602, 220);
      this.groupBox1.TabIndex = 29;
      this.groupBox1.TabStop = false;
      this.groupBox1.Text = "数据库版本：";
      // 
      // linkWeb
      // 
      this.linkWeb.AutoSize = true;
      this.linkWeb.Font = new System.Drawing.Font("宋体", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
      this.linkWeb.Location = new System.Drawing.Point(32, 172);
      this.linkWeb.Name = "linkWeb";
      this.linkWeb.Size = new System.Drawing.Size(219, 19);
      this.linkWeb.TabIndex = 37;
      this.linkWeb.TabStop = true;
      this.linkWeb.Text = "http://www.pinusdb.cn";
      this.linkWeb.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.linkWeb_LinkClicked);
      // 
      // linkEmail
      // 
      this.linkEmail.AutoSize = true;
      this.linkEmail.Font = new System.Drawing.Font("宋体", 14.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
      this.linkEmail.Location = new System.Drawing.Point(32, 137);
      this.linkEmail.Name = "linkEmail";
      this.linkEmail.Size = new System.Drawing.Size(189, 19);
      this.linkEmail.TabIndex = 36;
      this.linkEmail.TabStop = true;
      this.linkEmail.Text = "service@pinusdb.cn";
      this.linkEmail.LinkClicked += new System.Windows.Forms.LinkLabelLinkClickedEventHandler(this.linkEmail_LinkClicked);
      // 
      // label5
      // 
      this.label5.AutoSize = true;
      this.label5.Font = new System.Drawing.Font("宋体", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
      this.label5.Location = new System.Drawing.Point(32, 24);
      this.label5.Name = "label5";
      this.label5.Size = new System.Drawing.Size(248, 19);
      this.label5.TabIndex = 34;
      this.label5.Text = "松果时序数据库(PinusDB)";
      // 
      // lblDBVer
      // 
      this.lblDBVer.AutoSize = true;
      this.lblDBVer.Font = new System.Drawing.Font("华文中宋", 14.25F, System.Drawing.FontStyle.Bold, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
      this.lblDBVer.Location = new System.Drawing.Point(296, 22);
      this.lblDBVer.Name = "lblDBVer";
      this.lblDBVer.Size = new System.Drawing.Size(0, 21);
      this.lblDBVer.TabIndex = 33;
      // 
      // label6
      // 
      this.label6.AutoSize = true;
      this.label6.Font = new System.Drawing.Font("华文中宋", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(134)));
      this.label6.Location = new System.Drawing.Point(32, 98);
      this.label6.Name = "label6";
      this.label6.Size = new System.Drawing.Size(470, 19);
      this.label6.TabIndex = 32;
      this.label6.Text = "版权所有 (C) 2019 长沙巨松软件科技有限公司。保留所有权利。";
      // 
      // SysMessageForm
      // 
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.None;
      this.ClientSize = new System.Drawing.Size(640, 433);
      this.Controls.Add(this.groupBox1);
      this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.None;
      this.Name = "SysMessageForm";
      this.Text = "系统信息";
      this.Load += new System.EventHandler(this.SysMessageForm_Load);
      this.groupBox1.ResumeLayout(false);
      this.groupBox1.PerformLayout();
      this.ResumeLayout(false);

    }

    #endregion
    private System.Windows.Forms.Label label4;
    private System.Windows.Forms.Label lblLicDevCnt;
    private System.Windows.Forms.GroupBox groupBox1;
    private System.Windows.Forms.Label lblDBVer;
    private System.Windows.Forms.Label label6;
    private System.Windows.Forms.Label label5;
    private System.Windows.Forms.LinkLabel linkEmail;
    private System.Windows.Forms.LinkLabel linkWeb;
  }
}
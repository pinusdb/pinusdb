namespace PDBManager
{
  partial class LoginForm
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
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(LoginForm));
      this.label1 = new System.Windows.Forms.Label();
      this.label2 = new System.Windows.Forms.Label();
      this.label3 = new System.Windows.Forms.Label();
      this.label4 = new System.Windows.Forms.Label();
      this.txbServer = new System.Windows.Forms.TextBox();
      this.txbPort = new System.Windows.Forms.TextBox();
      this.txbUser = new System.Windows.Forms.TextBox();
      this.txbPwd = new System.Windows.Forms.TextBox();
      this.btnCancel = new System.Windows.Forms.Button();
      this.btnAccept = new System.Windows.Forms.Button();
      this.SuspendLayout();
      // 
      // label1
      // 
      this.label1.AutoSize = true;
      this.label1.Location = new System.Drawing.Point(35, 30);
      this.label1.Name = "label1";
      this.label1.Size = new System.Drawing.Size(47, 12);
      this.label1.TabIndex = 0;
      this.label1.Text = "服务器:";
      // 
      // label2
      // 
      this.label2.AutoSize = true;
      this.label2.Location = new System.Drawing.Point(47, 67);
      this.label2.Name = "label2";
      this.label2.Size = new System.Drawing.Size(35, 12);
      this.label2.TabIndex = 1;
      this.label2.Text = "端口:";
      // 
      // label3
      // 
      this.label3.AutoSize = true;
      this.label3.Location = new System.Drawing.Point(35, 104);
      this.label3.Name = "label3";
      this.label3.Size = new System.Drawing.Size(47, 12);
      this.label3.TabIndex = 2;
      this.label3.Text = "用户名:";
      // 
      // label4
      // 
      this.label4.AutoSize = true;
      this.label4.Location = new System.Drawing.Point(47, 143);
      this.label4.Name = "label4";
      this.label4.Size = new System.Drawing.Size(35, 12);
      this.label4.TabIndex = 3;
      this.label4.Text = "密码:";
      // 
      // txbServer
      // 
      this.txbServer.Location = new System.Drawing.Point(101, 27);
      this.txbServer.Name = "txbServer";
      this.txbServer.Size = new System.Drawing.Size(167, 21);
      this.txbServer.TabIndex = 4;
      this.txbServer.Text = "127.0.0.1";
      // 
      // txbPort
      // 
      this.txbPort.Location = new System.Drawing.Point(101, 64);
      this.txbPort.Name = "txbPort";
      this.txbPort.Size = new System.Drawing.Size(167, 21);
      this.txbPort.TabIndex = 5;
      this.txbPort.Text = "8105";
      // 
      // txbUser
      // 
      this.txbUser.Location = new System.Drawing.Point(101, 101);
      this.txbUser.Name = "txbUser";
      this.txbUser.Size = new System.Drawing.Size(167, 21);
      this.txbUser.TabIndex = 6;
      this.txbUser.Text = "sa";
      // 
      // txbPwd
      // 
      this.txbPwd.Location = new System.Drawing.Point(101, 140);
      this.txbPwd.Name = "txbPwd";
      this.txbPwd.PasswordChar = '*';
      this.txbPwd.Size = new System.Drawing.Size(167, 21);
      this.txbPwd.TabIndex = 7;
      this.txbPwd.KeyDown += new System.Windows.Forms.KeyEventHandler(this.txbPwd_KeyDown);
      // 
      // btnCancel
      // 
      this.btnCancel.Image = global::PDBManager.Properties.Resources.del;
      this.btnCancel.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
      this.btnCancel.Location = new System.Drawing.Point(193, 196);
      this.btnCancel.Name = "btnCancel";
      this.btnCancel.Padding = new System.Windows.Forms.Padding(10, 0, 8, 0);
      this.btnCancel.Size = new System.Drawing.Size(75, 23);
      this.btnCancel.TabIndex = 9;
      this.btnCancel.Text = "取消";
      this.btnCancel.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
      this.btnCancel.UseVisualStyleBackColor = true;
      this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
      // 
      // btnAccept
      // 
      this.btnAccept.Image = global::PDBManager.Properties.Resources.accept;
      this.btnAccept.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
      this.btnAccept.Location = new System.Drawing.Point(91, 196);
      this.btnAccept.Name = "btnAccept";
      this.btnAccept.Padding = new System.Windows.Forms.Padding(10, 0, 8, 0);
      this.btnAccept.Size = new System.Drawing.Size(75, 23);
      this.btnAccept.TabIndex = 8;
      this.btnAccept.Text = "登录";
      this.btnAccept.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
      this.btnAccept.UseVisualStyleBackColor = true;
      this.btnAccept.Click += new System.EventHandler(this.btnAccept_Click);
      // 
      // LoginForm
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(315, 245);
      this.Controls.Add(this.btnCancel);
      this.Controls.Add(this.btnAccept);
      this.Controls.Add(this.txbPwd);
      this.Controls.Add(this.txbUser);
      this.Controls.Add(this.txbPort);
      this.Controls.Add(this.txbServer);
      this.Controls.Add(this.label4);
      this.Controls.Add(this.label3);
      this.Controls.Add(this.label2);
      this.Controls.Add(this.label1);
      this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
      this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
      this.MaximizeBox = false;
      this.MinimizeBox = false;
      this.Name = "LoginForm";
      this.Text = "松果时序数据库-登录";
      this.Load += new System.EventHandler(this.LoginForm_Load);
      this.ResumeLayout(false);
      this.PerformLayout();

    }

    #endregion

    private System.Windows.Forms.Label label1;
    private System.Windows.Forms.Label label2;
    private System.Windows.Forms.Label label3;
    private System.Windows.Forms.Label label4;
    private System.Windows.Forms.TextBox txbServer;
    private System.Windows.Forms.TextBox txbPort;
    private System.Windows.Forms.TextBox txbUser;
    private System.Windows.Forms.TextBox txbPwd;
    private System.Windows.Forms.Button btnAccept;
    private System.Windows.Forms.Button btnCancel;
  }
}
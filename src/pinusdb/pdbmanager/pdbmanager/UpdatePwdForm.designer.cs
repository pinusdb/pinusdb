namespace PDBManager
{
  partial class UpdatePwdForm
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
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(UpdatePwdForm));
      this.txbPwd1 = new System.Windows.Forms.TextBox();
      this.txbPwd = new System.Windows.Forms.TextBox();
      this.label3 = new System.Windows.Forms.Label();
      this.label2 = new System.Windows.Forms.Label();
      this.btnAccept = new System.Windows.Forms.Button();
      this.label1 = new System.Windows.Forms.Label();
      this.SuspendLayout();
      // 
      // txbPwd1
      // 
      this.txbPwd1.Location = new System.Drawing.Point(92, 99);
      this.txbPwd1.Name = "txbPwd1";
      this.txbPwd1.PasswordChar = '*';
      this.txbPwd1.Size = new System.Drawing.Size(167, 21);
      this.txbPwd1.TabIndex = 2;
      this.txbPwd1.KeyDown += new System.Windows.Forms.KeyEventHandler(this.txbPwd1_KeyDown);
      // 
      // txbPwd
      // 
      this.txbPwd.Location = new System.Drawing.Point(92, 60);
      this.txbPwd.Name = "txbPwd";
      this.txbPwd.PasswordChar = '*';
      this.txbPwd.Size = new System.Drawing.Size(167, 21);
      this.txbPwd.TabIndex = 1;
      // 
      // label3
      // 
      this.label3.AutoSize = true;
      this.label3.Location = new System.Drawing.Point(38, 102);
      this.label3.Name = "label3";
      this.label3.Size = new System.Drawing.Size(47, 12);
      this.label3.TabIndex = 12;
      this.label3.Text = "新密码:";
      // 
      // label2
      // 
      this.label2.AutoSize = true;
      this.label2.Location = new System.Drawing.Point(38, 63);
      this.label2.Name = "label2";
      this.label2.Size = new System.Drawing.Size(47, 12);
      this.label2.TabIndex = 11;
      this.label2.Text = "新密码:";
      // 
      // btnAccept
      // 
      this.btnAccept.Image = global::PDBManager.Properties.Resources.accept;
      this.btnAccept.ImageAlign = System.Drawing.ContentAlignment.MiddleLeft;
      this.btnAccept.Location = new System.Drawing.Point(184, 157);
      this.btnAccept.Name = "btnAccept";
      this.btnAccept.Padding = new System.Windows.Forms.Padding(10, 0, 8, 0);
      this.btnAccept.Size = new System.Drawing.Size(75, 23);
      this.btnAccept.TabIndex = 16;
      this.btnAccept.Text = "确认";
      this.btnAccept.TextAlign = System.Drawing.ContentAlignment.MiddleRight;
      this.btnAccept.UseVisualStyleBackColor = true;
      this.btnAccept.Click += new System.EventHandler(this.btnAccept_Click);
      // 
      // label1
      // 
      this.label1.AutoSize = true;
      this.label1.ForeColor = System.Drawing.Color.Red;
      this.label1.Location = new System.Drawing.Point(38, 23);
      this.label1.Name = "label1";
      this.label1.Size = new System.Drawing.Size(185, 12);
      this.label1.TabIndex = 17;
      this.label1.Text = "当前为系统默认密码，请修改密码";
      // 
      // UpdatePwdForm
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(306, 204);
      this.Controls.Add(this.label1);
      this.Controls.Add(this.btnAccept);
      this.Controls.Add(this.txbPwd1);
      this.Controls.Add(this.txbPwd);
      this.Controls.Add(this.label3);
      this.Controls.Add(this.label2);
      this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
      this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
      this.MaximizeBox = false;
      this.MinimizeBox = false;
      this.Name = "UpdatePwdForm";
      this.Text = "修改密码";
      this.ResumeLayout(false);
      this.PerformLayout();

    }

    #endregion
    private System.Windows.Forms.Button btnAccept;
    private System.Windows.Forms.TextBox txbPwd1;
    private System.Windows.Forms.TextBox txbPwd;
    private System.Windows.Forms.Label label3;
    private System.Windows.Forms.Label label2;
    private System.Windows.Forms.Label label1;
  }
}
namespace PDBManager
{
  partial class MessageForm
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
      System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(MessageForm));
      this.panel1 = new System.Windows.Forms.Panel();
      this.btnAccept = new System.Windows.Forms.Button();
      this.btnCancel = new System.Windows.Forms.Button();
      this.picBoxIcon = new System.Windows.Forms.PictureBox();
      this.lblMsg = new System.Windows.Forms.Label();
      this.panel1.SuspendLayout();
      ((System.ComponentModel.ISupportInitialize)(this.picBoxIcon)).BeginInit();
      this.SuspendLayout();
      // 
      // panel1
      // 
      this.panel1.BackColor = System.Drawing.Color.White;
      this.panel1.Controls.Add(this.lblMsg);
      this.panel1.Controls.Add(this.picBoxIcon);
      this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
      this.panel1.Location = new System.Drawing.Point(0, 0);
      this.panel1.Name = "panel1";
      this.panel1.Size = new System.Drawing.Size(344, 110);
      this.panel1.TabIndex = 0;
      // 
      // btnAccept
      // 
      this.btnAccept.Location = new System.Drawing.Point(173, 121);
      this.btnAccept.Name = "btnAccept";
      this.btnAccept.Size = new System.Drawing.Size(75, 30);
      this.btnAccept.TabIndex = 1;
      this.btnAccept.Text = "确定";
      this.btnAccept.UseVisualStyleBackColor = true;
      this.btnAccept.Click += new System.EventHandler(this.btnAccept_Click);
      // 
      // btnCancel
      // 
      this.btnCancel.Location = new System.Drawing.Point(257, 121);
      this.btnCancel.Name = "btnCancel";
      this.btnCancel.Size = new System.Drawing.Size(75, 30);
      this.btnCancel.TabIndex = 2;
      this.btnCancel.Text = "取消";
      this.btnCancel.UseVisualStyleBackColor = true;
      this.btnCancel.Click += new System.EventHandler(this.btnCancel_Click);
      // 
      // picBoxIcon
      // 
      this.picBoxIcon.Image = global::PDBManager.Properties.Resources.information;
      this.picBoxIcon.Location = new System.Drawing.Point(22, 30);
      this.picBoxIcon.Name = "picBoxIcon";
      this.picBoxIcon.Size = new System.Drawing.Size(32, 32);
      this.picBoxIcon.TabIndex = 0;
      this.picBoxIcon.TabStop = false;
      // 
      // lblMsg
      // 
      this.lblMsg.Location = new System.Drawing.Point(69, 9);
      this.lblMsg.MaximumSize = new System.Drawing.Size(240, 80);
      this.lblMsg.Name = "lblMsg";
      this.lblMsg.Size = new System.Drawing.Size(240, 73);
      this.lblMsg.TabIndex = 1;
      this.lblMsg.TextAlign = System.Drawing.ContentAlignment.MiddleLeft;
      // 
      // MessageForm
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 12F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(344, 163);
      this.Controls.Add(this.btnCancel);
      this.Controls.Add(this.btnAccept);
      this.Controls.Add(this.panel1);
      this.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedSingle;
      this.Icon = ((System.Drawing.Icon)(resources.GetObject("$this.Icon")));
      this.MaximizeBox = false;
      this.MinimizeBox = false;
      this.Name = "MessageForm";
      this.ShowIcon = false;
      this.Text = "MessageForm";
      this.panel1.ResumeLayout(false);
      ((System.ComponentModel.ISupportInitialize)(this.picBoxIcon)).EndInit();
      this.ResumeLayout(false);

    }

    #endregion

    private System.Windows.Forms.Panel panel1;
    private System.Windows.Forms.PictureBox picBoxIcon;
    private System.Windows.Forms.Button btnAccept;
    private System.Windows.Forms.Button btnCancel;
    private System.Windows.Forms.Label lblMsg;
  }
}
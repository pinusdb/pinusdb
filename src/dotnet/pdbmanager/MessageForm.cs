using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace PDBManager
{
  public partial class MessageForm : Form
  {
    public MessageForm(string msg, string title, MessageBoxIcon msgIcon, bool showAccept, bool showCancel)
    {
      InitializeComponent();
      this.Text = title;
      this.lblMsg.Text = msg;

      btnAccept.Visible = false;
      btnCancel.Visible = false;

      if (showAccept)
      {
        btnAccept.Visible = true;

        if (showCancel)
        {
          this.btnAccept.Location = new System.Drawing.Point(173, 121);
        }
        else
        {
          this.btnAccept.Location = new System.Drawing.Point(257, 121);
        }
      }

      if (showCancel)
      {
        btnCancel.Visible = true;
      }

      switch (msgIcon)
      {
        case MessageBoxIcon.Error:
          this.picBoxIcon.Image = global::PDBManager.Properties.Resources.error;
          break;
        case MessageBoxIcon.Information:
          this.picBoxIcon.Image = global::PDBManager.Properties.Resources.information;
          break;
        case MessageBoxIcon.Question:
          this.picBoxIcon.Image = global::PDBManager.Properties.Resources.question;
          break;
      }
    }

    private void btnAccept_Click(object sender, EventArgs e)
    {
      this.DialogResult = DialogResult.OK;
      this.Close();
    }

    private void btnCancel_Click(object sender, EventArgs e)
    {
      this.DialogResult = DialogResult.Cancel;
      this.Close();
    }
  }
}

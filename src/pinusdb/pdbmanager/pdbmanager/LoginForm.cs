using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using PDB.DotNetSDK;

namespace PDBManager
{
  public partial class LoginForm : Form
  {
    public LoginForm()
    {
      InitializeComponent();
    }

    private void btnAccept_Click(object sender, EventArgs e)
    {
      if (LoginPDB())
      {
        this.DialogResult = DialogResult.OK;
        this.Close();
      }
    }

    private void btnCancel_Click(object sender, EventArgs e)
    {
      this.DialogResult = DialogResult.Cancel;
      this.Close();
    }

    private void txbPwd_KeyDown(object sender, KeyEventArgs e)
    {
      if (e.KeyCode == Keys.Enter)
      {
        if (LoginPDB())
        {
          this.DialogResult = DialogResult.OK;
          this.Close();
        }
      }
    }

    private bool LoginPDB()
    {
      if (this.txbServer.Text.Trim().Length == 0)
      {
        MsgBox.ShowError("服务器地址不能为空!");
        return false;
      }

      if (this.txbPort.Text.Trim().Length == 0)
      {
        MsgBox.ShowError("端口不能为空!");
        return false;
      }

      if (this.txbUser.Text.Trim().Length == 0)
      {
        MsgBox.ShowError("用户名不能为空!");
        return false;
      }

      if (this.txbPwd.Text.Trim().Length == 0)
      {
        MsgBox.ShowError("密码不能为空!");
        return false;
      }

      string connStr = string.Format("server={0};port={1};username={2};password={3}",
        this.txbServer.Text.Trim(), this.txbPort.Text.Trim(),
        this.txbUser.Text.Trim(), this.txbPwd.Text.Trim());

      try
      {
        using (PDBConnection conn = new PDBConnection(connStr))
        {
          conn.Open();
        }
      }
      catch (Exception ex)
      {
        MsgBox.ShowError(ex.Message);
        return false;
      }
      
      GlobalVar.Server = this.txbServer.Text.Trim();
      GlobalVar.Port = int.Parse(this.txbPort.Text.Trim());
      GlobalVar.User = this.txbUser.Text.Trim();
      GlobalVar.Pwd = this.txbPwd.Text.Trim();
      GlobalVar.ConnStr = connStr;
      GlobalVar.IsConnected = true;

      return true;
    }

    private void LoginForm_Load(object sender, EventArgs e)
    {
      ActiveControl = txbPwd;
    }
  }
}

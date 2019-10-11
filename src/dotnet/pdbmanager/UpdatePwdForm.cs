using PDB.DotNetSDK;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace PDBManager
{
  public partial class UpdatePwdForm : Form
  {
    public UpdatePwdForm()
    {
      InitializeComponent();
    }
    private bool updatePwd()
    {
      if (this.txbPwd.Text.Trim().Length < 6 || this.txbPwd1.Text.Trim().Length < 6)
      {
        MsgBox.ShowError("密码不能小于6位!");
        return false;
      }

      if (!this.txbPwd.Text.Trim().Equals(this.txbPwd1.Text.Trim()))
      {
        MsgBox.ShowError("两次输入密码不相等，请重新输入！");

        this.txbPwd.Text = "";
        this.txbPwd1.Text = "";
        this.txbPwd.Focus();
        return false;
      }else if(this.txbPwd.Text.Trim().Equals("pinusdb"))
      {
        MsgBox.ShowError("不能设置为与初始密码相同！");
        return false;
      }
      else
      {
        try
        {         
          string sql = "set password for sa=password('" + this.txbPwd.Text.Trim() + "')";
          using (PDBConnection conn = new PDBConnection(GlobalVar.ConnStr))
          {
            conn.Open();
            PDBCommand cmd = conn.CreateCommand();
            cmd.ExecuteNonQuery(sql);

            GlobalVar.Pwd = this.txbPwd.Text.Trim();
            string connStr = string.Format("server={0};port={1};username={2};password={3}",
            GlobalVar.Server, GlobalVar.Port, GlobalVar.User, GlobalVar.Pwd);
            GlobalVar.ConnStr = connStr;
            this.DialogResult = DialogResult.OK;
          }
          return true;
        }
        catch (Exception ex)
        {
          MsgBox.ShowError(ex.Message);
          return false;
        }
      }
    }
    private void btnAccept_Click(object sender, EventArgs e)
    {
      if (updatePwd())
      {
        this.DialogResult = DialogResult.OK;
        this.Close();
      }
    }

    private void txbPwd1_KeyDown(object sender, KeyEventArgs e)
    {
      if (e.KeyCode == Keys.Enter)
      {
        if (updatePwd())
        {
          this.DialogResult = DialogResult.OK;
          this.Close();
        }
      }
    }
  }
}

using PDB.DotNetSDK;
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
  public partial class AboutForm : Form
  {
    public AboutForm()
    {
      InitializeComponent();
    }

    private void btnConfirm_Click(object sender, EventArgs e)
    {
      this.Close();
    }

    private void AboutForm_Load(object sender, EventArgs e)
    {
      String sql = "select name,value from sys_config";
      DataTable dtResult = null;
      try
      {
        using (PDBConnection conn = new PDBConnection(GlobalVar.ConnStr))
        {
          conn.Open();
          PDBCommand cmd = conn.CreateCommand();
          dtResult = cmd.ExecuteQuery(sql);

        }
      }
      catch (Exception ex)
      {
        MsgBox.ShowError("读取系统配置表出错");
        return;
      }

      string majorVer = "";
      string minorVer = "";
      string buildVer = "";

      foreach (DataRow dr in dtResult.Rows)
      {
        if (dr["name"].ToString().Equals("majorVersion"))
        {
          majorVer = dr["value"].ToString();
        }
        else if (dr["name"].ToString().Equals("minorVersion"))
        {
          minorVer = dr["value"].ToString();
        }
        else if (dr["name"].ToString().Equals("buildVersion"))
        {
          buildVer = dr["value"].ToString();
        }
      }

      this.lblDBVersion.Text = majorVer + "." + minorVer + "." + buildVer;
    }
  }
}

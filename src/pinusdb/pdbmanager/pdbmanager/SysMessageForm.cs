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
  public partial class SysMessageForm : Form
  {
    public SysMessageForm()
    {
      InitializeComponent();
    }

    private void SysMessageForm_Load(object sender, EventArgs e)
    {
      try
      {
        string majorVer = "";
        string minorVer = "";
        string buildVer = "";
        string dbSql = "select name,value from sys_config";
        using (PDBConnection conn = new PDBConnection(GlobalVar.ConnStr))
        {
          conn.Open();
          PDBCommand cmd = conn.CreateCommand();
          DataTable dtResult = cmd.ExecuteQuery(dbSql);
          foreach(DataRow dr in dtResult.Rows)
          {
            string nameStr = dr["name"].ToString().ToLower();
            string valStr = dr["value"].ToString();

            switch(nameStr)
            {
              case "majorversion":
                majorVer = valStr;
                break;
              case "minorversion":
                minorVer = valStr;
                break;
              case "buildversion":
                buildVer = valStr;
                break;
              case "devcnt":
                lblLicDevCnt.Text = valStr;
                break;
            }
          }
          
          lblDBVer.Text = majorVer + "." + minorVer + "." + buildVer;
        }
      }catch(Exception)
      {
        MsgBox.ShowError("读取系统配置失败");
        return;
      }
      
    }
    private void linkEmail_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
    {
      System.Diagnostics.Process.Start("mail://service@pinusdb.cn");
    }

    private void linkWeb_LinkClicked(object sender, LinkLabelLinkClickedEventArgs e)
    {
      System.Diagnostics.Process.Start("http://www.pinusdb.cn");
    }
    
  }
}

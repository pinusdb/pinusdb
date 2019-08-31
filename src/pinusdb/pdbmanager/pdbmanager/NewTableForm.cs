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
  public partial class NewTableForm : Form
  {
    public delegate void initDel();
    public event initDel initEvent;

    public NewTableForm()
    {
      InitializeComponent();
    }

    private void NewTableForm_Load(object sender, EventArgs e)
    {
    
      DGVFields.Rows.Add("devid", "bigint");
      DGVFields.Rows.Add("tstamp", "datetime");
      for (int i = 0; i < 2; i++)
      {
        DGVFields.Rows[i].Cells[0].ReadOnly = true;
        DGVFields.Rows[i].Cells[1].ReadOnly = true;
      }
      
    }
    
    private void btnCreateTable_Click(object sender, EventArgs e)
    {
      string tabName = txbTabName.Text.Trim().ToLower();

      if (!NameTool.ValidTableName(tabName))
      {
        MsgBox.ShowError("非法的表名!");
        return;
      }

      StringBuilder sbSql = new StringBuilder();
      sbSql.Append(" CREATE TABLE ");
      sbSql.Append(tabName);
      sbSql.Append("(");
      for (int i = 0; i < DGVFields.Rows.Count; i++)
      {
        if(i== DGVFields.Rows.Count-1&& DGVFields.Rows[i].Cells[0].Value == null && DGVFields.Rows[i].Cells[1].Value == null)
          break;

        if (DGVFields.Rows[i].Cells[0].Value == null)
        {
          MsgBox.ShowError("字段名为空");
          return;
        }

        if (DGVFields.Rows[i].Cells[1].Value == null)
        {
          MsgBox.ShowError("字段类型为空");
          return;
        }

        sbSql.AppendFormat(" {0} {1},", DGVFields.Rows[i].Cells[0].Value, DGVFields.Rows[i].Cells[1].Value);
      }

      //去掉最后的逗号
      sbSql.Remove((sbSql.Length - 1), 1);
      sbSql.Append(")");

      CreateTable(sbSql.ToString());
      
    }

    private bool CreateTable(string sql)
    {
      try
      {
        using (PDBConnection conn = new PDBConnection(GlobalVar.ConnStr))
        {
          conn.Open();
          PDBCommand cmd = conn.CreateCommand();
          cmd.ExecuteNonQuery(sql);
        }
      }
      catch(Exception ex)
      {
        MsgBox.ShowError(ex.Message);
        return false;
      }
      initEvent();
      MsgBox.ShowInfo("创建表成功");
      
      return true;
    }

    private void btnCancel_Click(object sender, EventArgs e)
    {
      this.txbTabName.Text = "";
      for(int i=2;i< DGVFields.Rows.Count;i++)
      {
        DGVFields.Rows[i].Cells[0].Value = null;
        DGVFields.Rows[i].Cells[1].Value = null;
      }

    }
  }
}

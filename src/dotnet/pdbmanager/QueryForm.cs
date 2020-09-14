using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using PDB.DotNetSDK;

using ICSharpCode.TextEditor;
using ICSharpCode.TextEditor.Gui.CompletionWindow;
using ICSharpCode.TextEditor.Document;
using ICSharpCode.TextEditor.Actions;
using System.Diagnostics;
using System.IO;

namespace PDBManager
{
  public partial class QueryForm : Form
  {
    private DataTable dtResult = null;
    internal string sql;
    private static char[] blobChar = new char[] {
       '0', '1', '2', '3', '4', '5', '6', '7',
       '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    public QueryForm()
    {
      InitializeComponent();

      sqlContext.Encoding = System.Text.Encoding.Default;

      sqlContext.Document.HighlightingStrategy = HighlightingStrategyFactory.CreateHighlightingStrategy("TSQL");
    }


    private void BindResult()
    {
      lvResult.Items.Clear();
      lvResult.Columns.Clear();

      if (dtResult != null)
      {
        //添加列信息

        bool isNull = false;
        ColumnHeader col = new ColumnHeader();
        col.Text = null;
        col.Width = 40;
        lvResult.Columns.Add(col);

        foreach (DataColumn colInfo in dtResult.Columns)
        {
          ColumnHeader colHeader = new ColumnHeader();
          colHeader.Text = colInfo.ColumnName;

          if (colInfo.DataType == typeof(sbyte)
            || colInfo.DataType == typeof(short)
            || colInfo.DataType == typeof(int)
            || colInfo.DataType == typeof(long)
            || colInfo.DataType == typeof(float)
            || colInfo.DataType == typeof(double)
            || colInfo.DataType == typeof(byte[]))
            colHeader.TextAlign = HorizontalAlignment.Right;

          if (colInfo.DataType == typeof(DateTime))
            colHeader.Width = 180;
          else if (colInfo.DataType == typeof(string))
            colHeader.Width = 120;
          else
            colHeader.Width = 80;

          lvResult.Columns.Add(colHeader);
        }
        int number = 1;
        foreach (DataRow dr in dtResult.Rows)
        {
          ListViewItem dataItem = new ListViewItem();
          dataItem.UseItemStyleForSubItems = false;

          dataItem.SubItems[0].Text = number.ToString();
          number++;

          for (int i = 0; i < dtResult.Columns.Count; i++)
          {
            dataItem.SubItems.Add(ValueToString(dr[i], out isNull));

            if (isNull)
              dataItem.SubItems[i + 1].ForeColor = Color.Gray;
          }

          lvResult.Items.Add(dataItem);
        }
      }
      else
      {
        ColumnHeader colHeader = new ColumnHeader();
        colHeader.Text = "";
        colHeader.Width = 80;
        lvResult.Columns.Add(colHeader);
      }
    }
    internal void sqlExecute(string sql)
    {
      if (sql.Length == 0)
        sql = sqlContext.Text.Trim();

      if (sql.Length <= 6)
      {
        MsgBox.ShowError("错误的SQL");
        return;
      }

      string execTypeStr = sql.Substring(0, 6).ToLower();
      if (execTypeStr.Equals("select"))
      {
        QuerySql(sql);
      }
      else if (execTypeStr.Equals("insert"))
      {
        InsertSql(sql);
      }
      else
      {
        NonQuerySql(sql);
      }
    }
    internal void setSqlContext(string sql)
    {
      sqlContext.Text = sql;
    }
    internal string getSqlContext()
    {
      return sqlContext.Text;
    }
    private void btnExecute_Click(object sender, EventArgs e)
    {
      string sql = sqlContext.ActiveTextAreaControl.SelectionManager.SelectedText.Trim();
      if (!GlobalVar.IsConnected)
      {
        MsgBox.ShowError("连接已断开，请重新连接！");
        return;
      }
      sqlExecute(sql);
    }

    private void QuerySql(string sql)
    {
      dtResult = null;
      try
      {
        Stopwatch sw = new Stopwatch();
        sw.Start();
        using (PDBConnection conn = new PDBConnection(GlobalVar.ConnStr))
        {
          conn.Open();
          PDBCommand cmd = conn.CreateCommand();
          dtResult = cmd.ExecuteQuery(sql);
          sw.Stop();
          this.tabControlResult.SelectedTab = this.tabPageData;
          this.richTxtMsg.Text = "SQL:\n" + sql + "\n执行成功\n执行耗时:" + sw.ElapsedMilliseconds + "ms";
          this.richTxtMsg.ForeColor = Color.Green;
        }
      }
      catch (Exception ex)
      {
        this.richTxtMsg.Text = "SQL:\n" + sql + "\n发生错误:" + ex.Message;
        this.richTxtMsg.ForeColor = Color.Red;
        this.tabControlResult.SelectedTab = this.tabPageMsg;
      }
      BindResult();
    }

    private void InsertSql(string sql)
    {
      dtResult = null;
      try
      {
        using (PDBConnection conn = new PDBConnection(GlobalVar.ConnStr))
        {
          conn.Open();
          PDBCommand cmd = conn.CreateCommand();
          PDBErrorCode retVal = cmd.ExecuteInsert(sql);
          if (retVal != PDBErrorCode.PdbE_OK)
          {
            this.richTxtMsg.Text = "SQL:\n" + sql + "\n发生错误:\n";
            this.richTxtMsg.Text += PDBErrorMsg.GetErrorMsg(retVal);
            this.richTxtMsg.ForeColor = Color.Red;
          }
          else
          {
            this.richTxtMsg.Text = "SQL:\n" + sql + "\n执行成功!";
            this.richTxtMsg.ForeColor = Color.Green;
          }
        }
      }
      catch (Exception ex)
      {
        this.richTxtMsg.Text = "执行SQL:\n" + sql + "\n发生错误:" + ex.Message;
        this.richTxtMsg.ForeColor = Color.Red;
      }

      this.tabControlResult.SelectedTab = this.tabPageMsg;
      BindResult();
    }

    private void NonQuerySql(string sql)
    {
      dtResult = null;

      try
      {
        using (PDBConnection conn = new PDBConnection(GlobalVar.ConnStr))
        {
          conn.Open();
          PDBCommand cmd = conn.CreateCommand();
          cmd.ExecuteNonQuery(sql);

          this.richTxtMsg.Text = "SQL:\n" + sql + "\n执行成功";
          this.richTxtMsg.ForeColor = Color.Green;
        }
      }
      catch (Exception ex)
      {
        this.richTxtMsg.Text = "执行SQL:\n" + sql + "\n发生错误:" + ex.Message;
        this.richTxtMsg.ForeColor = Color.Red;
      }

      this.tabControlResult.SelectedTab = this.tabPageMsg;
      BindResult();
    }

    private string ValueToString(Object obj, out bool isNull)
    {
      isNull = false;

      if (obj is DBNull)
      {
        isNull = true;
        return "null";
      }
      else if (obj is bool)
        return ((bool)obj).ToString();
      else if (obj is sbyte)
        return ((sbyte)obj).ToString();
      else if (obj is short)
        return ((short)obj).ToString();
      else if (obj is int)
        return ((int)obj).ToString();
      else if (obj is long)
        return ((long)obj).ToString();
      else if (obj is float)
        return ((float)obj).ToString();
      else if (obj is double)
        return ((double)obj).ToString();
      else if (obj is string)
        return ((string)obj).ToString();
      else if (obj is DateTime)
        return ((DateTime)obj).ToString("yyyy-MM-dd HH:mm:ss.ffffff");
      else if (obj is byte[])
      {
        StringBuilder blobBuilder = new StringBuilder();
        byte[] byteVal = (byte[])obj;
        foreach (byte b in byteVal)
        {
          blobBuilder.Append(blobChar[((b >> 4) & 0xF)]);
          blobBuilder.Append(blobChar[(b & 0xF)]);
        }
        return blobBuilder.ToString();
      }

      return "";
    }




    private bool rangeAcross(int bg1, int ed1, int bg2, int ed2)
    {
      if (ed1 < bg2 || ed2 < bg1)
        return false;

      return true;
    }


    private void btnOpenFile_Click(object sender, EventArgs e)
    {
      OpenFileDialog ofDialog = new OpenFileDialog();

      if (ofDialog.ShowDialog() == DialogResult.OK)
      {
        try
        {
          StreamReader fileReader = new StreamReader(ofDialog.FileName, Encoding.Default);
          this.sqlContext.Text = fileReader.ReadToEnd();
          fileReader.Close();
        }
        catch (Exception ex)
        {
          MsgBox.ShowError("打开文件发生错误," + ex.Message + "!");
        }
      }
    }

    internal void btnSaveFile_Click(object sender, EventArgs e)
    {
      SaveFileDialog sfDialog = new SaveFileDialog();
      if (sfDialog.ShowDialog() == DialogResult.OK)
      {
        try
        {
          StreamWriter fileWriter = new StreamWriter(sfDialog.FileName);
          fileWriter.Write(this.sqlContext.Text);
          fileWriter.Close();
          MsgBox.ShowInfo("保存文件成功");
        }
        catch (Exception ex)
        {
          MsgBox.ShowError("保存文件发生错误," + ex.Message + "!");
        }
      }
    }

    private void btnSavaResult_Click(object sender, EventArgs e)
    {
      SaveFileDialog sfDialog = new SaveFileDialog();
      if (sfDialog.ShowDialog() == DialogResult.OK)
      {
        try
        {
          StreamWriter fileWriter = new StreamWriter(sfDialog.FileName);
          string tempText = "";
          for (int j = 1; j < lvResult.Columns.Count; j++)
            tempText += lvResult.Columns[j].Text + ",";
          fileWriter.WriteLine(tempText);

          for (int i = 0; i < lvResult.Items.Count; i++)
          {
            tempText = "";
            for (int j = 1; j < lvResult.Columns.Count; j++)
            {
              if (dtResult.Columns[j - 1].DataType == typeof(string))
              {
                string temp = "";
                int curIdx = 0;
                int bigIdx = 0;
                while (curIdx < lvResult.Items[i].SubItems[j].Text.Count())
                {
                  curIdx = lvResult.Items[i].SubItems[j].Text.IndexOf('"', curIdx);
                  if (curIdx == -1)
                  {
                    temp += lvResult.Items[i].SubItems[j].Text.Substring(bigIdx);
                    break;
                  }

                  curIdx++;
                  temp += lvResult.Items[i].SubItems[j].Text.Substring(bigIdx, (curIdx - bigIdx)) + "\"";
                  bigIdx = curIdx;
                }
                tempText += "\"" + temp + "\"" + ",";
              }
              else
                tempText += lvResult.Items[i].SubItems[j].Text + ",";
            }
            fileWriter.WriteLine(tempText);
          }

          fileWriter.Close();
          MsgBox.ShowInfo("保存结果成功");
        }
        catch (Exception ex)
        {
          MsgBox.ShowError("保存结果发生错误," + ex.Message + "!");
        }
      }


    }


  }
}

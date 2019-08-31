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
  public partial class MainForm : Form
  {
    public MainForm()
    {
      InitializeComponent();
      GlobalVar.IsConnected = false;
    }

    private void MainForm_SizeChanged(object sender, EventArgs e)
    {
      tabMain.Refresh();
    }

    private void menuMain_about_Click(object sender, EventArgs e)
    {
      AboutForm aboutForm = new AboutForm();
      aboutForm.StartPosition = FormStartPosition.CenterParent;
      aboutForm.ShowDialog();
    }

    private void menuMain_newQuery_Click(object sender, EventArgs e)
    {
      ShowFormToTab("查询   ", new QueryForm());
    }

    private void menuTable_newTable_Click(object sender, EventArgs e)
    {
      createTable();
    }

    private void menuMain_ConnDatabase_Click(object sender, EventArgs e)
    {
      LoginForm loginForm = new LoginForm();
      loginForm.StartPosition = FormStartPosition.CenterParent;
      loginForm.ShowDialog();

      if (loginForm.DialogResult != DialogResult.OK)
        return;

      if (GlobalVar.User.Equals("sa") && GlobalVar.Pwd.Equals("pinusdb"))
      {
        UpdatePwdForm updateForm = new UpdatePwdForm();
        updateForm.StartPosition = FormStartPosition.CenterScreen;
        updateForm.ShowDialog();
        if (updateForm.DialogResult != DialogResult.OK)
          return;
      }

      InitTreeNav();
      UpdateStatusInfo();
      InitBtnEnabled();

      timerState.Enabled = true;
    }

    private void menuMain_CloseDataBase_Click(object sender, EventArgs e)
    {
      disConnectDatabase();
    }

    private void menuMain_Exit_Click(object sender, EventArgs e)
    {
      this.Close();
    }

    //private void stripNav_ConnDatabase_Click(object sender, EventArgs e)
    //{

    //}
    private void createTable()
    {
      NewTableForm form = new NewTableForm();
      form.initEvent += InitTreeNav;
      ShowFormToTab("创建表   ", form);
    }

    private void stripNav_NewTable_Click(object sender, EventArgs e)
    {
      createTable();
    }

    private void stripNav_NewQuery_Click(object sender, EventArgs e)
    {
      ShowFormToTab("查询   ", new QueryForm());
    }


    private void ShowFormToTab(string formName, Form form)
    {
      TabPage tabPage = new TabPage(formName);
      this.tabMain.TabPages.Add(tabPage);
      form.TopLevel = false;
      form.Parent = tabPage;
      form.ControlBox = false;
      form.Dock = DockStyle.Fill;
      form.Show();
      this.tabMain.SelectedTab = tabPage;
    }

    private void tabMain_DrawItem(object sender, DrawItemEventArgs e)
    {
      try
      {
        Rectangle tabRect = this.tabMain.GetTabRect(e.Index);
        int CLOSE_SIZE = 13;
        //先添加TabPage属性
        e.Graphics.DrawString(this.tabMain.TabPages[e.Index].Text,
          this.Font, SystemBrushes.Desktop, tabRect.X + 8, tabRect.Y + 4);

        //再画一个矩形框
        using (Pen p = new Pen(Color.AliceBlue))
        {
          tabRect.Offset(tabRect.Width - (CLOSE_SIZE + 3), 2);
          tabRect.Width = CLOSE_SIZE;
          tabRect.Height = CLOSE_SIZE;
          e.Graphics.DrawRectangle(p, tabRect);
        }

        //画关闭符号
        using (Pen objPen = new Pen(Color.Black))
        {
          Point p1 = new Point(tabRect.X + 3, tabRect.Y + 3);
          Point p2 = new Point(tabRect.X + tabRect.Width - 3, tabRect.Y + tabRect.Height - 3);
          e.Graphics.DrawLine(objPen, p1, p2);

          Point p3 = new Point(tabRect.X + 3, tabRect.Y + tabRect.Height - 3);
          Point p4 = new Point(tabRect.X + tabRect.Width - 3, tabRect.Y + 3);
          e.Graphics.DrawLine(objPen, p3, p4);
        }

        e.Graphics.Dispose();
      }
      catch (Exception ex)
      {

      }
    }

    private void MainForm_Load(object sender, EventArgs e)
    {
      if (!GlobalVar.IsConnected)
      {
        LoginForm loginForm = new LoginForm();
        loginForm.StartPosition = FormStartPosition.CenterScreen;
        loginForm.ShowDialog();
        if (loginForm.DialogResult != DialogResult.OK)
        {
          this.Close();
          return;
        }
      }

      if (GlobalVar.User.Equals("sa") && GlobalVar.Pwd.Equals("pinusdb"))
      {
        UpdatePwdForm updateForm = new UpdatePwdForm();
        updateForm.StartPosition = FormStartPosition.CenterScreen;
        updateForm.ShowDialog();
        if (updateForm.DialogResult != DialogResult.OK)
        {
          this.Close();
          return;
        }
      }


      InitTreeNav();
      UpdateStatusInfo();
      InitBtnEnabled();
      ShowSysMessage();
      timerState.Enabled = true;
    }

    private void ShowSysMessage()
    {

      SysMessageForm sysForm = new SysMessageForm();
      ShowFormToTab("系统信息   ", sysForm);
    }
    private void InitTreeNav()
    {
      if (GlobalVar.IsConnected)
      {
        TreeNode rootNode = new TreeNode(string.Format("{0}:{1}", GlobalVar.Server, GlobalVar.Port));
        rootNode.ImageKey = "database.png";
        rootNode.SelectedImageKey = "database.png";

        //系统信息
        TreeNode sysInfoNode = new TreeNode("系统信息");
        sysInfoNode.ImageKey = "info.png";
        sysInfoNode.SelectedImageKey = "info.png";

        rootNode.Nodes.Add(sysInfoNode);

        /////////////////////////////////////////////////////////////////
        //系统表
        TreeNode sysTabNode = new TreeNode("系统表");
        sysTabNode.ImageKey = "folder.png";
        sysTabNode.SelectedImageKey = "folder.png";
        
        //数据表
        TreeNode dataTabNode = new TreeNode("数据表");
        dataTabNode.ImageKey = "folder.png";
        dataTabNode.SelectedImageKey = "folder.png";

        BindTableList(sysTabNode, dataTabNode);

        rootNode.Nodes.Add(sysTabNode);
        rootNode.Nodes.Add(dataTabNode);

        //////////////////////////////////////////////////////////////////

        tvNav.Nodes.Clear();

        tvNav.Nodes.Add(rootNode);
        rootNode.Expand();
        dataTabNode.Expand();

      }
      else
      {
        tvNav.Nodes.Clear();
      }
    }

    private void BindTableList(TreeNode sysTabRoot, TreeNode dataTabRoot)
    {
      try
      {
        using (PDBConnection conn = new PDBConnection(GlobalVar.ConnStr))
        {
          conn.Open();
          PDBCommand cmd = conn.CreateCommand();

          DataTable dtTable = cmd.ExecuteQuery("select * from sys_table");
          foreach(DataRow dr in dtTable.Rows)
          {
            string tabName = dr["tabname"].ToString();
            DataTable colTable = cmd.ExecuteQuery(string.Format("select * from sys_column where tabname='{0}'", tabName));

            if (tabName.StartsWith("sys_"))
            {
              BindTableInfo(sysTabRoot, tabName, colTable);
            }
            else
            {
              BindTableInfo(dataTabRoot, tabName, colTable);
            }
          }
        }
      }
      catch (Exception ex)
      {
        MsgBox.ShowError(ex.Message);
      }
    }

    private void BindTableInfo(TreeNode nodeRoot, string tabName, DataTable colTable)
    {
      TreeNode tabNode = new TreeNode(tabName);
      tabNode.ImageKey = "db_table.png";
      tabNode.SelectedImageKey = "db_table.png";
      
      foreach(DataRow dr in colTable.Rows)
      {
        string fieldType = dr["datatype"].ToString();
        string fieldName = dr["colname"].ToString();
        bool isKey = Convert.ToBoolean(dr["iskey"]);
        TreeNode fieldNode = new TreeNode(
          string.Format("{0},{1}", fieldName, fieldType));

        if (isKey)
        {
          fieldNode.ImageKey = "db_field_key.png";
          fieldNode.SelectedImageKey = "db_field_key.png";
        }
        else
        {
          fieldNode.ImageKey = "db_field.png";
          fieldNode.SelectedImageKey = "db_field.png";
        }

        tabNode.Nodes.Add(fieldNode);
      }

      nodeRoot.Nodes.Add(tabNode);
    } 
    
    private string DataTypeToStr(Type dataType)
    {
      if (dataType == typeof(bool))
        return "bool";
      else if (dataType == typeof(long))
        return "bigint";
      else if (dataType == typeof(float))
        return "float";
      else if (dataType == typeof(double))
        return "double";
      else if (dataType == typeof(string))
        return "string";
      else if (dataType == typeof(byte[]))
        return "blob";
      else if (dataType == typeof(DateTime))
        return "datetime";

      return "";
    }
    
    private void UpdateStatusInfo()
    {
      if (GlobalVar.IsConnected)
      {
        this.statusLbl_Status.Text = string.Format("已连接, {0}:{1}", GlobalVar.Server, GlobalVar.Port);
        this.statusLbl_Status.ForeColor = Color.Green;
        this.statusLbl_localTime.Text = DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss");
      }
      else
      {
        this.statusLbl_Status.Text = "断开";
        this.statusLbl_Status.ForeColor = Color.Red;
        this.statusLbl_localTime.Text = DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss");
      }
    }

    private void timerState_Tick(object sender, EventArgs e)
    {
      UpdateStatusInfo();
    }

    private void connectDatabase()
    {
      LoginForm loginForm = new LoginForm();
      loginForm.StartPosition = FormStartPosition.CenterParent;
      loginForm.ShowDialog();
      if (loginForm.DialogResult != DialogResult.OK)
      {
        return;
      }

      if (GlobalVar.User.Equals("sa") && GlobalVar.Pwd.Equals("pinusdb"))
      {
        UpdatePwdForm updateForm = new UpdatePwdForm();
        updateForm.StartPosition = FormStartPosition.CenterScreen;
        updateForm.ShowDialog();
        if (updateForm.DialogResult != DialogResult.OK)
          return;
      }


      InitTreeNav();
      UpdateStatusInfo();
      InitBtnEnabled();

      timerState.Enabled = true;
    }
    private void stripNav_Connect_Click(object sender, EventArgs e)
    {
      connectDatabase();
    }

    private void stripNav_Refresh_Click(object sender, EventArgs e)
    {
      InitTreeNav();
    }

    private void disConnectDatabase()
    {
      if (GlobalVar.IsConnected)
      {
        if (MsgBox.ShowQuestion(string.Format("确认断开[{0}:{1}]的连接？",
          GlobalVar.Server, GlobalVar.Port)) != DialogResult.OK)
        {
          return;
        }
      }

      GlobalVar.IsConnected = false;
      GlobalVar.Server = "";
      GlobalVar.User = "";
      GlobalVar.Port = 0;

      InitTreeNav();
      InitBtnEnabled();
    }
    private void stripNav_DisConnect_Click(object sender, EventArgs e)
    {
      disConnectDatabase();
    }

    private void InitBtnEnabled()
    {
      stripNav_Connect.Enabled = !GlobalVar.IsConnected;
      stripNav_DisConnect.Enabled = GlobalVar.IsConnected;
      stripNav_Refresh.Enabled = GlobalVar.IsConnected;
      stripNav_NewTable.Enabled = GlobalVar.IsConnected;
      stripNav_NewQuery.Enabled = GlobalVar.IsConnected;
    }

    private void menuTable_delTable_Click(object sender, EventArgs e)
    {
      deleteTable();
    }



    private void tabMain_MouseClick(object sender, MouseEventArgs e)
    {
      //退出当前页
      for(int i=0;i<this.tabMain.TabPages.Count;i++)
      {
        Rectangle r = tabMain.GetTabRect(i);
        Rectangle closeButton = new Rectangle(r.Right - 15, r.Top + 2, 13, 13);
        if(closeButton.Contains(e.Location))
        {
          if (this.tabMain.TabPages[i].Text.Trim() == "查询")
          {
            //获取查询窗体
            Control control = tabMain.TabPages[i].Controls[0];
            QueryForm qf = (QueryForm)control;
            if(qf.getSqlContext().Trim()!="")
            {
              DialogResult dr = MsgBox.ShowQuestion("是否保存当前文件?");
              if (dr == DialogResult.OK)
              {
                new QueryForm().btnSaveFile_Click(sender, e);
              }
            }
            
          }
          this.tabMain.TabPages.RemoveAt(i);
          if(i>0)
            this.tabMain.SelectedTab = this.tabMain.TabPages[i - 1];
        }
      }
    }

    private void MenuItem_newQuery_Click(object sender, EventArgs e)
    {
      ShowFormToTab("查询   ", new QueryForm());
    }

    private void tvNav_MouseDown(object sender, MouseEventArgs e)
    {
      if(e.Button==MouseButtons.Right)
      {
        Point clickPoint = new Point(e.X, e.Y);
        TreeNode currentNode = tvNav.GetNodeAt(clickPoint);
        if(currentNode != null)
        {
          if(currentNode.Level == 0)
            currentNode.ContextMenuStrip = MenuStripDatabase;
          else if(currentNode.Level==1)
          {
            if(currentNode.Text=="系统表")
            {
              currentNode.ContextMenuStrip = menuStripSystem;
              menuStripSystem.Items[1].Visible = false;
            }
            else if(currentNode.Text == "数据表")
            {
              currentNode.ContextMenuStrip = menuStripNav;
              menuStripNav.Items[2].Visible = false;
              menuStripNav.Items[3].Visible = false;
              menuStripNav.Items[4].Visible = false;
            }
          }
          else
          {
            switch (currentNode.Parent.Text)
            {
              case "系统表": {
                               currentNode.ContextMenuStrip = menuStripSystem;
                               menuStripSystem.Items[1].Visible = true;
                             } break;
              case "数据表": {
                               currentNode.ContextMenuStrip = menuStripNav;
                               menuStripNav.Items[2].Visible = true;
                               menuStripNav.Items[3].Visible = true;
                               menuStripNav.Items[4].Visible = true;
                             } break;
            }
          }
          tvNav.SelectedNode = currentNode;

        }

      }
      else
      {
        Point clickPoint = new Point(e.X, e.Y);
        TreeNode currentNode = tvNav.GetNodeAt(clickPoint);
        if (currentNode != null)
        {
          if (currentNode.Level == 1)
          {
            if (currentNode.Text == "系统信息")
            {
              for (int i = 0; i < this.tabMain.TabPages.Count; i++)
              {
                if (tabMain.TabPages[i].Text.Trim() == "系统信息")
                {
                  this.tabMain.SelectedTab = this.tabMain.TabPages[i];
                  return;
                }
              }             
              ShowSysMessage();
              tvNav.SelectedNode = currentNode;
            }
          }
        }
      }
    }

    private void MenuItem_newTable_Click(object sender, EventArgs e)
    {
      createTable();
    }

    private void MenuItem_delTable_Click(object sender, EventArgs e)
    {
      deleteTable();
    }
    private void deleteTable()
    {
      if (tvNav.SelectedNode != null)
      {
        if (tvNav.SelectedNode.Level == 0||tvNav.SelectedNode.Parent.Text != "数据表")
        {
          MsgBox.ShowError("请选择要删除的数据表!");
          return;
        }
        string tableName = tvNav.SelectedNode.Text;
        DialogResult dr=MsgBox.ShowQuestion("您确定要删除数据表【" + tableName + "】吗?");
        if (dr == DialogResult.OK)
        {
          string sql = "drop table " + tableName;
          try
          {
            using (PDBConnection conn = new PDBConnection(GlobalVar.ConnStr))
            {
              conn.Open();
              PDBCommand cmd = conn.CreateCommand();
              cmd.ExecuteNonQuery(sql);
            }
          }
          catch (Exception ex)
          {
            MsgBox.ShowError(ex.Message);
            return;
          }
          InitTreeNav();
          MsgBox.ShowInfo("删除表成功");
        }
      }
      else
      {
        MsgBox.ShowError("请选择要删除的数据表!");
      }

    }
    private void detachTable()
    {
      if (tvNav.SelectedNode != null)
      {
        //根节点没有父节点会报错
        if (tvNav.SelectedNode.Level == 0 || tvNav.SelectedNode.Parent.Text != "数据表")
        {
          MsgBox.ShowError("请选择要分离的数据表!");
          return;
        }
        string tableName = tvNav.SelectedNode.Text;
        DialogResult dr = MsgBox.ShowQuestion("您确定要分离数据表【" + tableName + "】吗?");
        if (dr == DialogResult.OK)
        {
          string sql = "detach table " + tableName;
          try
          {
            using (PDBConnection conn = new PDBConnection(GlobalVar.ConnStr))
            {
              conn.Open();
              PDBCommand cmd = conn.CreateCommand();
              cmd.ExecuteNonQuery(sql);
            }
          }
          catch (Exception ex)
          {
            MsgBox.ShowError(ex.Message);
            return;
          }
          InitTreeNav();
          MsgBox.ShowInfo("分离表成功");
        }
      }
      else
      {
        MsgBox.ShowError("请选择要分离的数据表!");
      }
    }
    private void MenuItem_detachTable_Click(object sender, EventArgs e)
    {
      detachTable();
    }

    private void MenuItem_detach_Click(object sender, EventArgs e)
    {
      detachTable();
    }

    private void MenuItem_selectTop1000_Click(object sender, EventArgs e)
    {
      if (tvNav.SelectedNode != null)
      {
        if (tvNav.SelectedNode.Level == 0 || tvNav.SelectedNode.Parent.Text != "数据表")
        {
          MsgBox.ShowError("请选择要查询的数据表!");
          return;
        }
        QueryForm form = new QueryForm();
        string tableName = tvNav.SelectedNode.Text;
        form.sql = "select * from " + tableName + " limit 1000";
        ShowFormToTab("查询   ", form);
        form.setSqlContext(form.sql);
        form.sqlExecute(form.sql);
      }
      else
      {
        MsgBox.ShowError("请选择要查询的数据表!");
      }
    }

    private void MenuItem_select_Click(object sender, EventArgs e)
    {
      if (tvNav.SelectedNode != null)
      {
        if (tvNav.SelectedNode.Level == 0 || tvNav.SelectedNode.Parent.Text != "系统表")
        {
          MsgBox.ShowError("请选择要查询的系统表!");
          return;
        }
        QueryForm form = new QueryForm();
        string tableName = tvNav.SelectedNode.Text;
        form.sql = "select * from " + tableName + " limit 1000";
        ShowFormToTab("查询   ", form);
        form.setSqlContext(form.sql);
        form.sqlExecute(form.sql);
      }
      else
      {
        MsgBox.ShowError("请选择要查询的系统表!");
      }
    }

    private void MenuItem_Query_Click(object sender, EventArgs e)
    {
      ShowFormToTab("查询   ", new QueryForm());
    }

    private void MenuItem_connect_Click(object sender, EventArgs e)
    {
      connectDatabase();
    }

    private void MenuItem_disconnect_Click(object sender, EventArgs e)
    {
      disConnectDatabase();
    }

    private void MenuItem_refresh_Click(object sender, EventArgs e)
    {
      InitTreeNav();
    }
  }
}
